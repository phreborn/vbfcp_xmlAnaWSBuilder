#!/bin/bash

#%s/ +\/-.*$//g
#%s/^.*ATLAS/ATLAS/g
#%s/   / /g
#%s/  / /g

point=p01
sysSet=allSys

for point in p01 p0d25
do
for sysSet in allSys jetSys ssSys theoSys photonSys
do
fitRslt=${point}_${sysSet}Fit.txt

list=
for line in $(cat ${fitRslt} | sed 's/ /?/g')
do
  sysname=$(echo ${line} | cut -d ? -f 1)
  num=$(echo ${line} | cut -d ? -f 2)
  list="${list},${sysname}=${num}"
done

echo ${list} > fix${sysSet}_${point}
done
done
