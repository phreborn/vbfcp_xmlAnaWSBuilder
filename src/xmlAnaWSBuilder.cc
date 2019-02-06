#include "xmlAnaWSBuilder.hh"

ClassImp(xmlAnaWSBuilder);

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// XML workspace builder keywords: below can be used in 
// building likelihood model. They will be replaced by
// corresponding objects.
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TString xmlAnaWSBuilder::RESPONSE="response::";
TString xmlAnaWSBuilder::OBSERVABLE=":observable:";
TString xmlAnaWSBuilder::PROCESS=":process:";
TString xmlAnaWSBuilder::COMMON=":common:";
TString xmlAnaWSBuilder::SELF=":self:";
TString xmlAnaWSBuilder::CATEGORY=":category:";

// C++ logics not displayable in XML format
TString xmlAnaWSBuilder::LT=":lt:";
TString xmlAnaWSBuilder::LE=":le:";
TString xmlAnaWSBuilder::GT=":gt:";
TString xmlAnaWSBuilder::GE=":ge:";
TString xmlAnaWSBuilder::AND=":and:";
TString xmlAnaWSBuilder::OR=":or:";

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// XML workspace builder keywords: below are used to
// Indicate type of operations one perform
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
TString xmlAnaWSBuilder::ALLPROC="_allproc_";
TString xmlAnaWSBuilder::YIELD="yield";
TString xmlAnaWSBuilder::SHAPE="shape";
TString xmlAnaWSBuilder::COUNTING="counting";

// Type of input data file
TString xmlAnaWSBuilder::ASCII="ascii";

// Type of PDF input
TString xmlAnaWSBuilder::USERDEF="userdef";
TString xmlAnaWSBuilder::EXTERNAL="external";
TString xmlAnaWSBuilder::HISTOGRAM="histogram";

// Constraint terms
TString xmlAnaWSBuilder::GAUSSIAN="gaus";
TString xmlAnaWSBuilder::LOGNORMAL="logn";
TString xmlAnaWSBuilder::ASYMMETRIC="asym"; 

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Hard-coded object naming partten: do not duplicate
// these partten when constructing your own model!
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Naming convention
TString xmlAnaWSBuilder::RESPONSEPREFIX="expected__";
TString xmlAnaWSBuilder::CONSTRTERMPREFIX="constr__";
TString xmlAnaWSBuilder::GLOBALOBSPREFIX="RNDM__";
TString xmlAnaWSBuilder::VARIATIONHIPREFIX="varHi__";
TString xmlAnaWSBuilder::VARIATIONLOPREFIX="varLo__";
TString xmlAnaWSBuilder::YIELDPREFIX="yield__";
TString xmlAnaWSBuilder::PDFPREFIX="pdf__";
TString xmlAnaWSBuilder::EXPECTATIONPREFIX="expectation__";

// PDF names
TString xmlAnaWSBuilder::SUMPDFNAME="_modelSB";
TString xmlAnaWSBuilder::FINALPDFNAME="_model";

// Factor names
TString xmlAnaWSBuilder::LUMINAME="_luminosity";
TString xmlAnaWSBuilder::XSNAME="_xs";
TString xmlAnaWSBuilder::BRNAME="_BR";
TString xmlAnaWSBuilder::NORMNAME="_norm";
TString xmlAnaWSBuilder::ACCEPTANCENAME="_A";
TString xmlAnaWSBuilder::CORRECTIONNAME="_C";
TString xmlAnaWSBuilder::EFFICIENCYNAME="_eff";

// Observed dataset name
TString xmlAnaWSBuilder::OBSDSNAME="obsdata";

// Sideband fit range
TString xmlAnaWSBuilder::SBLO="SBLo";
TString xmlAnaWSBuilder::BLIND="Blind";
TString xmlAnaWSBuilder::SBHI="SBHi";

xmlAnaWSBuilder::xmlAnaWSBuilder(TString inputFile){
  cout << "Parsing file: " << inputFile << endl;
  TDOMParser xmlparser;
  // reading in the file and parse by DOM
  auxUtil::parseXMLFile(&xmlparser, inputFile);

  TXMLDocument* xmldoc=xmlparser.GetXMLDocument();
  TXMLNode* rootNode=xmldoc->GetRootNode();
  TXMLNode* node=rootNode->GetChildren();

  _outputFileName=auxUtil::getAttributeValue(rootNode, "OutputFile");
  if(_outputFileName.Contains('/')){
    TString dirName=_outputFileName(0, _outputFileName.Last('/'));
    system("mkdir -vp "+dirName);
  }
  if(!_outputFileName.Contains(".root")){
    cerr<<"\tWARNING: Output file name does not contain \".root\" postfix. Adding it to avoid confusion."<<endl;
    _outputFileName+=".root";
  }
  _wsName=auxUtil::getAttributeValue(rootNode, "WorkspaceName");
  _mcName=auxUtil::getAttributeValue(rootNode, "ModelConfigName");
  _dataName=auxUtil::getAttributeValue(rootNode, "DataName");
  _goBlind=auxUtil::to_bool(auxUtil::getAttributeValue(rootNode, "Blind", true, "0"));
  
  _asimovHandler=auto_ptr<asimovUtil>(new asimovUtil());
  
  while ( node != 0 ){
    TString nodeName=node->GetNodeName();
    if(nodeName=="POI"){
      TString poiStr=node->GetText();
      _POIList=auxUtil::splitString(poiStr.Data(),',');
    } 
    if(nodeName=="Input"){
      _xmlPath.push_back(node->GetText());
    } 
    if(nodeName=="Asimov"){
      _asimovHandler->addEntry(node);
    }
    node=node->GetNextNode();
  }

  _Nch=_xmlPath.size();
  _useBinned=false;
  _plotOpt="";
  
  auxUtil::printTime();
  _timer.Start();
  cout<<"======================================="<<endl;
  cout<<"Output file: "<<_outputFileName<<endl;
  cout<<"Workspace name: "<<_wsName<<endl;
  cout<<"ModelConfig name: "<<_mcName<<endl;
  cout<<"Data name: "<<_dataName<<endl;
  if(_goBlind) cout<<auxUtil::WARNING<<"Blind analysis"<<auxUtil::ENDC<<endl;
  cout<<"POI: ";
  for(auto poi : _POIList) cout<<poi<<" ";
  cout<<endl;
  cout<<_Nch<<" categories to be included"<<endl;
  for(int ich=0;ich<_Nch;ich++) cout<<"XML file "<<ich<<": "<<_xmlPath[ich]<<endl;
  _asimovHandler->printSummary();
  cout<<"======================================="<<endl;
  // Start working...
  _combWS=auto_ptr<RooWorkspace>(new RooWorkspace(_wsName));
  _mConfig=auto_ptr<ModelConfig>(new ModelConfig(_mcName, _combWS.get()));
}

