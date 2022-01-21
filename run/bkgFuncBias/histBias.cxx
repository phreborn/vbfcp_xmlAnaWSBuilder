void histBias(){
  TCanvas *canv = new TCanvas("c", "canvas", 800, 600);
  TH1F *h1 = new TH1F("h1","best-fit #tilde{d} distr",100,-0.15,0.15);
  TTree *T = new TTree("ntuple","data from ascii file");
  Long64_t nlines = T->ReadFile("toy_bestfits.txt","hat_d");
  printf(" found %lld points\n",nlines);
  T->Draw("hat_d >> h1");
  h1->GetXaxis()->SetTitle("best-fit #tilde{d}");
  h1->GetYaxis()->SetTitle("nEvents/bin");
  h1->Draw("e");
  canv->SaveAs("distr_tildeD.png");
}
