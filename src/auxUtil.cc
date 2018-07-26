#include "auxUtil.hh"

ClassImp(auxUtil)

double auxUtil::epsilon=1e-6;

TString auxUtil::UNCERTHIPREFIX="uncertHi__";
TString auxUtil::UNCERTLOPREFIX="uncertLo__";
TString auxUtil::UNCERTSYMMPREFIX="uncertSymm__";

void auxUtil::printTitle(const char* titleText, int width, TString separator){
  TString line="";
  for(int i=0;i<width;i++) line+=separator;
  TString fullline=line+line+line+line;
  cout<<TString("\t "+fullline)<<endl;
  printf("\t "+line+"%*s%*s"+line+"\n",int(width+strlen(titleText)/2),titleText,int(width-strlen(titleText)/2),"");
  cout<<TString("\t "+fullline)<<endl;
}

void auxUtil::Reset(RooArgSet* original, RooArgSet* snapshot){
  *original=*snapshot;
  // Still need to recover the ranges for variables
  unique_ptr<TIterator> iter(original->createIterator());
  RooRealVar* parg = NULL;  
  while((parg=dynamic_cast<RooRealVar*>(iter->Next()))){
    RooRealVar *snapVar=dynamic_cast<RooRealVar*>(snapshot->find(parg->GetName()));
    parg->setRange(snapVar->getMin(), snapVar->getMax());
  }
}

int auxUtil::parseXMLFile(TDOMParser *xmlparser, TString inputFile){
  Int_t parseError = xmlparser->ParseFile( inputFile );

  if ( parseError ){
    cerr << "\tERROR: Loading of xml document \"" << inputFile
	      << "\" failed" << endl;
  }
  return parseError;
}

vector<TString> auxUtil::splitString(const TString& theOpt, const char separator )
{
   // splits the option string at 'separator' and fills the list
   // 'splitV' with the primitive strings
   vector<TString> splitV;
   TString splitOpt(theOpt);
   splitOpt.ReplaceAll("\n"," ");
   splitOpt = splitOpt.Strip(TString::kBoth,separator);
   while (splitOpt.Length()>0) {
      if ( !splitOpt.Contains(separator) ) {
         splitV.push_back(splitOpt);
         break;
      }
      else {
         TString toSave = splitOpt(0,splitOpt.First(separator));
         splitV.push_back(toSave);
         splitOpt = splitOpt(splitOpt.First(separator),splitOpt.Length());
      }
      splitOpt = splitOpt.Strip(TString::kLeading,separator);
   }
   return splitV;
}

TString auxUtil::generateExpr(TString head, RooArgSet *set, bool closeExpr){
  TString exprStr=head;
  unique_ptr<TIterator> iter (set->createIterator());
  RooAbsArg* parg = NULL;
  while((parg=dynamic_cast<RooAbsArg*>(iter->Next()))){
    exprStr+=TString(parg->GetName())+",";
  }
  if(closeExpr) closeFuncExpr(exprStr);
  return exprStr;
}

