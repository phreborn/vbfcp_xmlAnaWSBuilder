void sysCompare_OOplot(){
std::vector<TString> syslist = {"JET_JER_EffectiveNP_11", "JET_Flavor_Composition", "TT_b1", "TT_b6"};

TString bdtcat = "TT";

std::map<TString, TH1F*> Hists;
std::map<TString, TH1F*> RHists;
int count = 0;
for(auto sys : syslist){
  count += 1;
  TString inpath = "checkRanking_"+sys+"_m00_"+bdtcat+".root";
  TFile *inf = new TFile(inpath, "read");
  Hists[sys] = (TH1F*)inf->Get(sys+"_m00_"+bdtcat+"");
  
  if(sys.Contains("_b")){
    TH1F* hvarss = (TH1F*)inf->Get("SS_variation_"+sys+"_m00_"+bdtcat+"");
    Hists[sys]->Add(hvarss);
  }
  if(count == 1){
    Hists["SM_VBF"] = (TH1F*)inf->Get("asimov_VBF_"+sys+"_m00_"+bdtcat+"");
    Hists["SM_VBF"]->SetLineColor(kGreen);
  }

  Hists[sys]->SetLineColor(kBlue+7*count);

  TH1F *hsm = (TH1F*)Hists["SM_VBF"]->Clone("tmp_"+sys);
  hsm->Scale(1./hsm->Integral());
  TH1F *rh = (TH1F*)Hists[sys]->Clone("rh_"+sys);
  rh->Scale(1./rh->Integral());
  rh->Divide(hsm);
  RHists[sys] = rh;

  if(count == 1){
    rh = (TH1F*)Hists["SM_VBF"]->Clone("tmp_SM_VBF");
    rh->Scale(1./rh->Integral());
    rh->Divide(hsm);
    RHists["SM_VBF"] = rh;
  }
}

TCanvas *canv = new TCanvas("c", "c", 800, 600);
TLegend *lg = new TLegend(0.45, 0.65, 0.75, 0.9);

TPad pad1("pad1", "pad1", 0, 0.3, 1, 1.0);
pad1.SetBottomMargin(0);
pad1.Draw();
pad1.cd();

count = 0;
for(auto h : Hists){
  count += 1;
  TString hname = h.first;
  h.second->Scale(1./h.second->Integral());
  //h.second->SetLineColor(kBlue+7*count);
  lg->AddEntry(h.second, hname, "l");
  if(count == 1) {
    h.second->SetMinimum(0);
    h.second->SetMaximum((h.second->GetMaximum())*1.6);
    h.second->GetYaxis()->SetTitle("Normalized to 1");
    h.second->Draw("hist");
  }
  else h.second->Draw("same hist");
  cout<<hname<<" "<<h.second->GetMean() - 3<<endl;
}
lg->Draw("same");

canv->cd();
TPad pad2("pad2", "pad2", 0, 0.05, 1, 0.3);
pad2.SetTopMargin(0);
pad2.SetBottomMargin(0.2);
pad2.Draw();
pad2.cd();

//TH1F *hSM = Hists["SM_VBF"].Clone("tmp_SM_VBF");

count = 0;
for(auto h : RHists){
  count += 1;
  if(count == 1){
    h.second->SetMaximum(1.11);
    h.second->SetMinimum(0.95);
    h.second->GetXaxis()->SetTitle("OO");
    h.second->GetXaxis()->SetTitleSize(0.1);
    h.second->GetXaxis()->SetTitleOffset(0.8);
    h.second->Draw("hist");
  }else{
    h.second->Draw("same hist");
  }
}

canv->SaveAs("sysCompare_OOplot_cate"+bdtcat+".png");
canv->SaveAs("sysCompare_OOplot_cate"+bdtcat+".pdf");
}
