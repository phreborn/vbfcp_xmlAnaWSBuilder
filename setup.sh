#!/bin/bash

# To be used on lxplus
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib:

source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/gcc493_x86_64_slc6/slc6/gcc49/setup.sh
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/6.04.16-HiggsComb-x86_64-slc6-gcc49-opt/bin/thisroot.sh

# Check if bin/lib/include directories exist, if not create them
export _BIN_PATH=${PWD}/exe
export PATH=${_BIN_PATH}:${PATH}
