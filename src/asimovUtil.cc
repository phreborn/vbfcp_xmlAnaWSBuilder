#include "asimovUtil.hh"

ClassImp(asimovUtil)

TString asimovUtil::RAW="raw";
TString asimovUtil::FIT="fit";
TString asimovUtil::RESET="reset";
TString asimovUtil::GENASIMOV="genasimov";
TString asimovUtil::FLOAT="float";
TString asimovUtil::FIXSYST="fixsyst";
TString asimovUtil::MATCHGLOB="matchglob";
TString asimovUtil::SAVESNAPSHOT="savesnapshot";

void asimovUtil::generateAsimov(ModelConfig *mc, TString dataName){
  RooWorkspace *w=mc->GetWS();
  int nAsimov=_asimovNames.size();
  RooAbsData *data=w->data(dataName);

  RooArgSet everything;
  auxUtil::collectEverything(mc, &everything);
  w->saveSnapshot(RAW, everything);
  
  for(int iAsimov=0;iAsimov<nAsimov;iAsimov++){
    unique_ptr<RooArgSet> originalSnapshot(dynamic_cast<RooArgSet*>(everything.snapshot()));
    TString fixedVar="";
    auxUtil::printTitle("Operation "+_asimovNames[iAsimov], "+");
    _asimovSetups[iAsimov].ReplaceAll(" ","");
    // Nothing to be done...
    if(_asimovSetups[iAsimov]==""){
      cout<<"\tREGTEST: There is no setup info provided. Proceed with current variable values and ranges..."<<endl;
    }
    // Configuring variables
    else{
      vector<TString> varList=auxUtil::splitString(_asimovSetups[iAsimov],',');
      bool validSetup=true;
      TString badSetup="";
      for(vector<TString>::iterator var = varList.begin(); var != varList.end(); ++var){
	if(!var->Contains("=")){validSetup=false;badSetup=*var;break;}

	TString varName=(*var)(0, var->First('='));
	if(!w->var(varName)){validSetup=false;badSetup=*var;break;}
	
	TString varRange=(*var)(var->First('=')+1, var->Length());
	if(!varRange.Contains('_')){
	  if(!varRange.IsFloat()){validSetup=false;badSetup=*var;break;}
	  cout<<"\tREGTEST: Fixing variable "<<varName<<" to "<<varRange<<endl;
	  auxUtil::setValAndFix(w->var(varName), varRange.Atof());
	  if(mc->GetNuisanceParameters()->find(*w->var(varName))) fixedVar+=varName+","; // We can only float a parameter if it is a nuisance parameter
	}
	else{
	  vector<TString> varSetup=auxUtil::splitString(varRange,'_');
	  if(varSetup.size()!=3||!varSetup[0].IsFloat()||!varSetup[1].IsFloat()||!varSetup[2].IsFloat()){validSetup=false;badSetup=*var;break;}
	  cout<<"\tREGTEST: Floating variable "<<varName<<" to "<<Form("[%f, %f, %f]", varSetup[0].Atof(), varSetup[1].Atof(), varSetup[2].Atof())<<endl;
	  w->var(varName)->setConstant(false);
	  w->var(varName)->setRange(varSetup[1].Atof(),varSetup[2].Atof());
	  w->var(varName)->setVal(varSetup[0].Atof());
	}
      }
      if(!validSetup){
	cerr<<"\tERROR: Invalid setup info: "<<badSetup<<". Stop generating Asimov data "<<_asimovNames[iAsimov]<<endl;
	continue;
      }
    }
    cout<<"\tREGTEST: Action list ("<<_asimovProfiles[iAsimov]<<")"<<endl<<endl;
    vector<TString> actionList=auxUtil::splitString(_asimovProfiles[iAsimov], ':');

    for(vector<TString>::iterator act = actionList.begin(); act != actionList.end(); ++act){
      TString action=*act;
      // Do fit
      int status=-1;
      if(action==FIT){
	if(_dataToFit[iAsimov]!=""){
	  RooAbsData* newData=w->data(_dataToFit[iAsimov]);
	  if(!newData) auxUtil::alertAndAbort("Dataset"+_dataToFit[iAsimov]+" cannot be found in the workspace");
	  status=fitUtil::profileToData(mc, newData, _rangeName);
	}
	else status=fitUtil::profileToData(mc, data, _rangeName);
	if(status!=0&&status!=1){
	  cerr<<"\n\033[91m \tERROR: Fit not converging properly. You may want to investigate more before moving on. Press any key to continue... \033[0m\n"<<endl;
	  getchar();
	}
      }
      // Reset to initial parameter values
      else if(action==RAW) w->loadSnapshot(RAW);
      // Reset to parameter values at beginning of this round
      else if(action==RESET) auxUtil::Reset(&everything, originalSnapshot.get());
      // Float fixed nuisance parameters
      else if(action==FLOAT){
        vector<TString> fixedVarList=auxUtil::splitString(fixedVar,',');
        const RooArgSet *poi = mc->GetParametersOfInterest();
        for(vector<TString>::iterator fixed = fixedVarList.begin(); fixed != fixedVarList.end(); ++fixed){
          if (poi->find(*fixed)) continue;
          // cout<<"\tREGTEST: Releasing variable "<<*fixed<<endl;
          w->var(*fixed)->setConstant(false);
          // w->var(*fixed)->Print();
        }
        fixedVar="";
      }
      // Generating Asimov
      else if(action==GENASIMOV){
	cout<<"\tREGTEST: Generating Asimov dataset "<<_asimovNames[iAsimov]<<endl;
        unique_ptr<RooAbsData> asimovData(AsymptoticCalculator::GenerateAsimovData( *mc->GetPdf(), *mc->GetObservables()));
	// Need to perform injection here.
        w->import(*asimovData, Rename(_asimovNames[iAsimov]));
      }
      // Fix all the constrained nuisance parameters
      else if(action==FIXSYST){
	RooArgSet* constraints=mc->GetPdf()->getAllConstraints(*mc->GetObservables(), *const_cast<RooArgSet*>(mc->GetNuisanceParameters()));
	RooArgSet nuisSyst;
	unique_ptr<TIterator> iter(constraints->createIterator());
	RooAbsPdf *parg=NULL;
	while((parg=dynamic_cast<RooAbsPdf*>(iter->Next()))){
	  RooArgSet *nuisSet=parg->getObservables(*mc->GetNuisanceParameters());
	  if(nuisSet->getSize()==1){
	    RooRealVar *nuis=dynamic_cast<RooRealVar*>(nuisSet->first());
	    fixedVar+=nuis->GetName()+TString(",");
	    nuis->setConstant(true);
	  }
	}
      }
      // Matching global observable to nuisance parameter values
      else if(action==MATCHGLOB) matchGlob(mc);

      // Save snapshot
      else if(action==SAVESNAPSHOT){
	if(_SnapshotsAll[iAsimov]!=""){
	  cout<<"\tREGTEST: Saving snapshot "<<_SnapshotsAll[iAsimov]<<" for current parameters of interest, nuisance parameters, and global observables"<<endl;
	  w->saveSnapshot(_SnapshotsAll[iAsimov], everything);
	  _Snapshots.push_back(_SnapshotsAll[iAsimov]);
	}
	if(_SnapshotsNuis[iAsimov]!=""){
	  cout<<"\tREGTEST: Saving snapshot "<<_SnapshotsNuis[iAsimov]<<" for current nuisance parameters"<<endl;
	  w->saveSnapshot(_SnapshotsNuis[iAsimov], *mc->GetNuisanceParameters());
	  _Snapshots.push_back(_SnapshotsNuis[iAsimov]);
	}
	if(_SnapshotsGlob[iAsimov]!=""){
	  cout<<"\tREGTEST: Saving snapshot "<<_SnapshotsGlob[iAsimov]<<" for current global observables"<<endl;
	  w->saveSnapshot(_SnapshotsGlob[iAsimov], *mc->GetGlobalObservables());
	  _Snapshots.push_back(_SnapshotsGlob[iAsimov]);
	}
	if(_SnapshotsPOI[iAsimov]!=""){
	  cout<<"\tREGTEST: Saving snapshot "<<_SnapshotsPOI[iAsimov]<<" for current parameters of interest"<<endl;
	  w->saveSnapshot(_SnapshotsPOI[iAsimov], *mc->GetParametersOfInterest());
	  _Snapshots.push_back(_SnapshotsPOI[iAsimov]);
	}
      }
      else if(find(_Snapshots.begin(), _Snapshots.end(), action)!=_Snapshots.end()) w->loadSnapshot(action);
      else cerr<<"\tERROR: Unknown action: "<<action<<endl;
    }
  }
}

