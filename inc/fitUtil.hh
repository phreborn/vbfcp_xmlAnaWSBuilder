#ifndef FITUTIL_HEADER
#define FITUTIL_HEADER

#include "CommonHead.h"
#include "RooFitHead.h"
#include "RooStatsHead.h"

#include "auxUtil.hh"

using namespace std;
using namespace RooFit;
using namespace RooStats;

class fitUtil : public TObject{
public:
  // Minimizer setup
  static int _minimizerStrategy;
  static string _minimizerAlgo;
  static double _minimizerTolerance;
  static bool _nllOffset;
  static int _printLevel;

  static int profileToData(ModelConfig *mc, RooAbsData *data);
  ClassDef(fitUtil,1);
};

#endif
