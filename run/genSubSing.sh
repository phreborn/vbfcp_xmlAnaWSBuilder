#!/bin/bash

decompSys=0

pois="mu=1,mu_yy=1,mu_ggH_SM=0,mu_VBF_SM=0,mu_ggH=1,mu_VBF_RW=1_0_5"

injectTest=0
injectPoint=p01
if [ ${injectTest} -eq 1 ];then pois="${pois},mu_ggH_${injectPoint}=0,mu_VBF_${injectPoint}=0";fi

# 0: no, 1: Asimov data exported for m00 workspace, 2: both SM syst and BSM syst
unblindAsi=2
if [ ${unblindAsi} -eq 2 ];then pois="${pois},mu_spur=1,mu_spur_SM=0";fi

dataset=asimovData_SB_p01
dataset=asimovData_SB_SM
dataset=asimovData_SM_floatMu
dataset=asimovData_SM_Mu1
dataset=combData

quitCopy=1
wsExist=0

preCfg="ExtCheck"
preCfg="LT_p01Asi"
preCfg="AllCats_w1"
preCfg="TT_w1"
preCfg="AllCats_m00forAsi"
preCfg="AllCats_unblindAsi"
preCfg="AllCats"

tole=0.001

allJobs=jobsSub.sh
> ${allJobs}

if [ ! -d config${preCfg} ];then mkdir config${preCfg};fi
if [ ! -d WS${preCfg} ];then mkdir WS${preCfg};fi
if [ ! -d out${preCfg}_statOnly ];then mkdir out${preCfg}_statOnly;fi
if [ ! -d out${preCfg}_allSys ];then mkdir out${preCfg}_allSys;fi
if [ ${decompSys} -eq 1 ];then
  if [ ! -d out${preCfg}_jetSys ];then mkdir out${preCfg}_jetSys;fi
  if [ ! -d out${preCfg}_photonSys ];then mkdir out${preCfg}_photonSys;fi
  if [ ! -d out${preCfg}_ssSys ];then mkdir out${preCfg}_ssSys;fi
  if [ ! -d out${preCfg}_theorySys ];then mkdir out${preCfg}_theorySys;fi
fi

echo "copying config/..."
if [ ${wsExist} -eq 1 -o ${quitCopy} -eq 1 ];then
  echo "workspaces or config files exist, skip copying"
else
cp -r config${preCfg}/* config/
fi
echo ""

#dList=($(cat ../../../syst/cHW | grep -v "#"))
dList=($(cat ../../../syst/Dtilde | grep -v "#"))

sequence=($(seq 1 1 ${#dList[@]}))

condor=condor
intvl=0
for init in ${sequence[@]};do
  init=$((${init} - 1))
  fin=$((${init} + ${intvl}))
  jobName=Collect_${init}_${fin}; echo ${jobName}
  #if [ ! -d csv/${jobName} ];then mkdir -p csv/${jobName};fi
  hepout=${condor}/hep_sub_${jobName}
  if [ ! -d ${hepout} ]; then mkdir ${hepout}; fi
  executable=${condor}/exe_${jobName}.sh
  > ${executable}
  singexec=${condor}/singexe_${jobName}.sh
  > ${singexec}

  echo "#!/bin/bash" >> ${singexec}
  echo "" >> ${singexec}
  for num in `seq ${init} 1 ${fin}`;do
    echo "" >> ${singexec}
    echo "cd /scratchfs/atlas/huirun/atlaswork/VBF_CP/WSBuilder/xmlAnaWSBuilder/run" >> ${singexec}
    echo "singularity exec /cvmfs/unpacked.cern.ch/gitlab-registry.cern.ch/atlas_higgs_combination/software/hcomb-docker/analyzer:2-2 ${executable}" >> ${singexec}
  done

  echo "#!/bin/bash" >> ${executable}
  echo "" >> ${executable}
  for num in `seq ${init} 1 ${fin}`;do
    echo "" >> ${executable}
    if [ ${wsExist} -eq 1 ];then
      echo "workspace exists, skip building"
    else
    echo "XMLReader -x config/vbf_cp_${dList[${num}]}/d_tilde_${dList[${num}]}.xml -t ${tole}" >> ${executable}
    echo "if [ ! -d WS${preCfg} ];then mkdir WS${preCfg};fi" >> ${executable}
    echo "if [ -d WS${preCfg}/vbf_cp_${dList[${num}]} ];then rm -r WS${preCfg}/vbf_cp_${dList[${num}]};fi" >> ${executable}
    echo "cp -r workspace/vbf_cp_${dList[${num}]} WS${preCfg}" >> ${executable}
    fi
#    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p ${pois} -n ATLAS_* -o out${preCfg}_statOnly/out_${dList[${num}]}.root --savefitresult 1 --saveWS 1 --minTolerance ${tole}" >> ${executable}
#    echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p ${pois} -o out${preCfg}_allSys/out_${dList[${num}]}.root --savefitresult 1 --saveWS 1 --minTolerance ${tole}" >> ${executable}
    if [ ${decompSys} -eq 1 ];then
      echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5 -n ATLAS_PH*,ATLAS_EG*,*PRW*,*pdf*,*aS*,*qcd*,*shower*,*BIAS*,*lumi*,*HIGGS_MASS*,*rest_Higgs*,*mcstat* -o out${preCfg}_jetSys/out_${dList[${num}]}.root --savefitresult 1 --saveWS 1" >> ${executable}
      echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5 -n ATLAS_JET*,*PRW*,*pdf*,*aS*,*qcd*,*shower*,*BIAS*,*lumi*,*HIGGS_MASS*,*rest_Higgs*,*mcstat* -o out${preCfg}_photonSys/out_${dList[${num}]}.root --savefitresult 1 --saveWS 1" >> ${executable}
      echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5 -n ATLAS_JET*,ATLAS_PH*,ATLAS_EG*,*PRW*,*pdf*,*aS*,*qcd*,*shower*,*lumi*,*HIGGS_MASS*,*rest_Higgs*,*mcstat* -o out${preCfg}_ssSys/out_${dList[${num}]}.root --savefitresult 1 --saveWS 1" >> ${executable}
      echo "quickFit -f WS${preCfg}/vbf_cp_${dList[${num}]}/vbf_cp_${dList[${num}]}.root -w combWS -d ${dataset} -p mu=1,mu_VBF_SM=0,mu_ggH=1,mu_ggH_SM=0,mu_VBF_RW=1_0_5 -n ATLAS_JET*,ATLAS_PH*,ATLAS_EG*,*PRW*,*BIAS*,*lumi*,*HIGGS_MASS*,*rest_Higgs*,*mcstat* -o out${preCfg}_theorySys/out_${dList[${num}]}.root --savefitresult 1 --saveWS 1" >> ${executable}
    fi
  done

  chmod +x ${executable}
  chmod +x ${singexec}

  echo "hep_sub ${singexec} -g atlas -os CentOS7 -wt mid -mem 4096 -o ${hepout}/log-0.out -e ${hepout}/log-0.err" >> ${allJobs}

  if [ "$(ls ${hepout}/)" != "" ];then rm ${hepout}/*;fi
done
