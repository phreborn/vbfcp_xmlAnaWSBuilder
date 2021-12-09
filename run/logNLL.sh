#!/bin/bash

cat=AllCats_SMEFT
#cat=LT_SMEFT
for suffix in statOnly allSys ssSys jetSys photonSys theorySys;do
#for suffix in statOnly allSys;do
  outnll=autonll/dNLL_${cat}_${suffix}.log
  > ${outnll}
  tmpall=tmpall
  root -b -q getNLL.cxx\(\"out${cat}_${suffix}\"\) | grep nll | cut -d : -f 2 | sed 's/d/\./g' > ${tmpall}
  cat ${tmpall} | grep m | sort -r | sed 's/m/-/g' >> ${outnll}
  cat ${tmpall} | grep p | sed 's/p//g' >> ${outnll}
  rm ${tmpall}
done
