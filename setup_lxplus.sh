#!/bin/bash

# Setup software
source /cvmfs/sft.cern.ch/lcg/releases/LCG_96b/CMake/3.14.3/x86_64-centos7-gcc8-opt/CMake-env.sh
source /cvmfs/sft.cern.ch/lcg/releases/LCG_97_ATLAS_1/ROOT/v6.20.02/x86_64-centos7-gcc8-opt/ROOT-env.sh

# More memory
ulimit -S -s unlimited

# Greet the user
if [ $_DIRXMLWSBUILDER ]; then
    echo _DIRXMLWSBUILDER is already defined, use a clean shell
    return 1
fi

# speficy the SFRAME base directory, i.e. the directory in which this file lives
export _DIRXMLWSBUILDER=${PWD}

# Modify to describe your directory structure. Default is to use the a structure where
# all directories are below the SFrame base directory specified above
export _BIN_PATH=${_DIRXMLWSBUILDER}/bin
export _LIB_PATH=${_DIRXMLWSBUILDER}/lib

# The Makefiles depend only on the root-config script to use ROOT,
# so make sure that is available
if [[ `which root-config` == "" ]]; then
    echo "Error: ROOT environment doesn't seem to be configured!"
fi

if [[ `root-config --platform` == "macosx" ]]; then
    export DYLD_LIBRARY_PATH=${_LIB_PATH}:${DYLD_LIBRARY_PATH}
else
    export LD_LIBRARY_PATH=${_LIB_PATH}:${LD_LIBRARY_PATH}
fi

export PATH=${_BIN_PATH}:${PATH}
