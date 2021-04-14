tag=ggF

preCfg="LL"

allJobs=jobsSub.sh
> ${allJobs}

if [ ! -d WS${preCfg} ];then mkdir WS${preCfg};fi
if [ ! -d out${preCfg} ];then mkdir out${preCfg};fi

sysList=($(cat ../../../syst/Dtilde | grep -v "#"))

sequence=($(seq 1 1 ${#sysList[@]}))

intvl=0
for init in ${sequence[@]};do
  init=$((${init} - 1))
  fin=$((${init} + ${intvl}))
  jobName=Collect_${init}_${fin}; echo ${jobName}
  #if [ ! -d csv/${jobName} ];then mkdir -p csv/${jobName};fi
  if [ ! -d submit_${jobName} ]; then mkdir submit_${jobName}; fi
  executable=exe_${jobName}.sh
  > ${executable}

  echo "#!/bin/bash" >> exe_${jobName}.sh
  echo "" >> exe_${jobName}.sh
  echo "cd /scratchfs/atlas/chenhr/atlaswork/VBF_CP/WSBuilder/quickFit" >> exe_${jobName}.sh
  echo "source setup_lxplus.sh" >> exe_${jobName}.sh
  echo "cd /scratchfs/atlas/chenhr/atlaswork/VBF_CP/WSBuilder/xmlAnaWSBuilder" >> exe_${jobName}.sh
  echo "source setup_lxplus.sh" >> exe_${jobName}.sh
  echo "cd run" >> exe_${jobName}.sh
  for num in `seq ${init} 1 ${fin}`;do
    echo "" >> exe_${jobName}.sh
    echo "../bin/XMLReader -x config${preCfg}/vbf_cp_${sysList[${num}]}/d_tilde_${sysList[${num}]}.xml" >> exe_${jobName}.sh
    echo "cp -r workspace/vbf_cp_${sysList[${num}]} WS${preCfg}" >> exe_${jobName}.sh
    echo "quickFit -f WS${preCfg}/vbf_cp_${sysList[${num}]}/vbf_cp_${sysList[${num}]}.root -w combWS -d asimovData_SB_SM -p mu=1,mu_VBF_SM=0,mu_VBF_RW=1_0_5 -n ATLAS_* -o out${preCfg}/out_${sysList[${num}]}.root --savefitresult 1" >> exe_${jobName}.sh
  done

  chmod +x exe_${jobName}.sh

  echo "hep_sub exe_${jobName}.sh -g atlas -os CentOS7 -wt mid -mem 2048 -o submit_${jobName}/log-0.out -e submit_${jobName}/log-0.err" >> ${allJobs}

  if [ "$(ls submit_${jobName}/)" != "" ];then rm submit_${jobName}/*;fi
done