void xmlAnaWSBuilder::generateWS(){
  
  RooCategory channellist("channellist","channellist");
  RooSimultaneous CombinedPdf("CombinedPdf","",channellist) ;

  RooArgSet POI, nuisanceParameters, globalObservables, Observables, constraints;

  map<string,RooDataSet*> datasetMap, datasetMap_binned;
  vector<shared_ptr<RooWorkspace> > wArr;
  
  for( int ich = 0 ; ich < _Nch; ich ++ ){
    wArr.push_back(shared_ptr<RooWorkspace>(new RooWorkspace(Form("wchannel_%d", ich))));

    generateSingleChannel(_xmlPath[ich], wArr[ich].get());
    
    channellist.defineType(_CN[ich]) ;
    CombinedPdf.addPdf(*wArr[ich]->pdf(FINALPDFNAME+"_"+_CN[ich]),_CN[ich]) ;

    nuisanceParameters.add(*wArr[ich]->set("nuisanceParameters"), true);
    globalObservables.add(*wArr[ich]->set("globalObservables"), true);
    Observables.add(*wArr[ich]->set("Observables"));
    POI.add(*wArr[ich]->set("POI"), true);

    datasetMap[_CN[ich].Data()] = dynamic_cast<RooDataSet*>(wArr[ich]->data(OBSDSNAME));
    datasetMap_binned[_CN[ich].Data()] = dynamic_cast<RooDataSet*>(wArr[ich]->data(OBSDSNAME+"binned"));
    if(_debug) wArr[ich]->Print();
  }
  Observables.add(channellist);
  _combWS->import(CombinedPdf, Silence());

  _combWS->defineSet("nuisanceParameters",nuisanceParameters);
  _combWS->defineSet("Observables",Observables);
  _combWS->defineSet("globalObservables",globalObservables);
  _combWS->defineSet("POI",POI);   

  _mConfig->SetPdf(*_combWS->pdf("CombinedPdf"));
  _mConfig->SetObservables(*_combWS->set("Observables"));
  _mConfig->SetParametersOfInterest(*_combWS->set("POI"));
  _mConfig->SetNuisanceParameters(*_combWS->set("nuisanceParameters"));
  _mConfig->SetGlobalObservables(*_combWS->set("globalObservables"));
  _combWS->import(*_mConfig);

  RooRealVar wt("wt","wt",1);

  RooArgSet args;
  args.add(Observables);
  args.add(wt);

  RooDataSet obsData(_dataName, "Combined data", args, Index(channellist), Import(datasetMap), WeightVar(wt));
  RooDataSet obsDatabinned(_dataName+"binned", "Binned combined data", args, Index(channellist), Import(datasetMap_binned), WeightVar(wt));
  
  _combWS->import(obsData);
  if(obsDatabinned.numEntries()<obsData.numEntries()) _combWS->import(obsDatabinned);
  else{
    cout<<endl<<auxUtil::WARNING<<" \tREGTEST: No need to keep binned dataset, as the number of data events is smaller than the number of bins in all categories. "<<auxUtil::ENDC<<endl<<endl;
    _useBinned=false;
  }
  wArr.clear();

  // Save the original snapshot
  // _combWS->saveSnapshot("nominalNuis",*_mConfig->GetNuisanceParameters());
  // _combWS->saveSnapshot("nominalGlobs",*_mConfig->GetGlobalObservables());
  if(_goBlind) _asimovHandler->setRange(SBLO+","+SBHI);
  if(_useBinned) cout<<endl<<auxUtil::WARNING<<" \tREGTEST: Fitting binned dataset. "<<auxUtil::ENDC<<endl<<endl;
  
  if(_asimovHandler->genAsimov()) _asimovHandler->generateAsimov(_mConfig.get(), _useBinned?_dataName+"binned":_dataName);
  _combWS->importClassCode();
  _combWS->writeToFile(_outputFileName);
  
  TString outputFigName=_outputFileName;
  outputFigName.ReplaceAll(".root",".pdf");
  
  cout<<"========================================================================"<<endl;
  Summary(outputFigName);
  cout<<"Workspace "<<_wsName<<" has be successfully generated and saved in file "<<_outputFileName<<endl;
  cout<<"Plots for each category are summarized in "<<outputFigName<<endl;
  auxUtil::printTime();
  _timer.Stop();
  _timer.Print();
  cout<<"========================================================================"<<endl;
}

void xmlAnaWSBuilder::readSyst(TXMLNode* systNode, TString domain){
  if(_debug) cout<<"\tREGTEST: Reading systematic: "<<auxUtil::getAttributeValue(systNode, "Name")<<endl;
  Systematic syst;
  syst.NPName=auxUtil::getAttributeValue(systNode, "Name");
  syst.process=auxUtil::getAttributeValue(systNode, "Process", true, ""); // If the process of the systematic is specified, use the specified process. Otherwise use the default one
  if(domain==ALLPROC){		// Common systematics
    if(syst.process!="") syst.domain=syst.process; // If a process name is specified, use it as domain name and remove it from common systematic
    else syst.domain=domain;			   // Otherwise consider it as common systematic
  }
  else{
    syst.domain=domain;		// For systematics under a sample, the domain is always the sample name
    if(syst.process=="") syst.process=domain;
    else syst.process=domain+"_"+syst.process; // If the systematic has a process, attach it to domain name as process name
  }
  
  syst.whereTo=auxUtil::getAttributeValue(systNode, "WhereTo");
  syst.nominal=atof(auxUtil::getAttributeValue(systNode, "CentralValue"));
  syst.constrTerm=auxUtil::getAttributeValue(systNode, "Constr");
  if(syst.nominal<=0&&(syst.constrTerm==LOGNORMAL||syst.constrTerm==ASYMMETRIC))
    auxUtil::alertAndAbort("For constraint term type "+syst.constrTerm+Form(" non-positive central value (%f) is not acceptable", syst.nominal));
  TString uncert=auxUtil::getAttributeValue(systNode, "Mag");
  
  syst.beta=auxUtil::stripSign(uncert);
  
  if(uncert.Contains(',')){
    vector<TString> uncerts=auxUtil::splitString(uncert,',');
    syst.errHiExpr=uncerts[0];
    syst.errLoExpr=uncerts[1];
    auxUtil::stripSign(syst.errHiExpr); // Need to make sure we implement absolute value. Sign has been taken before.
    auxUtil::stripSign(syst.errLoExpr); // Need to make sure we implement absolute value. Sign has been taken before.
    // A RooFit feature noticed by Jared: if one uses FlexibleInterpVar to implement a systematic on a process, then for the same systematic on a different process FlexibleInterpVar has to be used also. Otherwise the code will crash.
    // Therefore we need to keep the keyword "asym". When it is specified the FlexibleInterpVar will be enforced
    if(syst.constrTerm!=ASYMMETRIC){
      if((syst.errHiExpr!=syst.errLoExpr) ||
	 !(syst.errHiExpr.IsFloat()&&syst.errLoExpr.IsFloat()&&syst.errHiExpr.Atof()==syst.errLoExpr.Atof())
	 ){
	cout<<"\t INFO: Upper ("<<syst.errHiExpr<<") and lower ("<<syst.errLoExpr<<") uncertianties for "<<syst.constrTerm<<" systematic "
	    <<syst.NPName<<" are actually not identical."<<endl;
	cout<<"\t Will implement it as asymmetric uncertainty using FlexibleInterpVar with interpolation code 4. Next time please use keyword \""+ASYMMETRIC+"\" for constraint term type instead of \""+syst.constrTerm+"\"."<<endl;
	syst.constrTerm=ASYMMETRIC;
      }
    }
  }
  else{
    syst.errHiExpr=syst.errLoExpr=uncert;
  }

  if(find(_Systematics[syst.domain].begin(), _Systematics[syst.domain].end(), syst)==_Systematics[syst.domain].end()) _Systematics[syst.domain].push_back(syst);
  else auxUtil::alertAndAbort("Systematic "+syst.NPName+" applied on "+syst.whereTo+" is duplicated for process "+syst.process);
}

void xmlAnaWSBuilder::readSample(TXMLNode* sampleNode){
  Sample sample;
  sample.procName=auxUtil::getAttributeValue(sampleNode, "Name");
  // sample.yield=atof(auxUtil::getAttributeValue(sampleNode, "Norm"));
  sample.inputFile=auxUtil::getAttributeValue(sampleNode, "InputFile", (_categoryType==COUNTING), "");

  // TString importSystGroupList=auxUtil::getAttributeValue(sampleNode, "ImportSyst", true, COMMON);
  TString importSystGroupList=auxUtil::getAttributeValue(sampleNode, "ImportSyst", true, SELF);
  sample.systGroups=auxUtil::splitString(importSystGroupList.Data(),',');
  auxUtil::removeDuplicatedString(sample.systGroups);
  auxUtil::removeString(sample.systGroups, sample.procName);
  
  TString norm=auxUtil::getAttributeValue(sampleNode, "Norm", true, ""); // default value 1
  TString xsection=auxUtil::getAttributeValue(sampleNode, "XSection", true, ""); // default value 1
  TString br=auxUtil::getAttributeValue(sampleNode, "BR", true, ""); // default value 1
  TString selectionEff=auxUtil::getAttributeValue(sampleNode, "SelectionEff", true, ""); // default value 1
  TString acceptance=auxUtil::getAttributeValue(sampleNode, "Acceptance", true, ""); // default value 1
  TString correction=auxUtil::getAttributeValue(sampleNode, "Correction", true, ""); // default value 1
  
  bool isMultiplyLumi=(_luminosity>0) ? auxUtil::to_bool(auxUtil::getAttributeValue(sampleNode, "MultiplyLumi", true, "1")) : false; // default value true. If luminosity not provided it will always be false

  // Assamble the yield central value
  if(isMultiplyLumi) sample.normFactors.push_back(LUMINAME);
  if(norm!="") sample.normFactors.push_back(NORMNAME+"_"+sample.procName+"["+norm+"]");
  if(xsection!="") sample.normFactors.push_back(XSNAME+"_"+sample.procName+"["+xsection+"]");
  if(br!="") sample.normFactors.push_back(BRNAME+"_"+sample.procName+"["+br+"]");
  if(selectionEff!="") sample.normFactors.push_back(EFFICIENCYNAME+"_"+sample.procName+"["+selectionEff+"]");
  if(acceptance!="") sample.normFactors.push_back(ACCEPTANCENAME+"_"+sample.procName+"["+acceptance+"]");
  if(correction!="") sample.normFactors.push_back(ACCEPTANCENAME+"_"+sample.procName+"["+correction+"]");

  sample.sharePdfGroup=auxUtil::getAttributeValue(sampleNode, "SharePdf", true, ""); // default value false
  
  TXMLNode* subNode = sampleNode->GetChildren();
  if(_debug) cout<<"\tREGTEST: Reading sample "<<sample.procName<<endl;
  readSampleXMLNode( subNode, sample ); // read xml file

  if(find(_Samples.begin(), _Samples.end(), sample)==_Samples.end()) _Samples.push_back(sample);
  else auxUtil::alertAndAbort("Sample "+sample.procName+" has been included more than once");
  return;
}

