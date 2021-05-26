void getNLL(TString fitresult = "outAllCats_allSys"){
  map<TString, double> d_map;
  d_map["m00"] = 0.;
  d_map["m01"] = -0.01;
  d_map["m02"] = -0.02;
  d_map["m03"] = -0.03;
  d_map["m04"] = -0.04;
  d_map["m05"] = -0.05;
  d_map["m06"] = -0.06;
  d_map["m07"] = -0.07;
  d_map["m08"] = -0.08;
  d_map["m10"] = -0.10;
  d_map["m12"] = -0.12;
  d_map["m14"] = -0.14;
  d_map["m16"] = -0.16;
  d_map["m18"] = -0.18;
  d_map["m20"] = -0.20;
  d_map["p01"] = 0.01;
  d_map["p02"] = 0.02;
  d_map["p03"] = 0.03;
  d_map["p04"] = 0.04;
  d_map["p05"] = 0.05;
  d_map["p06"] = 0.06;
  d_map["p07"] = 0.07;
  d_map["p08"] = 0.08;
  d_map["p10"] = 0.10;
  d_map["p12"] = 0.12;
  d_map["p14"] = 0.14;
  d_map["p16"] = 0.16;
  d_map["p18"] = 0.18;
  d_map["p20"] = 0.20;

  TFile *f_SM = new TFile(fitresult+"/out_m00.root", "read");
  TTree *t_SM = (TTree*)f_SM->Get("nllscan");

  double nll_SM;
  t_SM->SetBranchAddress("nll", &nll_SM);
  t_SM->GetEntry(0);

  for(auto d = d_map.begin(); d != d_map.end(); d++){
    TFile *f = new TFile(fitresult+"/out_"+d->first+".root", "read");
    TTree *t = (TTree*)f->Get("nllscan");

    double nll;
    t->SetBranchAddress("nll", &nll);
    t->GetEntry(0);

    cout<<d->first<<" "<<nll - nll_SM<<endl;
    delete f;
  }

  delete f_SM;
}