void asimovUtil::addEntry(TXMLNode *node){
  _asimovNames.push_back(auxUtil::getAttributeValue(node, "Name"));
  _asimovSetups.push_back(auxUtil::getAttributeValue(node, "Setup", true, ""));
  _asimovProfiles.push_back(auxUtil::getAttributeValue(node, "Action", true, ""));
  _SnapshotsAll.push_back(auxUtil::getAttributeValue(node, "SnapshotAll", true, ""));
  _SnapshotsGlob.push_back(auxUtil::getAttributeValue(node, "SnapshotGlob", true, ""));
  _SnapshotsNuis.push_back(auxUtil::getAttributeValue(node, "SnapshotNuis", true, ""));
  _SnapshotsPOI.push_back(auxUtil::getAttributeValue(node, "SnapshotPOI", true, ""));
  _dataToFit.push_back(auxUtil::getAttributeValue(node, "Data", true, ""));
}

void asimovUtil::printSummary(){
  cout<<"Following asimov dataset will be generated: ";
  double nAsimov=_asimovNames.size();
  for(int iAsimov=0;iAsimov<nAsimov;iAsimov++){
    if(_asimovProfiles[iAsimov].Contains(GENASIMOV)) cout<<_asimovNames[iAsimov]<<" ";
  }
  cout<<endl;
}