void xmlAnaWSBuilder::readSampleXMLNode(TXMLNode* node, Sample& sample){
  while ( node != 0 ){
    if ( node->GetNodeName() == TString( "Systematic" ) ){
      readSyst(node, sample.procName);
    }
    else if ( node->GetNodeName() == TString( "NormFactor" ) ){
      TString normFactor=getItemExpr(node, "Name", sample.procName);
      sample.normFactors.push_back(normFactor);
      bool isCorrelated=auxUtil::to_bool(auxUtil::getAttributeValue(node, "Correlate", true, "0"));
      if(isCorrelated) _ItemsCorrelate.push_back(auxUtil::getObjName(normFactor));
    }
    else if ( node->GetNodeName() == TString( "ShapeFactor" ) ){
      TString shapeFactor=getItemExpr(node, "Name", sample.procName);
      sample.shapeFactors.push_back(shapeFactor);
      bool isCorrelated=auxUtil::to_bool(auxUtil::getAttributeValue(node, "Correlate", true, "0"));
      if(isCorrelated) _ItemsCorrelate.push_back(auxUtil::getObjName(shapeFactor));
    }
    else if ( node->GetNodeName() == TString( "ImportItems" ) ){
      TString inputFileName=auxUtil::getAttributeValue(node, "FileName");
      TDOMParser xmlparser;
      auxUtil::parseXMLFile(&xmlparser, inputFileName);
      cout<<"\tREGTEST: Importing Items for "<< sample.procName << " from " << inputFileName << endl;

      TXMLDocument* xmldoc=xmlparser.GetXMLDocument();
      TXMLNode* rootNode=xmldoc->GetRootNode();
      TXMLNode* importNode=rootNode->GetChildren();
      readSampleXMLNode( importNode, sample );
    }
    node=node->GetNextNode();
  }
}

