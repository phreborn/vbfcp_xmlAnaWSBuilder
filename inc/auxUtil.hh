#ifndef AUXUTIL_HEADER
#define AUXUTIL_HEADER

#include "CommonHead.h"
#include "RooFitHead.h"
#include "RooStatsHead.h"

using namespace std;
using namespace RooFit;
using namespace RooStats;

class auxUtil : public TObject{
public:
  static TString UNCERTHIPREFIX;
  static TString UNCERTLOPREFIX;
  static TString UNCERTSYMMPREFIX;

  enum _itemType{FUNCTION, VARIABLE, EXIST};
  enum _uncertType{UPERROR, LOERROR, SYMMERROR};

  static TString WARNING;
  static TString ENDC;
  
  static void printTime(){time_t result=time(nullptr);cout<<asctime(localtime(&result));}
  static void printTitle(TString titleText, TString separator="-", int width=10);
  static vector<TString> splitString(const TString& theOpt, const char separator );
  static void removeDuplicatedString(vector<TString>& strArr){sort(strArr.begin(), strArr.end()); strArr.erase(unique(strArr.begin(), strArr.end()), strArr.end());}
  static void removeString(vector<TString>& strArr, TString target){strArr.erase(remove( strArr.begin(), strArr.end(), target), strArr.end());}
  static vector<TString> diffSet(vector<TString> A, vector<TString> B);
  static int parseXMLFile(TDOMParser *xmlparser, TString inputFile);
  static TString getAttributeValue( TXMLNode* rootNode, TString attributeKey, bool allowEmpty=false, TString defaultStr="");

  static TString generateExpr(TString head, RooArgSet *set, bool closeExpr=true);
  static void closeFuncExpr(TString &expr){expr+=")"; removeWhiteSpace(expr); expr.ReplaceAll(",)",")");}
  static TString getObjName(TString inputName);
  
  static void defineSet(RooWorkspace *w, RooArgSet set, TString setName);
  static void defineSet(RooWorkspace *w, vector<TString> set, TString setName);

  static void collectEverything(ModelConfig *mc, RooArgSet *set){set->add(*mc->GetNuisanceParameters());set->add(*mc->GetGlobalObservables());set->add(*mc->GetParametersOfInterest());}

  static void Reset(RooArgSet* original, RooArgSet* snapshot);

  static void releaseTheGhost(RooDataSet *obsdata, RooRealVar *x, RooRealVar *w, double ghostwt=1e-9);

  static int getItemType(TString item);
  static TString translateItemType(int type);
  static TString translateItemType(TString item){return translateItemType(getItemType(item));}
  static TString translateUncertType(int type);
  
  static void alertAndAbort(TString msg);
  static TString combineName(TString name, TString tag){ return name+"_"+tag;}
  static void setValAndFix(RooRealVar *var, double value);
  static int stripSign(TString &expr);
  static void removeWhiteSpace(TString &item);
  static vector<TString> decomposeFuncStr(TString function);
  static bool to_bool(TString str);
  static TXMLNode* findNode(TXMLNode* rootNode, TString nodeName);
  static TXMLAttr* findAttribute(TXMLNode* rootNode, TString attributeKey);
  static RooDataSet* histToDataSet(TH1* h, RooRealVar* x, RooRealVar* w);
    
  static bool checkExist(TString name);
  static TString readNumFromOption(TString opt, TString key);
  static double epsilon;
  ClassDef(auxUtil, 0);
};

#endif
