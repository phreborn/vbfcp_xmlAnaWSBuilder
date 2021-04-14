for d in `ls config/vbf_cp_* -d`;do
  dtilde=`echo ${d} | cut -d "-" -f 2 | cut -d "_" -f 3`
  cp /scratchfs/atlas/chenhr/atlaswork/VBF_CP/syst/config/vbf_cp_${dtilde}/d_tilde_${dtilde}.xml config/vbf_cp_${dtilde}/d_tilde_${dtilde}.xml
done
