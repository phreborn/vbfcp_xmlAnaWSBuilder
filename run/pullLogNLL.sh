#!/bin/bash

for sysSet in allSys jetSys ssSys theoSys photonSys
do

cat=AllCats
pname=p01
cat=AllCats_SMEFT
pname=p0d25
#cat=CPeven20
#cat=Inject
#cat=LT
#cat=StatOnly
#for suffix in statOnly allSys ssSys jetSys photonSys theorySys;do
for suffix in statOnly;do
  outnll=autonll/dNLL_${cat}_${suffix}.log
  outnll=autonll/dNLL_${cat}_pull${sysSet}_${pname}.log
  > ${outnll}
  tmpall=tmpall
  root -b -q getNLL.cxx\(\"out${cat}_pull${sysSet}\"\) | grep nll | cut -d : -f 2 > ${tmpall}
  #cat ${tmpall} | grep m | sort -r | sed 's/m/-0\./g' >> ${outnll}
  #cat ${tmpall} | grep p | sed 's/p/0\./g' >> ${outnll}
  cat ${tmpall} | grep m | sort -r | sed 's/m/-/g' | sed 's/d/\./g' >> ${outnll}
  cat ${tmpall} | grep p | sed 's/p/+/g' | sed 's/d/\./g' >> ${outnll}
  rm ${tmpall}
done

done
