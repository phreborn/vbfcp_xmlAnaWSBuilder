#!/bin/bash

pois="mu=1,mu_yy=1,mu_spur_SM=0,mu_ggH_SM=0,mu_VBF_SM=0,mu_spur=1,mu_ggH=1,mu_VBF_RW=1_0_5"

dataset=asimovData_SB_SM
dataset=combData

preCfg="AllCats"
#preCfg="noLL"
#preCfg="StatOnly"

allJobs=jobsSubFit.sh
> ${allJobs}

#if [ ! -d config${preCfg} ];then mkdir config${preCfg};fi
if [ ! -d WS${preCfg} ];then mkdir WS${preCfg};fi
if [ ! -d out${preCfg}_allSys ];then mkdir out${preCfg}_allSys;fi

#echo "copying config/..."
#cp -r config/* config${preCfg}/
#cp -r config${preCfg}/* config/
#echo ""

NPs=$(cat postNPErr.txt | sed 's/ /,/g')

#dList=($(cat ../../../syst/cHW | grep -v "#"))
dList=($(cat Dtilde | grep -v "#"))

sequence=($(seq 1 7 ${#dList[@]}))

intvl=6
for np in ${NPs}
do
npname=$(echo ${np} | cut -d , -f 1)
nphat=$(echo ${np} | cut -d , -f 2)
npup=$(echo ${np} | cut -d , -f 3)
npdn=$(echo ${np} | cut -d , -f 4)
for init in ${sequence[@]};do
  init=$((${init} - 1))
  fin=$((${init} + ${intvl}))
  jobName=Collect_${init}_${fin}_${npname}; echo ${jobName}
  #if [ ! -d csv/${jobName} ];then mkdir -p csv/${jobName};fi
  if [ ! -d hep_sub_${jobName} ]; then mkdir hep_sub_${jobName}; fi
  executable=exe_${jobName}.sh
  > ${executable}

  echo "#!/bin/bash" >> ${executable}
  echo "" >> ${executable}
  echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/WSBuilder/quickFit" >> ${executable}
  echo "source setup_lxplus.sh" >> ${executable}
  echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/WSBuilder/xmlAnaWSBuilder" >> ${executable}
  echo "source setup_lxplus.sh" >> ${executable}
  echo "cd run/impactOnDtilde" >> ${executable}
  echo "echo \$(date \"+%Y-%m-%d %H:%M:%S\")" >> ${executable}
  for num in `seq ${init} 1 ${fin}`;do
    echo "" >> ${executable}
#    echo "../../bin/XMLReader -x config/vbf_cp_${dList[${num}]}/d_tilde_${dList[${num}]}.xml" >> ${executable}
#    echo "if [ ! -d WS${preCfg} ];then mkdir WS${preCfg};fi" >> ${executable}
#    echo "if [ -d WS${preCfg}/vbf_cp_${dList[${num}]} ];then rm -r WS${preCfg}/vbf_cp_${dList[${num}]};fi" >> ${executable}
#    echo "cp -r workspace/vbf_cp_${dList[${num}]} WS${preCfg}" >> ${executable}
    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p ${pois},${npname}=${npup} -o out${preCfg}_allSys/out_${dList[${num}]}_${npname}_up_post.root --savefitresult 1" >> ${executable}
    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p ${pois},${npname}=${npdn} -o out${preCfg}_allSys/out_${dList[${num}]}_${npname}_dn_post.root --savefitresult 1" >> ${executable}
    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p ${pois},${npname}=1 -o out${preCfg}_allSys/out_${dList[${num}]}_${npname}_up_pre.root --savefitresult 1" >> ${executable}
    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p ${pois},${npname}=-1 -o out${preCfg}_allSys/out_${dList[${num}]}_${npname}_dn_pre.root --savefitresult 1" >> ${executable}
  done
  echo "echo \$(date \"+%Y-%m-%d %H:%M:%S\")" >> ${executable}

  chmod +x ${executable}

  echo "hep_sub ${executable} -g atlas -os CentOS7 -wt mid -mem 2048 -o hep_sub_${jobName}/log-0.out -e hep_sub_${jobName}/log-0.err" >> ${allJobs}

  if [ "$(ls hep_sub_${jobName}/)" != "" ];then rm hep_sub_${jobName}/*;fi
done
done
