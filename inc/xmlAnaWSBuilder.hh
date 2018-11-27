#ifndef XMLANAWSBUILDER_HEADER
#define XMLANAWSBUILDER_HEADER

#include "CommonHead.h"
#include "RooFitHead.h"
#include "RooStatsHead.h"

#include "auxUtil.hh"
#include "asimovUtil.hh"

#include "FlexibleInterpVarMkII.hh"
// Additional custom classes
#include "RooTwoSidedCBShape.hh"
#include "HggMG5aMCNLOLineShapePdf.hh"

using namespace std;
using namespace RooFit;
using namespace RooStats;

struct Sample{
  TString procName;	        // Name of the process 
  TString inputFile;		// Input XML file for constructing pdf (if needed)
  vector<TString> normFactors;	// Collection of scale factors for normalization
  vector<TString> shapeFactors;	// Collection of scale factors for shape
  vector<TString> systGroups;	// List of systematic groups to be imported
  RooArgSet expected;		// Collection of response terms and scale factors to be multiplied to the yield
  TString modelName;		// Name of pdf model for the process
  TString normName;		// Name of normalization function for the process
  TString sharePdfGroup;	// Whether this process share a PDF with other processes

  friend bool operator==(const Sample& x, const Sample& y){return x.procName==y.procName;} // Process name is the only identity for a sample. Duplicating it is not allowed.
  friend bool operator==(const Sample& x, const TString& y){return x.procName==y;}
};

struct Systematic{
  TString NPName;
  TString domain;
  TString process;
  TString whereTo;
  TString constrTerm;
  
  double nominal;
  double beta;

  TString errLoExpr;
  TString errHiExpr;

  friend bool operator==(const Systematic& x, const Systematic& y){return (x.NPName==y.NPName&&x.process==y.process&&x.whereTo==y.whereTo);} // NP name, NP group (process) and NP destination identify a systematic. Duplicating it is not allowed.
};

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
  TString _categoryName;
  TString _categoryType;
  double _xMin;
  double _xMax;
  bool _goBlind;
  double _blindMin;
  double _blindMax;
  int _Nch;
  double _luminosity;
  TString _inputDataFileName;
  TString _inputDataFileType;
  TString _inputDataTreeName;
  TString _inputDataVarName;
  TString _Cut;
  TString _inputDataHistName;
  bool _injectGhost;
  int _numData;
  
  // Vectors and maps which will be cleared after generating model for each channel
  map<TString, vector<Systematic> > _Systematics;
  vector<Sample> _Samples;
  vector<TString> _ItemsLowPriority;  
  vector<TString> _ItemsHighPriority;  
  vector<TString> _ItemsCorrelate;
  
  // Vectors and variables which will NOT be cleared after generating model for each channel
  vector<TString> _xmlPath;
  vector<TString> _CN;
  vector<TString> _Type;
  vector<TString> _POIList;
  vector<TString> _rangeList;
  
  // Flags
  bool _useBinned;
  TString _plotOpt;
  bool _debug;			// Flag for printing out more info
  
  // Global names

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // XML workspace builder keywords: below can be used in 
  // building likelihood model. They will be replaced by
  // corresponding objects.
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  static TString RESPONSE;
  static TString OBSERVABLE;
  static TString PROCESS;
  static TString COMMON;
  static TString SELF;
  static TString CATEGORY;
  
  static TString LT;
  static TString LE;
  static TString GT;
  static TString GE;
  static TString AND;
  static TString OR;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // XML workspace builder keywords: below are used to
  // Indicate type of operations one perform
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  static TString ALLPROC;
  static TString YIELD;
  static TString SHAPE;
  static TString COUNTING;
  static TString USERDEF;
  static TString EXTERNAL;
  static TString HISTOGRAM;
  static TString ASCII;

  static TString GAUSSIAN;
  static TString LOGNORMAL;
  static TString ASYMMETRIC;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Hard-coded object naming partten: do not duplicate
  // these partten when constructing your own model!
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

  static TString LUMINAME;
  static TString XSNAME;
  static TString BRNAME;
  static TString NORMNAME;
  static TString ACCEPTANCENAME;
  static TString EFFICIENCYNAME;
  static TString CORRECTIONNAME;
  
  static TString OBSDSNAME;

  static TString SBLO;
  static TString BLIND;
  static TString SBHI;
  
  TStopwatch _timer;

  void NPMaker(RooWorkspace *w, Systematic *syst, RooArgSet *nuispara, RooArgSet *constraints , RooArgSet *globobs, RooArgSet *expected);
  void generateSingleChannel(TString xmlName, RooWorkspace *wchannel);
  void readSyst(TXMLNode* systNode, TString process="allproc");
  void readSample(TXMLNode* sampleNode);
  void readSampleXMLNode(TXMLNode* node, Sample& sample);
  void getModel(RooWorkspace *w, Sample *sample, RooArgSet *nuispara=NULL, RooArgSet *constraints=NULL, RooArgSet *globobs=NULL);
  void checkNuisParam(RooAbsPdf *model, RooArgSet *nuispara);
  void clearUp(){_Systematics.clear();_ItemsLowPriority.clear(); _ItemsHighPriority.clear(); _ItemsCorrelate.clear(); _Samples.clear();}
  void attachConstraints(RooWorkspace *w, TString sumPdfStr, RooArgSet *constraints, TString finalModelName);
  TString getItemExpr(TXMLNode *node, TString attrName, TString process="");
  RooDataSet* readInData(RooRealVar *x, RooRealVar *w);
  TString implementObj(RooWorkspace *w, TString expr, bool checkExistBeforeImp=false);
  TString implementObjArray(RooWorkspace *w, vector<TString> objArr);
  TString implementUncertExpr(RooWorkspace *w, TString expr, TString varName, int uncertType);
  void readChannelXMLNode(TXMLNode *node);
public:
  xmlAnaWSBuilder(TString inputFile);
  void generateWS();
  void setDebug(bool flag){_debug=flag;};
  void setUseBinned(bool flag){_useBinned=flag;};
  void setPlotOption(TString option){_plotOpt=option;};
  void dataFileSanityCheck();
  void translateKeyword(TString &expr);

  void Summary(TString outputFigName);
  ClassDef(xmlAnaWSBuilder,3);
};

#endif