void xmlAnaWSBuilder::generateSingleChannel(TString xmlName, RooWorkspace *wchannel){

  RooArgSet nuispara, constraints, globobs, expected, expected_shape;

  map<TString, RooArgSet> expectedMap;
  
  TDOMParser xmlparser;
  // reading in the file and parse by DOM
  auxUtil::parseXMLFile(&xmlparser, xmlName);

  TXMLDocument* xmldoc=xmlparser.GetXMLDocument();
  TXMLNode* rootNode = xmldoc->GetRootNode();

  // Get the category name and property from
  _categoryName=auxUtil::getAttributeValue(rootNode, "Name");
  _categoryType=auxUtil::getAttributeValue(rootNode, "Type");
  _luminosity=atof(auxUtil::getAttributeValue(rootNode, "Lumi", true, "-1"));
  
  _categoryType.ToLower();

  if(find(_CN.begin(), _CN.end(), _categoryName)==_CN.end()) _CN.push_back(_categoryName);
  else auxUtil::alertAndAbort("Category name "+_categoryName+" used in XML file"+xmlName+" is already used by other categories. Please use a different name");
  auxUtil::printTitle("Category "+_categoryName);
  
  _Type.push_back(_categoryType);
  /* all the attributes of a channel */

  unique_ptr<RooWorkspace> wfactory(new RooWorkspace("factory_"+_categoryName));
  if(_luminosity>0) implementObj(wfactory.get(), LUMINAME+Form("[%f]", _luminosity)); // First thing: create a luminosity variable

  TXMLNode *dataNode=auxUtil::findNode(rootNode, "Data"); // This attribute is only allowed to appear once per-channel, and cannot be hided in a sub-XML file
  if (!dataNode) auxUtil::alertAndAbort("No data node found in channel XML file "+xmlName);
  
  _observableName=auxUtil::getAttributeValue(dataNode, "Observable");
  _observableName=implementObj(wfactory.get(), _observableName);
  _xMin=wfactory->var(_observableName)->getMin();
  _xMax=wfactory->var(_observableName)->getMax();

  if(_xMax<=_xMin) auxUtil::alertAndAbort("Invalid range for observable "+_observableName+Form(": min %f, max %f", _xMin, _xMax));

  int nbinx=(_categoryType==COUNTING) ? 1 : atoi(auxUtil::getAttributeValue(dataNode, "Binning"));
  wfactory->var(_observableName)->setBins(nbinx);
  
  _inputDataFileName=auxUtil::getAttributeValue(dataNode, "InputFile", (_categoryType==COUNTING), "");
  _inputDataFileType=auxUtil::getAttributeValue(dataNode, "FileType", true, ASCII);
  _inputDataFileType.ToLower();
  if(_inputDataFileType!=ASCII){	// means that we are reading data from a ntuple or histogram
    if(_inputDataFileType==HISTOGRAM) _inputDataHistName=auxUtil::getAttributeValue(dataNode, "HistName");
    else{
      _inputDataTreeName=auxUtil::getAttributeValue(dataNode, "TreeName");
      _inputDataVarName=auxUtil::getAttributeValue(dataNode, "VarName");
      _Cut=auxUtil::getAttributeValue(dataNode, "Cut", true, "");
      translateKeyword(_Cut);
    }
  }
  _injectGhost=auxUtil::to_bool(auxUtil::getAttributeValue(dataNode, "InjectGhost", true, "0")); // Default false

  _numData=(_categoryType==COUNTING && _inputDataFileName=="") ? atoi(auxUtil::getAttributeValue(dataNode, "NumData")) : -1; // Only needed for counting experiment where the input data file is not specified
  
  dataFileSanityCheck();	// Check now and report, before it is too late
  
  TXMLNode *correlateNode=auxUtil::findNode(rootNode, "Correlate"); // This attribute is only allowed to appear at most once per-channel, and cannot be hided in a sub-XML file
  if(correlateNode){	// If you would like to put some parameters which are not POI correlated
    TString itemStr=correlateNode->GetText();
    _ItemsCorrelate=auxUtil::splitString(itemStr.Data(),',');
  }

  readChannelXMLNode(rootNode->GetChildren());

  // Bug reported by Jared and Leo: if a high priority item contains a low priority item as proxy, the creation will fail. Therefore we need to go through the proxies used in high-priority items and move those which contain low-priority proxies to low-priority
  for(vector<TString>::iterator it=_ItemsHighPriority.begin(); it!=_ItemsHighPriority.end();){
    TString itemHi=*it;
    int type=auxUtil::getItemType(itemHi);
    if(_debug) cout<<itemHi<<" is "<<auxUtil::translateItemType(itemHi)<<endl;
    if(type==auxUtil::VARIABLE || type==auxUtil::EXIST){
      it++;
      continue; // Not function
    }
    vector<TString> itemList=auxUtil::decomposeFuncStr(itemHi);
    if(_debug) cout<<"Composition of high priority item "<<itemHi<<":";
    bool degrade=false;
    for(auto item : itemList){
      if(_debug) cout<<item<<" ";
      for(auto itemLo : _ItemsLowPriority){
    	TString itemLoName=auxUtil::getObjName(itemLo);
    	if(item==itemLoName){
    	  degrade=true;
    	  break;
    	}
      }
    }
    if(degrade){
      _ItemsLowPriority.push_back(itemHi);
      it=_ItemsHighPriority.erase(it);
    }
    else it++;

    if(_debug) cout<<endl;
  }

  if(_debug){
    cout<<"\tREGTEST: high priority items: "<<endl;
    for(auto item : _ItemsHighPriority) cout<<item<<endl;
    cout<<"\tREGTEST: low priority items: "<<endl;
    for(auto item : _ItemsLowPriority) cout<<item<<endl;
  }
  // First implement high priority items: common variables and functions not involve systematic uncertainties
  implementObjArray(wfactory.get(), _ItemsHighPriority);

  // Secondly implement systematics and import them into the workspace
  for(auto syst : _Systematics){
    TString domain=syst.first;
    vector<Systematic> systArr=syst.second;
    RooArgSet *respCollection=NULL, *respCollectionYield=NULL;
    
    if(domain==ALLPROC) respCollectionYield=&expected;      // Systematics to be applied to all resonant processes
    else{
      auto it=find(_Samples.begin(), _Samples.end(), domain);
      if(it!=_Samples.end()) respCollectionYield=&it->expected;
      else respCollectionYield=&expectedMap[domain];
    }
    
    for(Systematic syst : systArr){
      if(syst.whereTo==YIELD){
	respCollection=respCollectionYield;
      }
      else if(syst.whereTo==SHAPE) respCollection=&expected_shape; // A dump for shape response terms
      else auxUtil::alertAndAbort("Unknown systematic loacation "+syst.whereTo
				  +". Choose from \""+SHAPE+"\" or \""+YIELD+"\"");

      NPMaker(wfactory.get(), &syst, &nuispara, &constraints, &globobs, respCollection);
    }
  }

  // Now work on remaining common variables and functions to be implemented
  implementObjArray(wfactory.get(), _ItemsLowPriority);

  if(_debug) wfactory->Print();

  // Finally organize the scale factors on each sample
  if(expected.getSize()>0) implementObj(wfactory.get(), auxUtil::generateExpr("prod::"+EXPECTATIONPREFIX+"common(", &expected));

  for(auto it : expectedMap) implementObj(wfactory.get(), auxUtil::generateExpr("prod::"+EXPECTATIONPREFIX+it.first+"(", &it.second));
  
  for(auto& sample : _Samples){
    if(_debug) auxUtil::printTitle(sample.procName.Data(), "<", 20);
    
    implementObjArray(wfactory.get(), sample.shapeFactors);

    sample.normName=YIELDPREFIX+sample.procName;
    
    TString normStr="prod::"+sample.normName+"("+implementObjArray(wfactory.get(), sample.normFactors);

    if(sample.expected.getSize()>0){
      TString expectationStr=auxUtil::generateExpr("prod::"+EXPECTATIONPREFIX+"proc_"+sample.procName+"(", &sample.expected);
      normStr+=", "+implementObj(wfactory.get(), expectationStr);
    }

    if(find(sample.systGroups.begin(), sample.systGroups.end(), SELF)==sample.systGroups.end()){ // If :self: appears anywhere, do not do anything
      for(TString systGrp : sample.systGroups){
	if(systGrp == COMMON){
	  if(expected.getSize()>0) normStr+=", "+EXPECTATIONPREFIX+"common"; // Common systematics: only add if exist
	}
	else{
	  if(expectedMap.find(systGrp)==expectedMap.end()) auxUtil::alertAndAbort("Unknown systematic group "+systGrp+" in Sample "+sample.procName);
	  normStr+=", "+EXPECTATIONPREFIX+systGrp;
	}
      }
    }
    auxUtil::closeFuncExpr(normStr);
    
    implementObj(wfactory.get(), normStr);
    
    cout<<"\tREGTEST: Yield for category \""<<_categoryName<<"\" process \""<<sample.procName<<"\": "<<wfactory->function(sample.normName)->getVal()<<endl;
    if(_debug) wfactory->Print();
    getModel(wfactory.get(), &sample, &nuispara, &constraints, &globobs);
  }

  if(_debug) wfactory->Print();

  // Generate the combined PDF
  TString sumPdfStr="SUM::"+SUMPDFNAME+"(";
  for(auto sample : _Samples) sumPdfStr+=sample.normName+"*"+sample.modelName+",";

  auxUtil::closeFuncExpr(sumPdfStr);
  if(_debug) cout<<sumPdfStr<<endl;
  implementObj(wfactory.get(), sumPdfStr);
  // sumPdfStr=auxUtil::getObjName(sumPdfStr);
  RooAddPdf *SBPdf=dynamic_cast<RooAddPdf*>(wfactory->pdf(SUMPDFNAME));

  // Now handle blinding if needed
  TString blindRange=auxUtil::getAttributeValue(dataNode, "BlindRange", true, "");
  if(_goBlind){
    TString rangeName="";
    if(blindRange!=""){
      cout<<auxUtil::WARNING<<" \tREGTEST: Implement blind range "<<blindRange<<auxUtil::ENDC<<endl;
      vector<TString> rangeComp=auxUtil::splitString(blindRange,',');
      if(rangeComp.size()!=2 || !rangeComp[0].IsFloat() || !rangeComp[1].IsFloat()) auxUtil::alertAndAbort("Invalid format for blinding range: "+blindRange);
      _blindMin=rangeComp[0].Atof();
      _blindMax=rangeComp[1].Atof();
      if(_blindMax<=_blindMin || _blindMax>_xMax || _blindMin<_xMin) auxUtil::alertAndAbort(Form("Invalid blinding range provided: min %f, max %f", _blindMin, _blindMax));

      wfactory->var(_observableName)->setRange(SBLO+"_"+_categoryName, _xMin, _blindMin);
      wfactory->var(_observableName)->setRange(BLIND+"_"+_categoryName, _blindMin, _blindMax);
      wfactory->var(_observableName)->setRange(SBHI+"_"+_categoryName, _blindMax, _xMax);
      // wfactory->var(_observableName)->setRange("FULL", _xMin, _xMax); // Suggested by Wouter

      if(_blindMax==_xMax && _blindMin==_xMin){
	cout<<endl<<auxUtil::WARNING<<" \tREGTEST: Category "+_categoryName+" fully blinded. No side-band exists. "<<auxUtil::ENDC<<endl<<endl;
	rangeName="";
      }
      else if(_blindMax==_xMax) rangeName=SBLO+"_"+_categoryName;
      else if(_blindMin==_xMin) rangeName=SBHI+"_"+_categoryName;
      else rangeName=SBLO+"_"+_categoryName+","+SBHI+"_"+_categoryName;

      // SBPdf->fixCoefRange("FULL");
    }
    else{
      _blindMin=_xMax;
      _blindMax=_xMin;
    }
    _rangeList.push_back(rangeName);
  }


  checkNuisParam(SBPdf, &nuispara);

  // Keep Track of Correlated variables
  TString correlated = "";

  for(auto poi : _POIList) _ItemsCorrelate.push_back(poi);

  // remove duplicates from list of _ItemsCorrelate (must be sorted to work)
  auxUtil::removeDuplicatedString(_ItemsCorrelate);
  
  for(auto item : _ItemsCorrelate){
    if(!wfactory->obj(item)) continue; // Does not exist
    if(!wfactory->var(item)) auxUtil::alertAndAbort("Correlated variable "+item+" is not properly implemented as RooRealVar in the workspace"); // Only variables can be keep the names unchanged.
    correlated+=item+",";
  }
  
  correlated += auxUtil::generateExpr("",&nuispara,false);

  correlated+=_observableName;

  // Now, import the pdf to a new workspace, where the renaming of objects will happen automatically
  if(_debug) cout<<"\tREGTEST: The following variables will not be renamed: "<<correlated<<endl;
  wchannel->import( (*wfactory->pdf(SUMPDFNAME)), RenameAllNodes(_categoryName), RenameAllVariablesExcept(_categoryName,correlated), Silence());

  // Import constraint terms. Note we should not rename the constraint term gaussians
  attachConstraints(wchannel, SUMPDFNAME+"_"+_categoryName, &constraints, FINALPDFNAME+"_"+_categoryName);

  auxUtil::defineSet(wchannel, _POIList, "POI");
  auxUtil::defineSet(wchannel, nuispara, "nuisanceParameters");
  auxUtil::defineSet(wchannel, globobs, "globalObservables");
  auxUtil::defineSet(wchannel, RooArgSet(*wchannel->var(_observableName)), "Observables");

  if(_debug) wchannel->Print();

  //++++++++++++++++++++++++ Now produce dataset ++++++++++++++++++++++++++++++++
  RooRealVar* x=wchannel->var(_observableName);
  RooRealVar wt("wt","wt",1);
  RooArgSet obs_plus_wt;

  obs_plus_wt.add(wt);
  obs_plus_wt.add(*x);
  
  unique_ptr<RooDataSet> obsdata(readInData(x, &wt));
  if(_injectGhost) auxUtil::releaseTheGhost(obsdata.get(),x, &wt, auxUtil::epsilon/1000.);

  // prepare binned data
  unique_ptr<RooDataSet> obsdatabinned(dynamic_cast<RooDataSet*>(obsdata->Clone(OBSDSNAME+"binned")));

  if(obsdata->sumEntries()>x->numBins()){
    TH1D h_data("h_data","",x->numBins(),x->getMin(),x->getMax());
    RooArgSet* obs = const_cast<RooArgSet*>(obsdata->get());
    RooRealVar* xdata = dynamic_cast<RooRealVar*>(obs->find(_observableName));
    
    for (int i=0 ; i<obsdata->numEntries() ; i++) {
      obsdata->get(i) ;
      h_data.Fill( xdata->getVal(), obsdata->weight());
    }
    obsdatabinned.reset(new RooDataSet(OBSDSNAME+"binned",OBSDSNAME+"binned",obs_plus_wt,WeightVar(wt)));
    for( int ibin = 1 ; ibin <= h_data.GetNbinsX() ; ibin ++ ) {
      x->setVal(h_data.GetBinCenter(ibin));
      double weight = h_data.GetBinContent(ibin);
      wt.setVal(weight);
      obsdatabinned -> add( RooArgSet(*x, wt), weight);
    }
  }
  cout<<"\tREGTEST: Number of data events: "<<obsdata->sumEntries()<<endl;
  wchannel->import(*obsdata);
  wchannel->import(*obsdatabinned);
  clearUp();			// Remove content in the vectors and maps
}