void asimovUtil::matchGlob(ModelConfig *mc){
  RooArgSet gobs( *mc->GetGlobalObservables() );
  // snapshot data global observables
  RooArgSet snapGlobalObsData;
  RooStats::SetAllConstant( gobs, true );
  gobs.snapshot( snapGlobalObsData );
  RooArgSet nuis( *mc->GetNuisanceParameters() );
  std::unique_ptr<RooAbsPdf> nuispdf( RooStats::MakeNuisancePdf( *mc, "nuisancePdf" ) );
  RooProdPdf* prod = dynamic_cast<RooProdPdf*>( nuispdf.get() );
  if ( prod == 0 ) throw std::runtime_error( "asimovUtil: the nuisance pdf is not a RooProdPdf!" );

  std::unique_ptr<TIterator> iter( prod->pdfList().createIterator() );

  for ( RooAbsArg* a = ( RooAbsArg* ) iter->Next(); a != 0; a = ( RooAbsArg* ) iter->Next() ){
    RooAbsPdf* cterm = dynamic_cast<RooAbsPdf*>( a );

    if ( !cterm ) throw std::logic_error( "asimovUtil: a factor of the nuisance pdf is not a Pdf!" );

    if ( !cterm->dependsOn( nuis ) ) continue; // dummy constraints

    if ( typeid( *cterm ) == typeid( RooUniform ) ) continue;
    if ( typeid( *cterm ) == typeid( RooGenericPdf ) ) continue;

    std::unique_ptr<RooArgSet> cpars( cterm->getParameters( &gobs ) );
    std::unique_ptr<RooArgSet> cgobs( cterm->getObservables( &gobs ) );

    if ( cgobs->getSize() != 1 )
      throw std::runtime_error( Form( "asimovUtil: constraint term %s has multiple global observables", cterm->GetName() ) );

    RooRealVar& rrv = dynamic_cast<RooRealVar&>( *cgobs->first() );
    RooAbsReal* match = 0;

    if ( cpars->getSize() == 1 ) match = dynamic_cast<RooAbsReal*>( cpars->first() );
    else{
      std::unique_ptr<TIterator> iter2( cpars->createIterator() );
      
      for ( RooAbsArg* a2 = ( RooAbsArg* ) iter2->Next(); a2 != 0; a2 = ( RooAbsArg* ) iter2->Next() ){
	RooRealVar* rrv2 = dynamic_cast<RooRealVar*>( a2 );

	if ( rrv2 != 0 && !rrv2->isConstant() ){
	  if ( match != 0 ) throw std::runtime_error( Form( "asimovUtil: constraint term %s has multiple floating params", cterm->GetName() ) );
	  match = rrv2;
	}
      }
    }

    if ( match == 0 ){
      std::cerr << "ERROR: asimovUtil: can't find nuisance for constraint term " << cterm->GetName() << std::endl;
      std::cerr << "Parameters: " << std::endl;
      cpars->Print( "V" );
      std::cerr << "Observables: " << std::endl;
      cgobs->Print( "V" );
      throw std::runtime_error( Form( "asimovUtil: can't find nuisance for constraint term %s", cterm->GetName() ) );
    }

    std::string pdfName( cterm->ClassName() );

    if ( pdfName == "RooGaussian"
	 || pdfName == "RooBifurGauss"
	 || pdfName == "RooPoisson" ){
      // this is easy
      if ( pdfName == "RooGaussian" || pdfName == "RooBifurGauss" ) rrv.setVal( match->getVal() );
      else rrv.setVal( match->getVal()*rrv.getVal() );
    }
    else if ( pdfName == "RooGamma" ){
      // notation as in http://en.wikipedia.org/wiki/Gamma_distribution
      //     nuisance = x
      //     global obs = kappa ( = observed sideband events + 1)
      //     scaling    = theta ( = extrapolation from sideband to signal)
      // we want to set the global obs to a value for which the current value
      // of the nuisance is the best fit one.
      // best fit x = (k-1)*theta    ---->  k = x/theta + 1

      RooArgList leaves;
      cterm->leafNodeServerList( &leaves );
      std::unique_ptr<TIterator> iter2( leaves.createIterator() );
      RooAbsReal* match2 = 0;

      for ( RooAbsArg* a2 = ( RooAbsArg* ) iter2->Next(); a2 != 0; a2 = ( RooAbsArg* ) iter2->Next() ){
	RooAbsReal* rar = dynamic_cast<RooAbsReal*>( a2 );

	if ( rar == 0 || rar == match || rar == &rrv ) continue;

	if ( !rar->isConstant() ) throw std::runtime_error( Form( "asimovUtil: extra floating parameter %s of RooGamma %s.", rar->GetName(), cterm->GetName() ) );

	if ( rar->getVal() == 0 ) continue; // this could be mu

	if ( match2 != 0 ) throw std::runtime_error( Form( "asimovUtil: extra constant non-zero parameter %s of RooGamma %s.", rar->GetName(), cterm->GetName() ) );

	match2 = rar;
      }

      if ( match2 == 0 ) throw std::runtime_error( Form( "asimovUtil: could not find the scaling term for  RooGamma %s.", cterm->GetName() ) );

      rrv.setVal( match->getVal() / match2->getVal() + 1. );
    }
    else throw std::runtime_error( Form( "asimovUtil: can't handle constraint term %s of type %s", cterm->GetName(), cterm->ClassName() ) );
  }
}
