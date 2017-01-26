#!/bin/bash

export SCRAM_ARCH=slc6_amd64_gcc530
cd /afs/cern.ch/work/m/mshi/private/CMSSW_8_0_17/src
eval `scramv1 runtime -sh`
cd -
source /afs/cern.ch/cms/caf/setup.sh
CFG_FILE="QCD_1000ToInf_noSelection.py"
cmsRun /afs/cern.ch/work/m/mshi/private/CMSSW_8_0_17/src/GGHAA2Mu2TauAnalysis/SkimMuMuTauTau/test/QCDBackground/$CFG_FILE > QCD_1000ToInf_noSelection.txt
rfcp QCD_1000ToInf_noSelection.txt /afs/cern.ch/work/m/mshi/private/CMSSW_8_0_17/src/GGHAA2Mu2TauAnalysis/SkimMuMuTauTau/test/QCDBackground/QCD_1000ToInf_noSelection.txt
rfcp QCD_1000ToInf_noSelection.root /afs/cern.ch/work/m/mshi/private/CMSSW_8_0_17/src/GGHAA2Mu2TauAnalysis/SkimMuMuTauTau/test/QCDBackground/QCD_1000ToInf_noSelection.root

exit 0