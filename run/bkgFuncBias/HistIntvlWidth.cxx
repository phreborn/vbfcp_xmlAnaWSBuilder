#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasUtils.h"
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasStyle.h"
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasLabels.h"

#ifdef __CLING__
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasLabels.C"
#include "/scratchfs/atlas/huirun/atlaswork/ATLAS_style/atlasrootstyle/AtlasUtils.C"
#endif

void HistIntvlWidth(){
  SetAtlasStyle();

  TH1F *h1 = new TH1F("h1","68\% interval width distr",100,-0.1,0.2);
  TH1F *h2 = new TH1F("h2","95\% interval width distr",100,0.0,0.3);
  TTree *T = new TTree("ntuple","data from ascii file");
  Long64_t nlines = T->ReadFile("intvalwidth.txt","1sigma:2sigma");
  printf(" found %lld points\n",nlines);
  T->Draw("1sigma >> h1");
  T->Draw("2sigma >> h2");

  TF1 *gaus1 = new TF1("gaus1", "gaus(0)", -0.5, 0.5);
  h1->Fit("gaus1");
  float mu1 = gaus1->GetParameter(1);
  float sigma1 = gaus1->GetParameter(2);
  cout<<"68\%CL mu, sigma: "<<mu1<<", "<<sigma1<<endl;

  TF1 *gaus2 = new TF1("gaus2", "gaus(0)", -0.5, 0.5);
  h2->Fit("gaus2");
  float mu2 = gaus2->GetParameter(1);
  float sigma2 = gaus2->GetParameter(2);
  cout<<"95\%CL mu, sigma: "<<mu2<<", "<<sigma2<<endl;

  TCanvas *canv1 = new TCanvas("c1", "canvas", 800, 600);

  h1->GetXaxis()->SetTitle("best-fit #tilde{d}");
  h1->GetYaxis()->SetTitle("nEvents/bin");
  h1->Draw("e");

  myText(0.22, 0.88, 1, Form("%lli toys", nlines));
  myText(0.22, 0.83, 1, Form("0.68CL #mu = %0.2f, #sigma = %0.3f", mu1, sigma1));

  canv1->SaveAs("distr_68IntvlWidth.png");

  TCanvas *canv2 = new TCanvas("c2", "canvas", 800, 600);

  h2->GetXaxis()->SetTitle("best-fit #tilde{d}");
  h2->GetYaxis()->SetTitle("nEvents/bin");
  h2->Draw("e");

  myText(0.22, 0.88, 1, Form("%lli toys", nlines));
  myText(0.22, 0.83, 1, Form("0.95CL #mu = %0.2f, #sigma = %0.3f", mu2, sigma2));

  canv2->SaveAs("distr_95IntvlWidth.png");
}
