#!/bin/bash

for d in `ls ${1}/vbf_cp_* -d`;do
  dtilde=`echo ${d} | cut -d "-" -f 2 | cut -d "_" -f 3`
  cp /scratchfs/atlas/huirun/atlaswork/VBF_CP/syst/config/vbf_cp_${dtilde}/d_tilde_${dtilde}.xml ${1}/vbf_cp_${dtilde}/d_tilde_${dtilde}.xml
done
