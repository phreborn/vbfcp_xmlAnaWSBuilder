#include "CommonHead.h"

#include "xmlAnaWSBuilder.hh"
#include "auxUtil.hh"
#include "fitUtil.hh"
#include "asimovUtil.hh"

using namespace std;

struct option longopts[] = {
  { "xml", required_argument, NULL, 'x'},
  { "verbose", required_argument, NULL, 'v'},
  { "minimizerAlgo", required_argument, NULL, 'm'},
  { "minimizerStrategy", required_argument, NULL, 's'},
  { "minimizerTolerance", required_argument, NULL, 't'},
  { "nllOffset", required_argument, NULL, 'n'},
  { "printLevel", required_argument, NULL, 'p'},
  { "binned", required_argument, NULL, 'b'},
  { "changeDataStorageType", required_argument, NULL, 'c'},
  { "plotOption", required_argument, NULL, 'o'},
  { "help", no_argument, NULL, 'h'},
  {0, 0, 0, 0}
};

void printHelp(TString exe){
  cout<<"Usage: "<<exe<<" [options]"<<endl;
  cout<<"Allowed options:"<<endl;
  cout<<" -x [ --xml ] arg                   Input xml file location (required)"<<endl;
  cout<<" -v [ --verbose ] arg               Printing out debug info or not (default no)"<<endl;
  cout<<" -m [ --minimizerAlgo ] arg         Minimizer algorithm (default Minuit2)"<<endl;
  cout<<" -s [ --minimizerStrategy ] arg     Minimizer strategy (default 1)"<<endl;
  cout<<" -t [ --minimizerTolerance ] arg    Minimizer tolerance (default 1e-3)"<<endl;
  cout<<" -n [ --nllOffset ] arg             Enable nllOffset (default on)"<<endl;
  cout<<" -p [ --printLevel ] arg            Fit log print level (default 2)"<<endl;
  cout<<" -b [ --binned ] arg                Fit to binned data (default unbinned)"<<endl;
  cout<<" -c [ --changeDataStorageType] arg  Set RooAbsData StorageType to tree instead of vector (default false)"<<endl;  
  cout<<" -o [ --plotOption ] arg            Plot option (default empty)"<<endl;
  cout<<" -h [ --help ]                      Produce help message"<<endl;
}

int main( int argc , char **argv){
  RooMsgService::instance().getStream(1).removeTopic(RooFit::NumIntegration) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Fitting) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Minimization) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::InputArguments) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Eval) ;

  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);

  string inputFile="";
  bool isVerbose=false;
  bool binned=false;
  bool modifyStorageType=false;
  string plotOption="";

  int oc;
  while ((oc=getopt_long(argc, argv, ":x:v:m:s:t:n:p:b:c:o:h", longopts, NULL)) != -1){
    switch (oc) {
    case 'x':
      inputFile = optarg;
      break;
    case 'v':
      isVerbose = auxUtil::to_bool(optarg);
      cout<<"Set verbose mode to "<<isVerbose<<endl;
      break;
    case 'm':
      fitUtil::_minimizerAlgo = optarg;
      cout<<"Set minimizer algorithm to "<<fitUtil::_minimizerAlgo<<endl;
      break;
    case 's':
      fitUtil::_minimizerStrategy = atoi(optarg);
      cout<<"Set minimizer strategy to "<<fitUtil::_minimizerStrategy<<endl;
      break;
    case 't':
      fitUtil::_minimizerTolerance = atof(optarg);
      cout<<"Set minimizer tolerance to "<<fitUtil::_minimizerTolerance<<endl;
      break;
    case 'n':
      fitUtil::_nllOffset = auxUtil::to_bool(optarg);
      cout<<"Set NLL offset to "<<fitUtil::_nllOffset<<endl;
      break;
    case 'p':
      fitUtil::_printLevel = atoi(optarg);
      cout<<"Set print level to "<<fitUtil::_printLevel<<endl;
      break;
    case 'b':
      binned = auxUtil::to_bool(optarg);
      break;
    case 'c':
      modifyStorageType = auxUtil::to_bool(optarg);
      cout<<"Set RooAbsData defaultStoreType to Tree instead of vector"<<endl;
      break;
    case 'o':
      plotOption = optarg;
      cout<<"Set plot option: "<<plotOption<<endl;
      break;
    case 'h':
      printHelp(argv[0]);
      return 0;
    case ':':   /* missing option argument */
      fprintf(stderr, "%s: option `-%c' requires an argument\n",
	      argv[0], optopt);
      printHelp(argv[0]);
      return 0;
    case '?':
    default:
      fprintf(stderr, "%s: option `-%c' is invalid: ignored\n",
	      argv[0], optopt);
      printHelp(argv[0]);
      return 0;
    }
  }

  if(inputFile == ""){
    cerr<<"Error : no config file given, please specify config file name under -x option"<<endl;
    printHelp(argv[0]);
    return 0;
  }
  
  xmlAnaWSBuilder *wsBuilder=new xmlAnaWSBuilder(inputFile);
  wsBuilder->setDebug(isVerbose);
  wsBuilder->setUseBinned(binned);
  wsBuilder->setDataStorageType(modifyStorageType);
  wsBuilder->setPlotOption(plotOption.c_str());
  wsBuilder->generateWS();
  return 0 ;
}
