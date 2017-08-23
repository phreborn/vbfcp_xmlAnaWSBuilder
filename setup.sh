#!/bin/bash

# To be used on lxplus
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/boost/boost-1.55.0-python2.7-x86_64-slc6-gcc48/boost-1.55.0-python2.7-x86_64-slc6-gcc48/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib:/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/boost/boost-1.60.0-python2.7-x86_64-slc6-gcc49/boost-1.60.0-python2.7-x86_64-slc6-gcc49/lib/

#source /afs/cern.ch/atlas/project/HSG7/root/root_v6-04-02/setup.sh
#source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/gcc484_x86_64_slc6/slc6/gcc48/setup.sh 
#source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/5.34.32-HiggsComb-p1-x86_64-slc6-gcc48-opt/bin/thisroot.sh
#source /afs/cern.ch/atlas/project/HSG7/root/root_v6-04-16/setup.sh

source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/Gcc/gcc493_x86_64_slc6/slc6/gcc49/setup.sh
source /cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/root/6.04.16-HiggsComb-x86_64-slc6-gcc49-opt/bin/thisroot.sh