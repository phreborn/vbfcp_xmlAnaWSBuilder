#!/bin/bash

outf=FitRlstOut.txt
> ${outf}
for it in $(seq 0 149)
do
  echo toy${it} >> ${outf}
  str=($(cat hepsub_Collect_Toy${it}/log-0.out | grep "Fit Summary of POIs" | cut -d ' ' -f 9))
  echo ${str[*]} >> ${outf}
done