void auxUtil::Summary(ModelConfig *mc, TString dataName, TString outputFigName, TString plotOption, bool verbose){
  RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
  gErrorIgnoreLevel=kWarning;
  RooWorkspace *w=mc->GetWS();
  RooSimultaneous *m_pdf = dynamic_cast<RooSimultaneous*>(mc->GetPdf()); assert (m_pdf);
  RooAbsCategoryLValue* m_cat = const_cast<RooAbsCategoryLValue*>(&m_pdf->indexCat());
  const RooArgSet *m_gobs = dynamic_cast<const RooArgSet*>(mc->GetGlobalObservables()); assert(m_gobs);
  int numChannels = m_cat->numBins(0);
  RooDataSet *m_data=dynamic_cast<RooDataSet*>(w->data(dataName));
  TList *m_dataList = m_data->split( *m_cat, true );

  RooRealVar *firstPOI=dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
  double muhat=firstPOI->getVal();
  int width=10;
  printTitle("Begin summary",width,"~");
  cout << "\tThere are " << numChannels << " sub channels:" << endl;
  TCanvas c("summary","summary",800,600);
  
  c.Print(outputFigName+"[");
  for ( int i= 0; i < numChannels; i++ ) {
    m_cat->setBin(i);
    TString channelname=m_cat->getLabel();
    RooAbsPdf* pdfi = m_pdf->getPdf(channelname);
    RooDataSet* datai = ( RooDataSet* )( m_dataList->At( i ) );
    cout << "\t\tIndex: " << i << ", Pdf: " << pdfi->GetName() << ", Data: " << datai->GetName()  << ", SumEntries: " << datai->sumEntries() << endl;

    RooRealVar *x=dynamic_cast<RooRealVar*>(pdfi->getObservables(datai)->first());
    unique_ptr<RooPlot> frame(x->frame());
    datai->plotOn(frame.get(), DataError(RooAbsData::Poisson));
    firstPOI->setVal(0);
    pdfi->plotOn(frame.get(), LineColor(kBlue), Normalization(1.0,RooAbsReal::RelativeExpected));
    if(plotOption.Contains("ucmles")){
      w->loadSnapshot("ucmles");
    }
    else firstPOI->setVal(1);
    pdfi->plotOn(frame.get(), LineColor(kRed), Normalization(1.0,RooAbsReal::RelativeExpected));
    c.cd();
    frame->SetMinimum(auxUtil::epsilon*10);
    if(plotOption.Contains("logy")){
      frame->SetMinimum(1e-1);
      c.SetLogy();
    }
    frame->Draw();
    c.Print(outputFigName);
  }
  c.Print(outputFigName+"]");
  firstPOI->setVal(muhat);
  printTitle("POI",width);
  mc->GetParametersOfInterest()->Print("v");
  if(verbose){
    printTitle("Nuisance parameters",width);
    mc->GetNuisanceParameters()->Print();
    printTitle("Global observables",width);
    mc->GetGlobalObservables()->Print();
  }
  printTitle("Dataset",width);
  list<RooAbsData*> allData=w->allData();
  for (list<RooAbsData*>::iterator data = allData.begin(); data != allData.end(); data++) {
    (*data)->Print();
  }

  printTitle("End summary",width,"~");
}

void auxUtil::defineSet(RooWorkspace *w, RooArgSet set, TString setName){
  unique_ptr<TIterator> iter(set.createIterator());
  RooRealVar* parg = NULL;
  RooArgSet nameSet;
  while((parg=dynamic_cast<RooRealVar*>(iter->Next()))){
    if(w->var(parg->GetName())) nameSet.add(*w->var(parg->GetName()));
  }

  w->defineSet(setName, nameSet);
}

void auxUtil::defineSet(RooWorkspace *w, vector<TString> set, TString setName){
  RooArgSet nameSet;
  for(vector<TString>::iterator it = set.begin(); it != set.end(); ++it){
    if(w->var(*it)) nameSet.add(*w->var(*it));
  }

  w->defineSet(setName, nameSet);
}

void auxUtil::releaseTheGhost(RooDataSet *obsdata, RooRealVar *x, RooRealVar *w, double ghostwt){
  TH1D h_data("h_data","",x->numBins(),x->getMin(),x->getMax());
  RooArgSet* obs = const_cast<RooArgSet*>(obsdata->get());
  RooRealVar* xdata = dynamic_cast<RooRealVar*>(obs->find(x->GetName()));
  int nevt1=obsdata->numEntries();
  for (int i=0 ; i<nevt1 ; i++) {
    obsdata->get(i) ;
    h_data.Fill( xdata->getVal() );
  }
  for( int ibin = 1 ; ibin <= x->numBins() ; ibin++) {
    if(h_data.GetBinContent(ibin)==0){
      x->setVal(h_data.GetBinCenter(ibin));
      w->setVal(ghostwt);
      obsdata->add( RooArgSet(*x,*w), ghostwt);
    }
  }
}