void xmlAnaWSBuilder::NPMaker(RooWorkspace *w, Systematic *syst, RooArgSet *nuispara, RooArgSet *constraints, RooArgSet *globobs, RooArgSet *expected){
  
  TString varName=syst->whereTo+"_"+syst->NPName;
  if(syst->domain!=ALLPROC) varName+="_"+syst->process;

  TString globName=GLOBALOBSPREFIX+syst->NPName;
  TString constrName=CONSTRTERMPREFIX+syst->NPName;
  TString responseName=RESPONSEPREFIX+varName; // Important: a NP can be applied to both shape and yield.

  if(syst->constrTerm==ASYMMETRIC) {
    if(_debug) cout << "\tREGTEST: Set up nuisance parameter "
		    << syst->NPName << " as asymmetric uncertainty on process "<< syst->process <<endl;
    RooRealVar nuis_var(syst->NPName,syst->NPName,0,-5,5);
    RooRealVar beta_var("beta_"+varName,"beta_"+varName,syst->beta);
    RooProduct nuis_times_beta(varName+"_times_beta",varName+"_times_beta",RooArgSet(nuis_var,beta_var));
    RooArgList nuiList(nuis_times_beta);
    vector<int> code;
    code.push_back(4); // Poly interpolation
      
    if(syst->errHiExpr.IsFloat()&&syst->errLoExpr.IsFloat()){
      // Both upper and lower uncertainties are numbers. let's just use FlexibleInterpVar
      vector<double> sigma_var_high, sigma_var_low;

      sigma_var_high.push_back( syst->nominal+syst->errHiExpr.Atof());
      sigma_var_low.push_back( syst->nominal/(syst->nominal+syst->errLoExpr.Atof())); // Use the log normal convention. In asymmetric case honestly the constraint pdf is unknown in most cases...
      
      RooStats::HistFactory::FlexibleInterpVar expected_var(responseName,responseName,nuiList,syst->nominal,sigma_var_low,sigma_var_high,code);
      w->import(expected_var,Silence());
    }
    else{
      // We will need to upgrade FlexibleInterpVar
      TString nominal_expr=implementObj(w, "nominal_"+varName+Form("[%f]", syst->nominal));
      
      TString uncertHiName=implementUncertExpr(w, syst->errHiExpr, varName, auxUtil::UPERROR);
      TString uncertLoName=implementUncertExpr(w, syst->errLoExpr, varName, auxUtil::LOERROR);

      TString variationHiName=implementObj(w, "expr::"+VARIATIONHIPREFIX+varName+"('@0+@1', "+nominal_expr+", "+uncertHiName+")");
      TString variationLoName=implementObj(w, "expr::"+VARIATIONLOPREFIX+varName+"('@0/(@0+@1)', "+nominal_expr+", "+uncertLoName+")");

      RooArgList sigma_expr_high, sigma_expr_low;
      sigma_expr_low.add(*w->arg(variationLoName));
      sigma_expr_high.add(*w->arg(variationHiName));
    
      FlexibleInterpVarMkII expected_var(responseName,responseName,nuiList,syst->nominal,sigma_expr_low, sigma_expr_high, code);

      w->import(expected_var, Silence());
    }
  }
  else{
    TString nominal_expr=implementObj(w, "nominal_"+varName+Form("[%f]", syst->nominal));
    TString NPName=implementObj(w, syst->NPName+"[ 0, -5, 5 ]", true); // Sometimes the duplicated variable cannot be recycled
    TString nuis_times_beta_expr=implementObj(w, "prod::"+varName+"_times_beta("+syst->NPName+", beta_"+varName+Form("[%f])", syst->beta));
    
    if(syst->constrTerm==GAUSSIAN){
      // The reason we need to keep plain implementation for Gaussian uncertainty is mainly due to spurious signal.
      // If we use FlexibleInterpVar, the response will be truncated at 0, but we need the response to go negative.
      if(_debug) cout << "\tREGTEST: Set up nuisance parameter "
		      << syst->NPName << " as gaussian uncertainty on process "<< syst->process <<endl;
      TString uncertName=implementUncertExpr(w, syst->errHiExpr, varName, auxUtil::SYMMERROR);
      TString uncert_wrapper_expr="prod::uncert_"+GAUSSIAN+"_"+varName+"("+nuis_times_beta_expr+", "+uncertName+")";
      TString expected_expr="sum::"+responseName+"("+nominal_expr+", "+uncert_wrapper_expr+")";
      implementObj(w, expected_expr);
    }
    else if(syst->constrTerm==LOGNORMAL){
      if(_debug) cout << "\tREGTEST: Set up nuisance parameter "
		      << syst->NPName << " as lognormal uncertainty on process "<< syst->process <<endl;
      TString uncertName=implementUncertExpr(w, syst->errHiExpr, varName, auxUtil::SYMMERROR);
      TString log_kappa_expr="expr::log_kappa_"+varName+"('log(1+@0/@1)', "+uncertName+", "+nominal_expr+")";
      
      TString uncert_wrapper_expr="expr::uncert_"+LOGNORMAL+"_"+varName+"('exp(@0*@1)',"+nuis_times_beta_expr+", "+log_kappa_expr+")";
      TString expected_expr="prod::"+responseName+"("+uncert_wrapper_expr+", "+nominal_expr+")";
      implementObj(w, expected_expr);
    }
    else auxUtil::alertAndAbort("Unknown constraint type "+syst->constrTerm+" for NP "+syst->NPName+" in process "+syst->process
			      +". Choose from \""+LOGNORMAL+"\" (lognormal), \""+GAUSSIAN+"\" (gaussian) or \""+ASYMMETRIC+"\" (asymmetric)");
  }
  if(_debug) cout<<"\tREGTEST: Creating RooGaussian constraint term for NP "<<syst->NPName<<endl;
  implementObj(w, "RooGaussian::"+constrName+"("+syst->NPName+","+globName+"[0,-5,5],1)", true);
  
  nuispara->add(*w->var(syst->NPName),true);
  constraints->add(*w->pdf(constrName),true);
  globobs->add(*w->var(globName),true);
  expected->add(*w->function(responseName),true);
  
  if(_debug) cout<<"REGTEST: finished implementing systematic "<<syst->NPName<<endl;
}

