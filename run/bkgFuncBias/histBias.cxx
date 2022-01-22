#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasUtils.h"
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasStyle.h"
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasLabels.h"

#ifdef __CLING__
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasLabels.C"
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasUtils.C"
#endif

void histBias(){
  SetAtlasStyle();

  TCanvas *canv = new TCanvas("c", "canvas", 800, 600);
  TH1F *h1 = new TH1F("h1","best-fit #tilde{d} distr",100,-0.15,0.15);
  TTree *T = new TTree("ntuple","data from ascii file");
  Long64_t nlines = T->ReadFile("toy_bestfits.txt","hat_d");
  printf(" found %lld points\n",nlines);
  T->Draw("hat_d >> h1");

  TF1 *gaus = new TF1("gaus", "gaus(0)", -0.2, 0.2);
  h1->Fit("gaus");
  float mu = gaus->GetParameter(1);
  float sigma = gaus->GetParameter(2);
  cout<<"mu, sigma: "<<mu<<", "<<sigma<<endl;

  h1->GetXaxis()->SetTitle("best-fit #tilde{d}");
  h1->GetYaxis()->SetTitle("nEvents/bin");
  h1->Draw("e");

  myText(0.22, 0.88, 1, Form("%lli toys", nlines));
  myText(0.22, 0.83, 1, Form("#mu = %0.4f, #sigma = %0.3f", mu, sigma));

  canv->SaveAs("distr_tildeD.png");
}
