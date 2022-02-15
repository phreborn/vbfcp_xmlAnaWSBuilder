tag=ggF

dataset=asimovData_SB_SM
dataset=combData

preCfg="AllCats"
pname=p01
preCfg="AllCats_SMEFT"
pname=p0d25
#preCfg="LL"
#preCfg="StatOnly"

allJobs=jobsSub.sh
> ${allJobs}

#dList=($(cat ../../../syst/cHW | grep -v "#"))
dList=($(cat ../../../syst/Dtilde | grep -v "#"))
dList=($(cat ../../../syst/cHW_fine | grep -v "#"))

sequence=($(seq 1 1 ${#dList[@]}))

sysSet=allSys

intvl=0
for init in ${sequence[@]};do
  init=$((${init} - 1))
  fin=$((${init} + ${intvl}))
  jobName=Collect_${init}_${fin}; echo ${jobName}
  #if [ ! -d csv/${jobName} ];then mkdir -p csv/${jobName};fi
  if [ ! -d hep_sub_${jobName} ]; then mkdir hep_sub_${jobName}; fi
  executable=exe_${jobName}.sh
  > ${executable}

  echo "#!/bin/bash" >> exe_${jobName}.sh
  echo "" >> exe_${jobName}.sh
  echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/WSBuilder/quickFit" >> exe_${jobName}.sh
  echo "source setup_lxplus.sh" >> exe_${jobName}.sh
  echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/WSBuilder/xmlAnaWSBuilder" >> exe_${jobName}.sh
#  echo "source setup_lxplus.sh" >> exe_${jobName}.sh
  echo "cd run" >> exe_${jobName}.sh
  for num in `seq ${init} 1 ${fin}`;do

  for sysSet in allSys jetSys ssSys theoSys photonSys
  do
    sysfixlist=$(cat fix${sysSet}_${pname})
    if [ ! -d out${preCfg}_pull${sysSet} ];then mkdir out${preCfg}_pull${sysSet};fi

    echo "" >> exe_${jobName}.sh
    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5${sysfixlist} -n ATLAS_* -o out${preCfg}_pull${sysSet}/out_${dList[${num}]}.root --savefitresult 1" >> exe_${jobName}.sh
  done
  done

  chmod +x exe_${jobName}.sh

  echo "hep_sub exe_${jobName}.sh -g atlas -os CentOS7 -wt mid -mem 2048 -o hep_sub_${jobName}/log-0.out -e hep_sub_${jobName}/log-0.err" >> ${allJobs}

  if [ "$(ls hep_sub_${jobName}/)" != "" ];then rm hep_sub_${jobName}/*;fi
done
