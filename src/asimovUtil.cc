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
      if(action==FIT){
	int status=fitUtil::profileToData(mc, data, _rangeName);
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
      else if(action==MATCHGLOB){
	RooArgSet* constraints=mc->GetPdf()->getAllConstraints(*mc->GetObservables(), *const_cast<RooArgSet*>(mc->GetNuisanceParameters()));
	RooArgSet nuisSyst;
	unique_ptr<TIterator> iter(constraints->createIterator());
	RooAbsPdf *parg=NULL;
	while((parg=dynamic_cast<RooAbsPdf*>(iter->Next()))){
	  RooArgSet *nuisSet=parg->getObservables(*mc->GetNuisanceParameters());
	  RooArgSet *globSet=parg->getObservables(*mc->GetGlobalObservables());
	  if(nuisSet->getSize()==1&&globSet->getSize()==1){
	    RooRealVar *nuis=dynamic_cast<RooRealVar*>(nuisSet->first());
	    RooRealVar *glob=dynamic_cast<RooRealVar*>(globSet->first());
	    auxUtil::setValAndFix(glob, nuis->getVal());
	  }
	}
      }
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
    cout<<endl<<"---------------------------------------"<<endl<<endl;
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
  // _injectionFiles.push_back(auxUtil::getAttributeValue(node, "Injection", true, ""));
}

void asimovUtil::printSummary(){
  cout<<"Following asimov dataset will be generated: ";
  double nAsimov=_asimovNames.size();
  for(int iAsimov=0;iAsimov<nAsimov;iAsimov++){
    if(_asimovProfiles[iAsimov].Contains(GENASIMOV)) cout<<_asimovNames[iAsimov]<<" ";
  }
  cout<<endl;
}
