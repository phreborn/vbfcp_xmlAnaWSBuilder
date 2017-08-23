#ifndef ASIMOVUTIL_HEADER
#define ASIMOVUTIL_HEADER

#include "CommonHead.h"
#include "RooFitHead.h"
#include "RooStatsHead.h"

#include "auxUtil.hh"
#include "fitUtil.hh"

using namespace std;
using namespace RooFit;
using namespace RooStats;

class asimovUtil : public TObject{
private:
  vector<TString> _asimovNames, _asimovSetups, _asimovProfiles;
  vector<TString> _SnapshotsAll, _SnapshotsNuis, _SnapshotsGlob, _SnapshotsPOI, _Snapshots;
  vector<TString> _injectionFiles;
  // action items
  static TString RAW;
  static TString FIT;
  static TString RESET;
  static TString GENASIMOV;
  static TString FLOAT;
  static TString FIXSYST;
  static TString MATCHGLOB;
  static TString SAVESNAPSHOT;
public:
  asimovUtil(){}
  void addEntry(TXMLNode *node);
  void generateAsimov(ModelConfig *mc, TString dataName);
  void printSummary();
  bool genAsimov(){return _asimovNames.size()>0;}
  ClassDef(asimovUtil,2);
};

#endif
