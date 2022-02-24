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
  float chisqur1 = gaus1->GetChisquare();
  float ndof1 = gaus1->GetNDF();
  cout<<"68\%CL mu, sigma, chi2/ndof: "<<mu1<<", "<<sigma1<<", "<<chisqur1/ndof1<<endl;

  TF1 *gaus2 = new TF1("gaus2", "gaus(0)", -0.5, 0.5);
  h2->Fit("gaus2");
  float mu2 = gaus2->GetParameter(1);
  float sigma2 = gaus2->GetParameter(2);
  float chisqur2 = gaus2->GetChisquare();
  float ndof2 = gaus2->GetNDF();
  cout<<"95\%CL mu, sigma, chi2/ndof: "<<mu2<<", "<<sigma2<<", "<<chisqur2/ndof2<<endl;

  TCanvas *canv1 = new TCanvas("c1", "canvas", 800, 600);

  h1->GetXaxis()->SetTitle("68\%CL interval width");
  h1->GetYaxis()->SetTitle("nEvents/bin");
  h1->Draw("e");

  myText(0.22, 0.88, 1, Form("%lli toys", nlines));
  myText(0.22, 0.83, 1, Form("#mu = %0.2f, #sigma = %0.3f", mu1, sigma1));
  myText(0.22, 0.78, 1, Form("#chi^{2}/ndof = %0.2f/%0.f", chisqur1, ndof1));

  canv1->SaveAs("distr_68IntvlWidth.png");

  TCanvas *canv2 = new TCanvas("c2", "canvas", 800, 600);

  h2->GetXaxis()->SetTitle("95\%CL interval width");
  h2->GetYaxis()->SetTitle("nEvents/bin");
  h2->Draw("e");

  myText(0.22, 0.88, 1, Form("%lli toys", nlines));
  myText(0.22, 0.83, 1, Form("#mu = %0.2f, #sigma = %0.3f", mu2, sigma2));
  myText(0.22, 0.78, 1, Form("#chi^{2}/ndof = %0.2f/%0.f", chisqur2, ndof2));

  canv2->SaveAs("distr_95IntvlWidth.png");
}