void xmlAnaWSBuilder::getModel(RooWorkspace *w, Sample *sample, RooArgSet *nuispara, RooArgSet *constraints, RooArgSet *globobs){
  if(_debug) cout<<"Entering function getModel"<<endl;
  bool isSharedPdf=(sample->sharePdfGroup!="");
  TString tagName=(isSharedPdf?sample->sharePdfGroup:sample->procName);
  
  sample->modelName=PDFPREFIX+tagName; // Final pdf name

  if(bool(w->pdf(sample->modelName))){
    if(isSharedPdf){
      if(_debug) cout<<"\tREGTEST: PDF "<<sample->modelName<<" has been created in the workspace."<<endl;
      return; // Use shared pdf
    }
    else auxUtil::alertAndAbort("PDF "+sample->modelName+" already exists but the user asks to create it again");
  }
  
  if(_debug) cout<<sample->modelName<<endl;
  if(_categoryType==COUNTING){
    // In counting experiment we only need a uniform pdf
    implementObj(w, "RooUniform::"+sample->modelName+"("+_observableName+")");
  }
  else{
    // Implement a xml parser for implementing all kinds of models
    TString inputFileName=sample->inputFile;
    TDOMParser xmlparser;

    auxUtil::parseXMLFile(&xmlparser, inputFileName);

    TXMLDocument* xmldoc=xmlparser.GetXMLDocument();
    TXMLNode* rootNode=xmldoc->GetRootNode();
    TXMLNode* node=rootNode->GetChildren();

    TString modelType=auxUtil::getAttributeValue(rootNode, "Type");
    modelType.ToLower();
    if(modelType==USERDEF){
      if(_debug) cout<<"\tREGTEST: Creating user-defined pdf from "<<inputFileName<<endl;
      int cacheBinning=atoi(auxUtil::getAttributeValue(rootNode, "CacheBinning", true, "-1"));
      if(cacheBinning>0) w->var(_observableName)->setBins(cacheBinning, "cache"); // For the accuracy of Fourier transformation
      bool isSuccess=false;
      while ( node != 0 ){
	TString nodeName=node->GetNodeName();
	if(_debug) cout<<nodeName<<endl;
	if(nodeName=="Item"){
	  implementObj(w, getItemExpr(node, "Name", tagName));
	} 
	else if(nodeName=="ModelItem"){
	  TString factoryStr=getItemExpr(node, "Name", tagName);
	  TString oldPdfName=auxUtil::getObjName(factoryStr);
	  factoryStr.ReplaceAll(oldPdfName, sample->modelName);
	  implementObj(w, factoryStr);
	  isSuccess=true;
	  break;		// Assume model already constructed. No need to continue;
	}
	else{
	  // cerr<<"ERROR: Unknown node name: "<<nodeName<<endl;
	}
	node=node->GetNextNode();
      }
      if(!isSuccess) auxUtil::alertAndAbort("Cannot find \"ModelItem\" in XML file "+inputFileName);
    }
    else if(modelType==EXTERNAL){
      // In this case a workspace containing the model is used as input
      TString inputWSFileName=auxUtil::getAttributeValue(rootNode, "Input");
      TString wsName=auxUtil::getAttributeValue(rootNode, "WSName");
      TString modelName=auxUtil::getAttributeValue(rootNode, "ModelName");
      TString observableName=auxUtil::getAttributeValue(rootNode, "ObservableName");
      
      cout<<"\tREGTEST: Use existing PDF named as "<<modelName<<" from "<<inputWSFileName<<endl;
      
      unique_ptr<TFile> fExtWS(TFile::Open(inputWSFileName, "read"));
      RooWorkspace *wModel=dynamic_cast<RooWorkspace*>(fExtWS->Get(wsName));
      RooAbsReal *pModel=dynamic_cast<RooAbsReal*>(wModel->function(modelName));
      TString oldStr="", newStr="", doNotTouch=observableName+",";
      
      while ( node != 0 ){
	TString nodeName=node->GetNodeName();
	if(nodeName=="Item"){
	  implementObj(w, getItemExpr(node, "Name", tagName));
	}
	else if(nodeName=="Fix"){
	  TString varName=auxUtil::getAttributeValue(node, "Name");
	  TString valName=auxUtil::getAttributeValue(node, "Value", true, "default");

	  if(valName.IsFloat()) auxUtil::setValAndFix(wModel->var(varName), valName.Atof());
	}
	else if(nodeName=="Rename"){
	  // Rename the object names in the input workspace
	  TString oldName=auxUtil::combineName(auxUtil::getAttributeValue(node, "OldName"), tagName);
	  TString newName=getItemExpr(node, "NewName", tagName);
	  oldStr+=oldName+",";
	  newStr+=newName+",";
	}
	else if(nodeName=="ExtSyst"){
	  // Adding external systematics
	  TString NPName=auxUtil::getAttributeValue(node, "NPName");
	  TString GOName=auxUtil::getAttributeValue(node, "GOName", true, "");
	  TString constrName=auxUtil::getAttributeValue(node, "ConstrName", (GOName==""), "");
	  doNotTouch+=NPName+",";
	  if(GOName!=""){
	    if(!wModel->var(NPName)||!wModel->var(GOName)||!wModel->pdf(constrName)){
	      auxUtil::alertAndAbort("Something is wrong with "+NPName+" "+GOName+" "+constrName);
	    }
	    TString newConstrName=CONSTRTERMPREFIX+NPName;
	    TString newGOName=GLOBALOBSPREFIX+NPName;
	    w->import(*wModel->pdf(constrName), RenameVariable(constrName+","+GOName, newConstrName+","+newGOName));
	    if(_debug){
	      cout<<w->var(NPName)<<" "<<w->pdf(newConstrName)<<" "<<w->var(newGOName)<<endl;
	      cout<<"\tREGTEST: import systematics "<<NPName<<endl;
	    }
	    constraints->add(*w->pdf(newConstrName),true);
	    globobs->add(*w->var(newGOName),true);
	  }
	  w->var(NPName)->setConstant(false);
	  nuispara->add(*w->var(NPName),true);
	}
	else{
	  // cerr<<"ERROR: Unknown node name: "<<nodeName<<endl;
	}
	node=node->GetNextNode();
      }

      // N.B. we need to rename everything in this model by adding a tag to it
      RooWorkspace wTemp("wTemp");
      wTemp.import(*pModel, RenameAllNodes(tagName), RenameAllVariablesExcept(tagName, doNotTouch), Silence());
      // wTemp.importClassCode();
      if(_debug) wTemp.Print();
      
      pModel=dynamic_cast<RooAbsReal*>(wTemp.function(auxUtil::combineName(modelName, tagName))->Clone(sample->modelName));
      // observableName=auxUtil::combineName(observableName, tagName);
      
      RooRealVar *observable=wTemp.var(observableName);

      if(observableName!=_observableName) {oldStr+=observableName; newStr+=_observableName;}
      
      cout<<"\tREGTEST: The following variables will be renamed:"<<endl;
      cout<<"\tREGTEST: OLD: "<<oldStr<<endl;
      cout<<"\tREGTEST: NEW: "<<newStr<<endl;

      RooAbsPdf *pModelPdf=dynamic_cast<RooAbsPdf*>(pModel);
      
      if(!pModelPdf){
	cout<<"\tWARNING: The object is not a p.d.f as seen from RooFit"<<endl;
	// This is a function, not a PDF (presumably from histFactory). We need to convert it into a PDF
	cout<<"\tREGTEST: Constructing RooRealSumPdf for the pdf from histFactory"<<endl;
	RooUniform dummypdf(sample->modelName+"dummy_pdf","For RooRealSumPdf construction", *observable);
	RooRealVar frac(sample->modelName+"real_pdf_frac","For RooRealSumPdf construction",1);
	pModelPdf=new RooRealSumPdf(sample->modelName,"from histFactory",RooArgList(*pModel, dummypdf), RooArgList(frac));
      }

      w->import(*pModelPdf, RenameVariable(oldStr, newStr), RecycleConflictNodes(), Silence());
    }
    else if(modelType==HISTOGRAM){
      // Create a signal with simple histogram
      TString inputHistFileName=auxUtil::getAttributeValue(rootNode, "Input");
      TString modelName=auxUtil::getAttributeValue(rootNode, "ModelName");
      int rebin=atoi(auxUtil::getAttributeValue(rootNode, "Rebin", true, "-1"));
      unique_ptr<TFile> fExtHist(TFile::Open(inputHistFileName, "read"));

      TH1 *h=dynamic_cast<TH1*>(fExtHist->Get(modelName));
      if(rebin>0) h->Rebin(rebin);
      
      RooDataHist hdata("hdata","hdata",*w->var(_observableName), h);

      RooHistPdf hpdf(sample->modelName,sample->modelName,*w->var(_observableName),hdata);

      w->import(hpdf);
    }
    else auxUtil::alertAndAbort("Unknown model type: "+modelType
				+". Choose from \""+USERDEF+"\", \""+EXTERNAL+"\", \""+HISTOGRAM+"\"");
  }
}

void xmlAnaWSBuilder::attachConstraints(RooWorkspace *w, TString sumPdfName, RooArgSet *constraints, TString finalModelName){
  RooAbsPdf *pdf=w->pdf(sumPdfName);
  unique_ptr<TIterator> iter(constraints->createIterator());
  RooAbsPdf *parg=NULL;
  while((parg=dynamic_cast<RooAbsPdf*>(iter->Next()))){
    TString constrName=parg->GetName();
    TString NPName=constrName.ReplaceAll(CONSTRTERMPREFIX, "");

    if(!pdf->getVariables()->find(NPName)){
      cerr<<"\tWARNING: Constraint term "<<parg->GetName()<<" with NP "<<NPName<<" is redundant in channel \""<<_categoryName<<"\". It will be removed..."<<endl;
      constraints->remove(*parg);
    }
    else w->import(*parg, Silence());
  }
  TString modelStr=auxUtil::generateExpr("PROD::"+finalModelName+"("+sumPdfName+",", constraints);
  implementObj(w, modelStr);
}

