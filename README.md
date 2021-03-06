# XML Analytic Workspace Builder
### Developed based on workspace building code by Haichen Wang

## Installation

### Clone xmlAnaWSBuilder and set up environment

To install the software, run
```
git clone ssh://git@gitlab.cern.ch:7999/atlas-hgam-sw/xmlAnaWSBuilder.git
cd xmlAnaWSBuilder
```

Then please set up ROOT and cmake. The recommended version is 6.18 or above. If you are on lxplus, you can directly run
```
source setup_lxplus.sh
```

### Install RooFitExtensions

Common custom classes like RooTwoSidedCBShape are now centrally maintained in RooFitExtensions. To avoid possible conflicts this class has been removed from the XML workspace builder, and the installation now requires [RooFitExtensions](https://gitlab.cern.ch/atlas_higgs_combination/software/RooFitExtensions).

If you have not installed RooFitExtensions, simply run
```
sh scripts/install_roofitext.sh
```
Otherwise, please make sure the installation path can be found. It can be ensured by declaring the following environment variable that points to the RooFitExtensions installation path and/or the path contains RooFitExtensionsConfig.cmake
```
export RooFitExtensions_DIR=<your path>
```

### Compile xmlAnaWSBuilder

The XML analytic workspace builder has now moved to cmake. To compile, run
```
mkdir build && cd build
cmake ..
make -j4
make install
cd ..
```

By default the installation step will put the executable and library under ```xmlAnaWSBuilder``` folder. If you would like to install them in a different location, please specify it at the cmake step.

## Using the software
After the installation is finished, on lxplus you only need to set up the environment by
```
source setup_lxplus.sh
```
every time before using the software. 

For more details on how to use the software, please checkout https://twiki.cern.ch/twiki/bin/view/AtlasProtected/XmlAnaWSBuilder

## A note on setup script
The `setup_lxplus.sh` provided in the repository and discussed above is **an example**. It will be maintained up-to-date on a best effort basis, but users are recommended to prepare their own setup script following this example (when using an environment different than `lxplus` this is actually mandatory). 

A different ROOT version can be used as long as it is above 6.18 (please report an issue in case a newer version leads to problems). Cmake version can also be adjusted.
Essentially you only need to modify the following two lines in the script
```
source /cvmfs/sft.cern.ch/lcg/releases/LCG_96b/CMake/3.14.3/x86_64-centos7-gcc8-opt/CMake-env.sh
source /cvmfs/sft.cern.ch/lcg/releases/LCG_96b/ROOT/6.18.04/x86_64-centos7-gcc8-opt/ROOT-env.sh
```
If ROOT and cmake are already automatically set up, these lines can be simply removed.

The rest of the script is taking care of the following
* Attach `lib` folder to ${LD_LIBRARY_PATH} (IMPORTANT if you are using any custom class not in vanila ROOT -- otherwise when you open the workspace it will crash)
* Attach `bin` folder to ${PATH} (allow you to use XMLReader command everywhere)

