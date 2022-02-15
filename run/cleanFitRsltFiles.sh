#!/bin/bash

for pname in p01 p0d25
do
for sysSet in allSys jetSys ssSys theoSys photonSys
do
  fitRslt=${pname}_${sysSet}Fit.txt
  sed -i 's/ +\/-.*$//g' ${fitRslt}
  sed -i 's/^.*ATLAS/ATLAS/g' ${fitRslt}
  sed -i 's/   / /g' ${fitRslt}
  sed -i 's/  / /g' ${fitRslt}
done
done
