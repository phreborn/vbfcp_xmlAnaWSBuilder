tag=ggF

preCfg="AllCats"
#preCfg="noLL"
#preCfg="StatOnly"

allJobs=jobsSub.sh
> ${allJobs}

#if [ ! -d config${preCfg} ];then mkdir config${preCfg};fi
if [ ! -d WS${preCfg} ];then mkdir WS${preCfg};fi
if [ ! -d out${preCfg}_allSys ];then mkdir out${preCfg}_allSys;fi

echo "copying config/..."
#cp -r config/* config${preCfg}/
cp -r ../config${preCfg}/* config/
echo ""

#dList=($(cat ../../../syst/cHW | grep -v "#"))
dList=($(cat Dtilde | grep -v "#"))

sequence=($(seq 1 1 ${#dList[@]}))

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
  echo "source setup_lxplus.sh" >> exe_${jobName}.sh
  echo "cd run/impactOnDtilde" >> exe_${jobName}.sh
  for num in `seq ${init} 1 ${fin}`;do
    echo "" >> exe_${jobName}.sh
    echo "../../bin/XMLReader -x config/vbf_cp_${dList[${num}]}/d_tilde_${dList[${num}]}.xml" >> exe_${jobName}.sh
    echo "if [ ! -d WS${preCfg} ];then mkdir WS${preCfg};fi" >> exe_${jobName}.sh
    echo "if [ -d WS${preCfg}/vbf_cp_${dList[${num}]} ];then rm -r WS${preCfg}/vbf_cp_${dList[${num}]};fi" >> exe_${jobName}.sh
    echo "cp -r workspace/vbf_cp_${dList[${num}]} WS${preCfg}" >> exe_${jobName}.sh
#    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d asimovData_SB_SM -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5 -o out${preCfg}_allSys/out_${dList[${num}]}.root --savefitresult 1" >> exe_${jobName}.sh
  done

  chmod +x exe_${jobName}.sh

  echo "hep_sub exe_${jobName}.sh -g atlas -os CentOS7 -wt mid -mem 2048 -o hep_sub_${jobName}/log-0.out -e hep_sub_${jobName}/log-0.err" >> ${allJobs}

  if [ "$(ls hep_sub_${jobName}/)" != "" ];then rm hep_sub_${jobName}/*;fi
done
