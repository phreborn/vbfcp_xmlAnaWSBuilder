#include "../../../../syst/yield_sys/ioUtils.h"

bool getAttribs(TString fpath, std::map<TString, std::vector<float>> &sysAttribs){
  ifstream file;
  file.open(fpath.Data());
  if( ! file.is_open())
  {
      cout<<"can not open file! "<<fpath<<endl;
      return false;
  }   
  char tmp[1000];
  while(!file.eof()){
    file.getline(tmp,1000);
    string line(tmp);
    size_t ptmp;
    
    if(line == "") continue;
    
    TString sys = "";
    size_t pos = line.find(" "); std::cout<<line.substr(0,pos)<<std::endl;
    if(pos!=string::npos) sys = line.substr(0,pos).data();
    else continue;

    sysAttribs[sys].clear();
    
    sysAttribs[sys].push_back(atof(getTok(line, 1, " ").data())); cout<<sysAttribs[sys].at(0)<<endl;
    sysAttribs[sys].push_back(atof(getTok(line, 2, " ").data())); cout<<sysAttribs[sys].at(1)<<endl;
    sysAttribs[sys].push_back(atof(getTok(line, 3, " ").data())); cout<<sysAttribs[sys].at(2)<<endl;
    sysAttribs[sys].push_back(atof(getTok(line, 4, " ").data())); cout<<sysAttribs[sys].at(3)<<endl;
    sysAttribs[sys].push_back(atof(getTok(line, 5, " ").data())); cout<<sysAttribs[sys].at(4)<<endl;
  } 
  return true;
}

bool getNPrslts(TString fpath, std::map<TString, std::vector<float>> &sysAttribs){
  ifstream file;
  file.open(fpath.Data());
  if( ! file.is_open())
  {
      cout<<"can not open file! "<<fpath<<endl;
      return false;
  }   
  char tmp[1000];
  while(!file.eof()){
    file.getline(tmp,1000);
    string line(tmp);
    size_t ptmp;
    
    if(line == "") continue;
    
    TString sys = "";
    size_t pos = line.find(" "); std::cout<<line.substr(0,pos)<<std::endl;
    if(pos!=string::npos) sys = line.substr(0,pos).data();
    else continue;

    sysAttribs[sys].clear();
    
    sysAttribs[sys].push_back(atof(getTok(line, 1, " ").data())); cout<<sysAttribs[sys].at(0)<<endl;
    sysAttribs[sys].push_back(atof(getTok(line, 2, " ").data())); cout<<sysAttribs[sys].at(1)<<endl;
    sysAttribs[sys].push_back(atof(getTok(line, 3, " ").data())); cout<<sysAttribs[sys].at(2)<<endl;
  } 
  return true;
}

void createPullInput(){  
  std::map<TString, std::vector<float>> sysAttribs;
  getAttribs("impactDtilde.txt", sysAttribs);
  std::map<TString, std::vector<float>> sysNPrslts;
  getNPrslts("postNPErr.txt", sysNPrslts);
  for (auto sys : sysAttribs){
    TString sysname = sys.first;
    TFile *fout = new TFile("../../../StatisticsTools/root-files/dtilde/pulls/"+sysname+".root", "recreate");
    TTree *t = new TTree("result", "");
    string nuisance;
    double nuis_nom;
    double nuis_hat;
    double nuis_hi;
    double nuis_lo;
    double nuis_prefit;
    double mu_VBF_RW_hat;
    double mu_VBF_RW_up;
    double mu_VBF_RW_down;
    double mu_VBF_RW_up_nom;
    double mu_VBF_RW_down_nom;
    nuisance = sysname.Data();
    nuis_nom = sysNPrslts[sysname].at(0);
    nuis_hi = sysNPrslts[sysname].at(1);
    nuis_lo = sysNPrslts[sysname].at(2);
    nuis_prefit = 1;
    mu_VBF_RW_hat = -4.46624040037e-05;
    mu_VBF_RW_up = sysAttribs[sysname].at(1);
    mu_VBF_RW_down = sysAttribs[sysname].at(2);
    mu_VBF_RW_up_nom = sysAttribs[sysname].at(3);
    mu_VBF_RW_down_nom = sysAttribs[sysname].at(4);
    t->Branch("nuisance", &nuisance);
    t->Branch("nuis_nom", &nuis_nom);
    t->Branch("nuis_hi", &nuis_hi);
    t->Branch("nuis_lo", &nuis_lo);
    t->Branch("nuis_prefit", &nuis_prefit);
    t->Branch("tilde_d_hat", &mu_VBF_RW_hat);
    t->Branch("tilde_d_up", &mu_VBF_RW_up);
    t->Branch("tilde_d_down", &mu_VBF_RW_down);
    t->Branch("tilde_d_up_nom", &mu_VBF_RW_up_nom);
    t->Branch("tilde_d_down_nom", &mu_VBF_RW_down_nom);
    t->Fill();
    t->Write();
    fout->Close();
  }
}
