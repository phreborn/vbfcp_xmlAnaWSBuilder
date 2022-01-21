#!/bin/bash

outf=toy_bestfits.txt
> ${outf}

for i in $(seq 0 49)
do
  echo Toy${i}
  python getBestFitNom.py ${i} | grep "best fit:" | cut -d " " -f 3 | tee -a ${outf}
done
