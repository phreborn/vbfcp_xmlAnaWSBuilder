#!/bin/bash

syslist=$(cat postNPErr.txt | cut -d ' ' -f 1 | sort)
echo ${syslist}

cat=AllCats
#cat=noLL
#cat=Inject
#cat=LL
#cat=StatOnly
#for suffix in statOnly allSys ssSys jetSys photonSys theorySys;do
for suffix in allSys;do
for sysname in ${syslist};do
#for sysname in ATLAS_JET_Flavor_Composition;do
  echo ${sysname}
for ud in up dn;do
for popr in post pre;do
  fittype=${ud}_${popr}
  outnll=autonll/dNLL_${cat}_${suffix}_${sysname}_${fittype}.log
  > ${outnll}
  tmpall=tmpall
  root -b -q getNLL.cxx\(\"out${cat}_${suffix}\",\"${sysname}\",\"${fittype}\"\) | grep nll | cut -d : -f 2 > ${tmpall}
  cat ${tmpall} | grep m | sort -r | sed 's/m/-0\./g' >> ${outnll}
  cat ${tmpall} | grep p | sed 's/p/0\./g' >> ${outnll}
  #cat ${tmpall} | grep m | sort -r | sed 's/m/-/g' | sed 's/d/\./g' >> ${outnll}
  #cat ${tmpall} | grep p | sed 's/p/+/g' | sed 's/d/\./g' >> ${outnll}
  rm ${tmpall}
done
done
done
done