void xmlAnaWSBuilder::checkNuisParam(RooAbsPdf *model, RooArgSet *nuispara){
  RooArgSet *set=model->getVariables();
  unique_ptr<TIterator> iter(set -> createIterator());
  RooRealVar* parg = NULL;
  RooArgSet floatSet;
  while((parg=dynamic_cast<RooRealVar*>(iter->Next()))){
    if(!parg->isConstant()){
      // Only check floating variables
      if(parg->getMax()==parg->getMin()){
      	cout<<"\tREGTEST: fixing "<<parg->GetName()<<" to constant as it has same upper and lower boundary"<<endl;
      	parg->setConstant(true);
      }
      else 
	floatSet.add(*parg);
    }
  }
  // Remove POI and observable from the floatSet
  int nPOI=_POIList.size();
  for(int ipoi=0;ipoi<nPOI;ipoi++){
    if(floatSet.find(_POIList[ipoi])) floatSet.remove(*floatSet.find(_POIList[ipoi]));
  }
  if(floatSet.find(_observableName)) floatSet.remove(*floatSet.find(_observableName));
  
  int nNP=nuispara->getSize();

  iter.reset(floatSet.createIterator());
  while((parg=dynamic_cast<RooRealVar*>(iter->Next()))){
    if(!nuispara->find(*parg)){
      nuispara->add(*parg);
      cout<<"\tREGTEST: Adding "<<parg->GetName()<<" to the nuisance parameter set"<<endl;
    }
  }

  if(floatSet.getSize()<nNP){
    // There are redundant nuisance parameters in the model, which we do not care...
    cerr<<"\tWARNING: There supposed to be "<<nPOI+nNP<<" free parameters, but only seen "<<floatSet.getSize()<<" in the channel "<<_categoryName<<endl;
    cout<<"\tIn principle not a issue, but please make sure you understand what you are doing."<<endl;
    if(_debug){
      cout<<"++++++++++++++++++++++ all the free parameters ++++++++++++++++++++++"<<endl;
      floatSet.Print();
      cout<<"++++++++++++++++++++++ all the nuisance parameters ++++++++++++++++++++++"<<endl;
      nuispara->Print();
      cout<<"++++++++++++++++++++++ end of printout ++++++++++++++++++++++"<<endl;
    }
  }
  else{
    cout<<"\tREGTEST: Number of nuisance parameters looks good!"<<endl;
  }
}

TString xmlAnaWSBuilder::getItemExpr(TXMLNode *node, TString attrName, TString process){
  TString expr=auxUtil::getAttributeValue(node, attrName);
  translateKeyword(expr);
  if(expr.Contains(PROCESS)){
    if(process=="") auxUtil::alertAndAbort("Process name not provided for expression "+expr);
    expr.ReplaceAll(PROCESS, "_"+process);
  }
  return expr;
}

RooDataSet* xmlAnaWSBuilder::readInData(RooRealVar *x, RooRealVar *w){
  RooArgSet obs_plus_wt;

  obs_plus_wt.add(*w);
  obs_plus_wt.add(*x);
  
  RooDataSet* obsdata=new RooDataSet(OBSDSNAME,OBSDSNAME,obs_plus_wt,WeightVar(*w));

  if(_categoryType==COUNTING && _numData>=0){ // Generate number of events for counting experiment
    double binCenter=(x->getMin()+x->getMax())/2.;
    double weight=(_numData==0)?auxUtil::epsilon/1000.:_numData;
    x->setVal(binCenter);
    w->setVal(weight);
    obsdata->add( RooArgSet(*x, *w), weight);
  }
  else{
    unique_ptr<RooDataSet> obsdata_tmp;

    if(_inputDataFileType==ASCII){
      obsdata_tmp.reset(RooDataSet::read(_inputDataFileName, RooArgList(*x)));
    }
    else if(_inputDataFileType==HISTOGRAM){
      unique_ptr<TFile> f(TFile::Open(_inputDataFileName));
      TH1* h=dynamic_cast<TH1*>(f->Get(_inputDataHistName));
      obsdata_tmp.reset(auxUtil::histToDataSet(h, x, w));
    }
    else{
      unique_ptr<TFile> f(TFile::Open(_inputDataFileName));
      TTree *t=dynamic_cast<TTree*>(f->Get(_inputDataTreeName));
      if(_Cut!="") t=t->CopyTree(_Cut);
      RooRealVar x_tree(_inputDataVarName,_inputDataVarName, _xMin, _xMax);
      obsdata_tmp.reset(new RooDataSet(OBSDSNAME+"_tmp",OBSDSNAME+"_tmp", RooArgSet(x_tree), Import(*t)));
      f->Close();
    }

    RooArgSet* obs_tmp = const_cast<RooArgSet*>(obsdata_tmp->get());
    RooRealVar* xdata_tmp = dynamic_cast<RooRealVar*>(obs_tmp->first()); // We only have one observable in total, so it is okay
  
    for (int i=0 ; i<obsdata_tmp->numEntries() ; i++) {
      obsdata_tmp->get(i) ;
      x->setVal(xdata_tmp->getVal());
      double weight=obsdata_tmp->weight();
      w->setVal(weight);
      if(_goBlind && x->getVal()>_blindMin && x->getVal()<_blindMax) continue;
      obsdata->add( RooArgSet(*x, *w), weight);
    }
  }
  if(_debug) obsdata->Print("v");
  return obsdata;
}

TString xmlAnaWSBuilder::implementObj(RooWorkspace *w, TString expr, bool checkExistBeforeImp){
  // If the obj is claimed to exist, but actually not, then abort.
  int type=auxUtil::getItemType(expr);
  if(type==auxUtil::EXIST){
    if(!w->arg(expr)) auxUtil::alertAndAbort("object "+expr+" does not exist");
    else return expr;
  }
  
  // If the variable is believed to have some chance to exist, we should check.
  TString varName=auxUtil::getObjName(expr);
  if(checkExistBeforeImp){
    if(w->arg(varName)){
      if(_debug) cout<<"\tREGTEST: object "<<varName<<" already exists"<<endl;
      return varName;
    }
  }

  // Otherwise we just blindly implement
  if(_debug) cout<<"\tREGTEST: Generating "<<auxUtil::translateItemType(type)<<" "<<expr<<endl;
  if(!w->factory(expr)) auxUtil::alertAndAbort("Creation of expression "+expr+" failed");
  
  return varName;
}

TString xmlAnaWSBuilder::implementObjArray(RooWorkspace *w, vector<TString> objArr){
  TString outputStr="";
  for(auto item : objArr) outputStr+=implementObj(w, item)+",";
  return outputStr.Chop();
}

TString xmlAnaWSBuilder::implementUncertExpr(RooWorkspace *w, TString expr, TString varName, int uncertType){
  if(expr.IsFloat()) return implementObj(w, auxUtil::translateUncertType(uncertType)+varName+"["+expr+"]");
  else return implementObj(w, expr);
}

void xmlAnaWSBuilder::readChannelXMLNode(TXMLNode *node){
  while ( node != 0 ){
    if ( node->GetNodeName() == TString( "Item" ) ){
      TString item=getItemExpr(node, "Name");
      if(item.Contains(RESPONSE) || item.Contains(RESPONSEPREFIX)) _ItemsLowPriority.push_back(item);
      else _ItemsHighPriority.push_back(item);
    }
    
    else if ( node->GetNodeName() == TString( "Systematic" ) ){
      readSyst(node, ALLPROC);
    }

    else if ( node->GetNodeName() == TString( "Sample" ) ){
      readSample(node);
    }

    else if ( node->GetNodeName() == TString( "ImportItems" ) || node->GetNodeName() == TString( "IncludeSysts" ) ){
      TString inputFileName=auxUtil::getAttributeValue(node, "FileName");
      TDOMParser xmlparser;
      auxUtil::parseXMLFile(&xmlparser, inputFileName);
      cout<<"\tREGTEST: Importing Items from " << inputFileName << endl;

      TXMLDocument* xmldoc=xmlparser.GetXMLDocument();
      TXMLNode* rootNode=xmldoc->GetRootNode();
      TXMLNode* importNode=rootNode->GetChildren();
      readChannelXMLNode( importNode );
    }
    node=node->GetNextNode();
  }
}

void xmlAnaWSBuilder::dataFileSanityCheck(){
  if(_categoryType==COUNTING){
    if(_inputDataFileName!="" && !auxUtil::checkExist(_inputDataFileName)) auxUtil::alertAndAbort("Cannot find input data file "+_inputDataFileName);
    else if(_inputDataFileName=="" && _numData<0) auxUtil::alertAndAbort("Please either provide a input data file, or provide a valid number of data events for a counting experiment");
  }
  else{
    if(!auxUtil::checkExist(_inputDataFileName)) auxUtil::alertAndAbort("Cannot find input data file "+_inputDataFileName);
    if(_inputDataFileType!=ASCII){	// means that we are reading data from a tree
      TFile *f=TFile::Open(_inputDataFileName);
      if(_inputDataFileType==HISTOGRAM){
	TH1 *h=dynamic_cast<TH1*>(f->Get(_inputDataHistName));
	if(!h) auxUtil::alertAndAbort("Cannot find TH1 "+_inputDataHistName+" in data file "+_inputDataFileName);
      }
      else{
	TTree *t=dynamic_cast<TTree*>(f->Get(_inputDataTreeName));
	if(!t) auxUtil::alertAndAbort("Cannot find TTree "+_inputDataTreeName+" in data file "+_inputDataFileName);
	TBranch *b=t->FindBranch(_inputDataVarName);
	if(!b) auxUtil::alertAndAbort("Cannot find TBranch "+_inputDataVarName+" in TTree "+_inputDataTreeName+" in data file "+_inputDataFileName);
      }
      f->Close();
    }
  }
}

