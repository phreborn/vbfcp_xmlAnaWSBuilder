#include "CommonHead.h"

#include "xmlAnaWSBuilder.hh"
#include "auxUtil.hh"
#include "fitUtil.hh"
#include "asimovUtil.hh"

#include <boost/program_options.hpp>

using namespace std;
using namespace boost;

// Default values for the minimizer

int main( int argc , char **argv){
  namespace po = boost::program_options;
  // RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
  
  string inputFile="";
  bool isVerbose=false;
  bool binned=false;
  string plotOption="";
  
  po::options_description desc("Allowed options");
  desc.add_options()
    ("xml,x", po::value<string>(&inputFile), "Input xml file location (required)")
    ("verbose,v", po::value<bool>(&isVerbose), "Printing out debug info or not")
    ("minimizerAlgo,m", po::value<string>(&fitUtil::_minimizerAlgo), "Minimizer algorithm (default Minuit2)")
    ("minimizerStrategy,s", po::value<int>(&fitUtil::_minimizerStrategy), "Minimizer strategy (default 1)")
    ("minimizerTolerance,t", po::value<double>(&fitUtil::_minimizerTolerance), "Minimizer tolerance (default 1e-3)")
    ("nllOffset", po::value<bool>(&fitUtil::_nllOffset), "Enable nllOffset (default on)")
    ("printLevel,p", po::value<int>(&fitUtil::_printLevel), "Fit log print level (default 0)")
    ("binned,b", po::value<bool>(&binned), "Fit to binned data (default unbinned)")
    ("plot", po::value<string>(&plotOption), "Plot option")

    ("help,h", "Produce help message")
    ;
  
  po::variables_map vm0;
  // parse the first time, using only common options and allow unregistered options
  try{
    po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm0);
    po::notify(vm0);
  } catch(std::exception &ex) {
    cerr << "Invalid options: " << ex.what() << endl;
    cout << "Use combine --help to get a list of all the allowed options"  << endl;
    return -1;
  } catch(...) {
    cerr << "Unidentified error parsing options." << endl;
    return -1;
  }

  if(vm0.count("help")||argc==1||!vm0.count("xml")) {
    cout << "Usage: "<<argv[0]<<" [options]\n";
    cout << desc <<endl;
    return 0;
  }

  RooMsgService::instance().getStream(1).removeTopic(RooFit::NumIntegration) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Fitting) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Minimization) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::InputArguments) ;
  RooMsgService::instance().getStream(1).removeTopic(RooFit::Eval) ;

  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);

  xmlAnaWSBuilder *wsBuilder=new xmlAnaWSBuilder(inputFile);
  wsBuilder->setDebug(isVerbose);
  wsBuilder->setUseBinned(binned);
  wsBuilder->setPlotOption(plotOption.c_str());
  wsBuilder->generateWS();
  return 0 ;
}
