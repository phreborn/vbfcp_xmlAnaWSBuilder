#ifndef XMLANAWSBUILDER_HEADER
#define XMLANAWSBUILDER_HEADER

#include "CommonHead.h"
#include "RooFitHead.h"
#include "RooStatsHead.h"

#include "auxUtil.hh"
#include "asimovUtil.hh"

#include "FlexibleInterpVarMkII.hh"
// Additional custom classes
#include "HggTwoSidedCBPdf.hh"
#include "HggMG5aMCNLOLineShapePdf.hh"

using namespace std;
using namespace RooFit;
using namespace RooStats;

struct Sample{
  TString procName;
  vector<TString> procType;		// systematics to be imported
  double yield;
  TString inputFile;		// For parametrization input if needed
  TString normFactors;	// Collection of scale factors for normalization
  TString correlatedNFs;	// Collection of scale factors to not be renamed
  TString shapeFactors;	// Collection of scale factors for shape
  RooArgSet expected;		// Collection of response terms and scale factors to be timed to the signal yield
  TString modelName;		// Name of pdf model for the process
  TString normName;		// Name of the final normalization
  TString sharePdfGroup;
};

struct Systematic{
  TString NPName;
  TString process;
  TString whereTo;
  TString constrTerm;
  
  double nominal;
  double beta;

  TString errLoExpr;
  TString errHiExpr;
};

typedef std::map<TString, Sample>::iterator it_sample;
typedef std::map<TString, vector<Systematic> >::iterator it_syst;

class xmlAnaWSBuilder : public TObject{
private:
// Workspace and ModelConfig
// Use auto_ptr for now as rootcint used by ROOT 5 cannot interpret more modern unique_ptr
  auto_ptr<RooWorkspace> _combWS;
  auto_ptr<ModelConfig> _mConfig;
  auto_ptr<asimovUtil> _asimovHandler;

  // Important object names
  TString _wsName;
  TString _mcName;
  TString _dataName;
  TString _outputFileName;

  // Global variables which will be rewritten for each channel
  TString _observableName;
  double _xMin;
  double _xMax;
  int _Nch;
  double _luminosity;
  TString _inputDataFileName;
  TString _inputDataFileType;
  TString _inputDataTreeName;
  TString _inputDataVarName;
  bool _injectGhost;
  
  // Vectors and maps which will be cleared after generating model for each channel
  map<TString, vector<Systematic> > _Systematics;
  map<TString, Sample> _Samples;
  vector<TString> _ItemsLowPriority;  
  vector<TString> _ItemsHighPriority;  

  // Vectors and variables which will NOT be cleared after generating model for each channel
  vector<TString> _xmlPath;
  vector<TString> _CN;
  vector<TString> _Type;
  vector<TString> _POIList;
  vector<TString> _ItemsCorrelate;

  // Flags
  bool _useBinned;
  TString _plotOpt;
  bool _debug;			// Flag for printing out more info
  
  // Global names
  static TString ALLPROC;
  static TString YIELD;
  static TString SHAPE;
  static TString COUNTING;
  static TString USERDEF;
  static TString EXTERNAL;
  static TString HISTOGRAM;
  static TString ASCII;
  
  static TString RESPONSE;
  static TString OBSERVABLE;
  static TString PROCESS;
  static TString COMMON;
  static TString SELF;

  static TString GAUSSIAN;
  static TString LOGNORMAL;
  static TString ASYMMETRIC;
  
  static TString RESPONSEPREFIX;
  static TString CONSTRTERMPREFIX;
  static TString GLOBALOBSPREFIX;
  static TString VARIATIONHIPREFIX;
  static TString VARIATIONLOPREFIX;
  static TString YIELDPREFIX;
  static TString PDFPREFIX;
  static TString EXPECTATIONPREFIX;
  
  static TString SUMPDFNAME;
  static TString FINALPDFNAME;

  static TString LT;
  static TString LE;
  static TString GT;
  static TString GE;
  
  TStopwatch _timer;

  int CN2IDX(TString channelname);
  void NPMaker(RooWorkspace *w, Systematic *syst, RooArgSet *nuispara, RooArgSet *constraints , RooArgSet *globobs, RooArgSet *expected);
  void generateSingleChannel(TString xmlName, RooWorkspace *wchannel);
  void readSyst(TXMLNode* systNode, TString process="allproc");
  void readSample(TXMLNode* sampleNode);
  void readSampleChildren(TXMLNode* subNode, Sample& sample);
  void getModel(RooWorkspace *w, Sample *sample, TString channeltype="shape", RooArgSet *nuispara=NULL, RooArgSet *constraints=NULL, RooArgSet *globobs=NULL);
  void checkNuisParam(RooAbsPdf *model, RooArgSet *nuispara);
  void clearUp(){_Systematics.clear();_ItemsLowPriority.clear(); _ItemsHighPriority.clear(); _Samples.clear();}
  void attachConstraints(RooWorkspace *w, TString sumPdfStr, RooArgSet *constraints, TString finalModelName);
  TString getItemExpr(TXMLNode *node, TString attrName, TString process="");
  RooDataSet* readInData(RooRealVar *x, RooRealVar *w);
  TString implementObj(RooWorkspace *w, TString expr, bool checkExistBeforeImp=false);
  void implementObjArray(RooWorkspace *w, vector<TString> objArr);
  TString implementUncertExpr(RooWorkspace *w, TString expr, TString varName, int uncertType);
  
public:
  xmlAnaWSBuilder(TString inputFile);
  void generateWS();
  void setDebug(bool flag){_debug=flag;};
  void setUseBinned(bool flag){_useBinned=flag;};
  void setPlotOption(TString option){_plotOpt=option;};
  ClassDef(xmlAnaWSBuilder,3);
};

#endif