int auxUtil::getItemType(TString item){
  if(item.Contains("::")&&item.Contains("(")) return FUNCTION; // Function or pdf
  else if(item.Contains("[")) return VARIABLE;			// Variable
  else return EXIST;					// Existing obj in workspace
}

TString auxUtil::translateItemType(int type){
  if(type==FUNCTION) return "function";
  else if(type==VARIABLE) return "variable";
  else if(type==EXIST) return "existing object";
  else return "unknown";
}

TString auxUtil::translateUncertType(int type){
  if(type==UPERROR) return UNCERTHIPREFIX;
  else if(type==LOERROR) return UNCERTLOPREFIX;
  else if(type==SYMMERROR) return UNCERTSYMMPREFIX;
  else return "unknown";
}

TString auxUtil::getObjName(TString objName){
  int type=getItemType(objName);
  if(type==EXIST) return objName;
  
  if(type==VARIABLE){
    objName=objName(0,objName.First('['));
    // cout<<"\tREGTEST: Getting variable name "<<objName<<endl;
  }
  
  else{
    objName=objName(objName.First("::")+2,objName.First('(')-objName.First("::")-2);
    // cout<<"\tREGTEST: Getting function name "<<objName<<endl;
  }
  return objName;
}

TString auxUtil::getAttributeValue( TXMLNode* rootNode, TString attributeKey, bool allowEmpty, TString defaultStr){
  TListIter attribIt = rootNode->GetAttributes();
  TXMLAttr* curAttr = 0;
  TString attributeValue = "";

  while (( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ){
    if ( curAttr->GetName() == attributeKey ){
      attributeValue = curAttr->GetValue() ;
      break;
    }
  }
  if(attributeValue==""){
    if(allowEmpty) attributeValue=defaultStr;
    else alertAndAbort("Attribute "+attributeKey+" cannot be found in node "+rootNode->GetNodeName());
  }
  removeWhiteSpace(attributeValue);
  return attributeValue;
}

void auxUtil::alertAndAbort(TString msg){
  cerr<<"\tERROR: "<<msg<<". Please intervene..."<<endl;
  exit(-1);
}

void auxUtil::setValAndFix(RooRealVar *var, double value){
  if ( var->getMax() < value ) {
    var->setMax(value + 1);
  } else if ( var->getMin() > value ) {
    var->setMin(value - 1);
  }
  var->setVal(value);
  var->setConstant(true);
}

int auxUtil::stripSign(TString &expr){
  int sign=1;
  if(expr.BeginsWith('-')||expr.BeginsWith('+')){
    if(expr.BeginsWith('-')) sign=-1;
    expr=expr(1,expr.Length());
  }
  return sign;
}

void auxUtil::removeWhiteSpace(TString& item){
  std::string s=item.Data();
  s.erase( std::remove_if( s.begin(), s.end(), std::bind( std::isspace<char>, std::placeholders::_1, std::locale::classic() ) ), s.end() );
  item=s.c_str();
}

vector<TString> auxUtil::decomposeFuncStr(TString function){
  removeWhiteSpace(function);
  vector<TString> itemList=splitString(function(function.First('(')+1,function.Last(')')-function.First('(')-1),',');
  if(function.Contains("expr::")) itemList.erase(itemList.begin()); // TODO: find other special syntax to be taken care of
  return itemList;
}

bool auxUtil::to_bool(TString str) {
  bool b;
  if(str.IsDigit()) b=str.Atoi();
  else{				// Is a string
    str.ToLower();
    std::istringstream is(str.Data());
    is >> std::boolalpha >> b;
  }
  return b;
}

TXMLNode *auxUtil::findNode(TXMLNode* rootNode, TString nodeName){
  TXMLNode* node=rootNode->GetChildren();
  while ( node != 0 ){
    if(nodeName==TString(node->GetNodeName())) return node;
    node=node->GetNextNode();
  }
  return NULL;
}

bool auxUtil::checkExist(TString name) {
  if (FILE *file = fopen(name, "r")) {
    fclose(file);
    return true;
  }
  else return false;
}
