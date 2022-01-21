#!/bin/bash

allJobs=jobsSub.sh
> ${allJobs}

cfpath=/publicfs/atlas/atlasnew/higgs/hgg/chenhr/vbfcp/WSconfigs/configToys/

for i in $(seq 0 49)
do
  preCfg=Toy${i}

  #echo "copying config toy ${i} ..."
  #cp /publicfs/atlas/atlasnew/higgs/hgg/chenhr/vbfcp/WSconfigs/configToys/config${preCfg} . -r

  if [ ! -d out${preCfg}_statOnly ];then mkdir out${preCfg}_statOnly;fi
  #if [ ! -d out${preCfg}_allSys ];then mkdir out${preCfg}_allSys;fi
  
  #dList=($(cat ../../../syst/cHW | grep -v "#"))
  dList=($(cat ../../../syst/Dtilde | grep -v "#"))
  
  sequence=($(seq 1 1 ${#dList[@]}))
  
  jobName=Collect_${preCfg}; echo ${jobName}
  if [ ! -d hepsub_${jobName} ]; then mkdir hepsub_${jobName}; fi
  executable=exe_${jobName}.sh
  > ${executable}
  
  echo "#!/bin/bash" >> ${executable}
  echo "" >> ${executable}
  echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/WSBuilder/quickFit" >> ${executable}
  echo "source setup_lxplus.sh" >> ${executable}
  echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/WSBuilder/xmlAnaWSBuilder" >> ${executable}
  echo "source setup_lxplus.sh" >> ${executable}
  echo "cd run" >> ${executable}
  wsname=ws${preCfg}
  echo "if [ ! -d ${wsname} ];then mkdir ${wsname};fi" >> ${executable}
  
  intvl=0
  for init in ${sequence[@]};do
    init=$((${init} - 1))
    fin=$((${init} + ${intvl}))
    for num in `seq ${init} 1 ${fin}`;do
      echo "" >> ${executable}
      echo "if [ -d ${wsname}/vbf_cp_${dList[${num}]} ];then rm -r ${wsname}/vbf_cp_${dList[${num}]};fi" >> exe_${jobName}.sh
      echo "mkdir ${wsname}/vbf_cp_${dList[${num}]}" >> ${executable}
      echo "../bin/XMLReader -x ${cfpath}/config${preCfg}/vbf_cp_${dList[${num}]}/d_tilde_${dList[${num}]}.xml" >> ${executable}
      echo "quickFit -f ${wsname}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d combData -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5 -n ATLAS_*,*_bfb -o out${preCfg}_statOnly/out_${dList[${num}]}.root --savefitresult 1" >> ${executable}
  #    echo "quickFit -f ${wsname}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d asimovData_SB_SM -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5 -o out${preCfg}_allSys/out_${dList[${num}]}.root --savefitresult 1" >> ${executable}
    done
  done
  chmod +x ${executable}
  
  echo "hep_sub ${executable} -g atlas -os CentOS7 -wt mid -mem 2048 -o hepsub_${jobName}/log-0.out -e hepsub_${jobName}/log-0.err" >> ${allJobs}
  
  if [ "$(ls hepsub_${jobName}/)" != "" ];then rm hepsub_${jobName}/*;fi
done
