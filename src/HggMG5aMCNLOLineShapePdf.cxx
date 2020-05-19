#include "HggMG5aMCNLOLineShapePdf.h"

ClassImp(HggMG5aMCNLOLineShapePdf);

//_____________________________________________________________________________
HggMG5aMCNLOLineShapePdf:: HggMG5aMCNLOLineShapePdf() {
}

//_____________________________________________________________________________
HggMG5aMCNLOLineShapePdf::HggMG5aMCNLOLineShapePdf(const char *name, const char *title,
						   RooAbsReal& x, RooAbsReal& mean, RooAbsReal& width, RooAbsReal& alpha, int cme) :
  RooAbsPdf(name, title),
  _x("x", "Dependent", this, x),
  _mean("mean", "Mass", this, mean),
  _width("width", "Width", this, width),
  _alpha("alpha", "Alpha", this, alpha),
  _cme(cme)
{
}


//_____________________________________________________________________________
HggMG5aMCNLOLineShapePdf::HggMG5aMCNLOLineShapePdf(const HggMG5aMCNLOLineShapePdf& other, const char* name) :
  RooAbsPdf(other, name),
  _x("x", this, other._x),
  _mean("mean", this, other._mean),
  _width("width", this, other._width),
  _alpha("alpha", this, other._alpha),
  _cme(other._cme)
{
}

//_____________________________________________________________________________
// Matrix element correction
Double_t HggMG5aMCNLOLineShapePdf::ME(Double_t mgg) const {
  return mgg*mgg*mgg*mgg*mgg*mgg;
}

//_____________________________________________________________________________
// Parton luminosity correction
Double_t HggMG5aMCNLOLineShapePdf::PL(Double_t mgg) const {
  Double_t mgg_norm=0;
  Double_t L=0;
  if(_cme==13){
    // Using formula provided by Yee (last update on July 13, 2016)
    mgg_norm=mgg/13000.;	// Normalize mgg by sqrt(s)

    // Scalar parton luminosity parameterization
    if(_alpha<1e-3) return pow(1-pow(mgg_norm,1./3.), 11.6566)*pow(mgg_norm, -2.55713)*(2.09254e-6);
    // Graviton parton luminosity parameterization
    else{
      double Lgg = pow(1-pow(mgg_norm,0.982575/3),11.2968)*pow(mgg_norm,-2.58083)*(1.89206e-6);
      double Lqq = pow(1-pow(mgg_norm,1./3),8.0919)*pow(mgg_norm,-2.26565)*(8.43489e-8);
      L=Lgg+_alpha*Lqq;
    }
  }
  else if(_cme==8){
    // 8 TeV parameterization by Yee
    mgg_norm=mgg/8000.;	// Normalize mgg by sqrt(s)
    L=pow((1-pow(mgg_norm, 1./3.)), 10.55)*pow(mgg_norm, -2.76)*(2.04e-6 - 1.9e-6*mgg_norm + 1.25e-5*mgg_norm*mgg_norm - 1.52e-5*mgg_norm*mgg_norm*mgg_norm);
  }
  else{
    std::cerr<<"\tERROR: unknown center-of-mass-energy type "<<_cme
	     <<". Choose between 13 (TeV) and 8 (TeV). Aborting..."
	     <<std::endl;
    exit(-1);
  }
  if(!std::isfinite(L)) L=0;
  return L;
}
  
//_____________________________________________________________________________
// Mediocre Breit-Wigner
Double_t HggMG5aMCNLOLineShapePdf::BW(Double_t mgg) const {
  // Breit-Wigner shape for scalar
  if(fabs(_alpha)<1e-3) return 1/((mgg*mgg-_mean*_mean)*(mgg*mgg-_mean*_mean)+(_mean*_width)*(_mean*_width));
  // Breit-Wigner shape for graviton
  else{
    Double_t m2Res=_mean*_mean;
    Double_t sHat=mgg*mgg;
    Double_t GamMRat=_width/_mean;
    return 1 / ((sHat - m2Res)*(sHat - m2Res) + (sHat * GamMRat)*(sHat * GamMRat));
  }
}

//_____________________________________________________________________________
Double_t HggMG5aMCNLOLineShapePdf::evaluate() const {
  return std::max(ME(_x)*PL(_x)*BW(_x)*_x, std::numeric_limits<Double_t>::denorm_min());
}