void xmlAnaWSBuilder::translateKeyword(TString &expr){
  expr.ReplaceAll(RESPONSE, RESPONSEPREFIX+SHAPE+"_"); // Implement proper response terms. Assume only shape uncertainty would appear
  expr.ReplaceAll(OBSERVABLE, _observableName); // Implement proper observables
  expr.ReplaceAll(CATEGORY, _categoryName); // Category name
  expr.ReplaceAll(LT,"<");
  expr.ReplaceAll(LE,"<=");
  expr.ReplaceAll(GT,">");
  expr.ReplaceAll(GE,">=");
  expr.ReplaceAll(AND,"&&");
  expr.ReplaceAll(OR,"||");
}

void xmlAnaWSBuilder::Summary(TString outputFigName){
  RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
  gErrorIgnoreLevel=kWarning;
  vector<TString> options=auxUtil::splitString(_plotOpt,',');
  int m_rebin=1;
  bool m_logy=false;
  for(auto opt : options){
    if(opt.Contains("rebin")){
      m_rebin=atoi(opt(opt.First("rebin")+5,opt.Length()).Data());
      cout<<auxUtil::WARNING<<"Rebin with "<<m_rebin<<auxUtil::ENDC<<endl;
    }
    if(opt.Contains("logy")) m_logy=true;
  }
  
  RooSimultaneous *m_pdf = dynamic_cast<RooSimultaneous*>(_mConfig->GetPdf()); assert (m_pdf);
  RooAbsCategoryLValue* m_cat = const_cast<RooAbsCategoryLValue*>(&m_pdf->indexCat());
  const RooArgSet *m_gobs = dynamic_cast<const RooArgSet*>(_mConfig->GetGlobalObservables()); assert(m_gobs);
  int numChannels = m_cat->numBins(0);
  RooDataSet *m_data=dynamic_cast<RooDataSet*>(_combWS->data(_dataName));
  TList *m_dataList = m_data->split( *m_cat, true );

  auxUtil::printTitle("Begin summary", "~");
  cout << "\tThere are " << numChannels << " sub channels:" << endl;
  TCanvas c("summary","summary",800,600);
  
  c.Print(outputFigName+"[");
  for ( int i= 0; i < numChannels; i++ ) {
    m_cat->setBin(i);
    TString channelname=m_cat->getLabel();
    RooAbsPdf* pdfi = m_pdf->getPdf(channelname);
    RooDataSet* datai = ( RooDataSet* )( m_dataList->At( i ) );
    cout << "\t\tIndex: " << i << ", Pdf: " << pdfi->GetName() << ", Data: " << datai->GetName() << ", SumEntries: " << datai->sumEntries() << endl;

    RooRealVar *x=dynamic_cast<RooRealVar*>(pdfi->getObservables(datai)->first());
    int obsNBins = x->numBins()/m_rebin;
    unique_ptr<RooPlot> frame(x->frame());
    datai->plotOn(frame.get(), DataError(RooAbsData::Poisson), RooFit::Binning(obsNBins));

    double addSF=1;
    if(_goBlind && _rangeList[i]!=""){
      pdfi->plotOn(frame.get(), LineColor(kBlue), LineStyle(2), NormRange(_rangeList[i]));
      pdfi->plotOn(frame.get(), LineColor(kBlue), LineStyle(1), Range(_rangeList[i]), NormRange(_rangeList[i]));
      addSF=pdfi->createIntegral(RooArgSet(*x), NormSet(*x))->getVal()/pdfi->createIntegral(RooArgSet(*x), NormSet(*x), Range(_rangeList[i]))->getVal();
    }
    else pdfi->plotOn(frame.get(), LineColor(kBlue));

    c.cd();
    TPad pad1("pad1_"+channelname,"pad1",0.01,0.40,0.99,0.99);
    TPad pad2("pad2_"+channelname,"pad2",0.01,0.05,0.99,0.402);
    pad1.Draw();
    pad2.Draw();
    pad1.SetBottomMargin(0.);
    pad1.cd();
    frame->SetMinimum(auxUtil::epsilon*1000);
    if(m_logy){
      frame->SetMinimum(1e-1);
      c.SetLogy();
    }
    
    frame->SetLabelFont(43,"y");
    frame->SetLabelOffset(0.005,"y");
    frame->SetLabelSize(14,"y");
    frame->SetTitleFont(43,"y");
    frame->SetTitleOffset(1.2,"y");
    frame->SetTitleSize(17,"y");
    frame->SetXTitle("");
    frame->Draw();

    pad2.SetTopMargin(0.);
    pad2.SetBottomMargin(0.25);
    pad2.cd();
    
    TH1D hsub("hsub_"+channelname,"hsub",obsNBins,x->getMin(),x->getMax()); 
    hsub.Sumw2();

    unique_ptr<TH1D> hbkg((TH1D*)pdfi->createHistogram("hbkg", *x, RooFit::Binning(obsNBins)));
    unique_ptr<TH1D> hdata((TH1D*)datai->createHistogram("hdata", *x, RooFit::Binning(obsNBins)));
    for(int ibin=1;ibin<=obsNBins;ibin++) hdata->SetBinError(ibin, sqrt(hdata->GetBinContent(ibin)));
    hbkg->Scale(hdata->Integral()/hbkg->Integral("width")*addSF);

    for( int i = 0 ; i < obsNBins; i ++ ){
      double weight = hbkg->GetBinContent(i+1);
      double value = hdata->GetBinContent(i+1);
      double error = hdata->GetBinError(i+1);
      if(value>0.5) {
	hsub.SetBinContent(i+1,value/weight);
	hsub.SetBinError(i+1,error/weight);
      }
    }
    hsub.GetYaxis()->SetRangeUser(0.801,1.199);
    hsub.SetMarkerStyle(kFullCircle);
    hsub.SetMarkerColor(kBlack);
    hsub.SetMarkerSize(1.);
    hsub.SetStats(kFALSE);
    hsub.SetTitle("");
    hsub.SetLineColor(kBlack);
    
    hsub.GetYaxis()->SetTitle("Data / Fit ");
    hsub.GetYaxis()->SetTitleFont(43);
    hsub.GetYaxis()->SetTitleSize(17);
    hsub.GetYaxis()->SetTitleOffset(1.3);
    hsub.GetYaxis()->SetLabelFont(43);
    hsub.GetYaxis()->SetLabelSize(14);

    hsub.GetXaxis()->SetTitleFont(43);
    hsub.GetXaxis()->SetTitleSize(17);
    // TString xAxisTitle(x->GetTitle()) ;
    hsub.GetXaxis()->SetTitle(Form("%s [%s]", x->GetTitle(), x->getUnit()));
    hsub.GetXaxis()->SetTitleOffset(3.5);
    hsub.GetXaxis()->SetLabelFont(43);
    hsub.GetXaxis()->SetLabelSize(14);
    hsub.GetXaxis()->SetLabelOffset(0.016);
    hsub.SetLineWidth(1);

    unique_ptr<RooPlot> frameS(x->frame());
    hsub.Draw();

    TLine l(x->getMin(),1,x->getMax(),1);
    l.SetLineColor(kCyan+2);
    l.SetLineWidth(2);
    l.Draw("same");

    TLine ldown(x->getMin(),1.1,x->getMax(),1.1);
    ldown.SetLineColor(kCyan+1);
    ldown.SetLineWidth(1);
    ldown.SetLineStyle(kDashed);
    ldown.Draw("same");

    TLine lup(x->getMin(),0.9,x->getMax(),0.9);
    lup.SetLineColor(kCyan+1);
    lup.SetLineWidth(1);
    lup.SetLineStyle(kDashed);
    lup.Draw("same");

    hsub.Draw("same");
    frameS->Draw("same");

    c.Print(outputFigName);
  }
  c.Print(outputFigName+"]");
  
  auxUtil::printTitle("POI");
  _mConfig->GetParametersOfInterest()->Print("v");
  if(_debug){
    auxUtil::printTitle("Nuisance parameters");
    _mConfig->GetNuisanceParameters()->Print();
    auxUtil::printTitle("Global observables");
    _mConfig->GetGlobalObservables()->Print();
  }
  auxUtil::printTitle("Dataset");
  list<RooAbsData*> allData=_combWS->allData();
  for (list<RooAbsData*>::iterator data = allData.begin(); data != allData.end(); data++) {
    (*data)->Print();
  }

  auxUtil::printTitle("End summary", "~");
}
