#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys

from ROOT import *
from math import sin
from array import array

#gROOT.SetBatch()

syslist = []
with open("postNPErr.txt", 'r') as f:
  for line in f.readlines():
    line = line.replace('\n', '')
    if line == '': continue
    syslist.append(line.split(' ')[0])

#with open("impactDtilde.txt", 'w') as fout:
# for sysname in syslist:
#  #fout = TFile("../../../StatisticsTools/root-files/dtilde/pulls/"+sysname+".root", 'recreate')
#  dtilde = []
#  for fitType in ['up_post', 'dn_post', 'up_pre', 'dn_pre']:
dirname = "autonll/"
#inNLL = dirname + "dNLL_testest.log"
#inNLL = dirname + "dNLL_AllCats_allSys_nominal.log"
inNLL = dirname + "dNLL_AllCats_allSys_ATLAS_Hgg_BIAS_OO_TT_b6_up_post.log"

### reading NLL points and ordering ###
nlls = {}
with open(inNLL, 'r') as f:
  for line in f.readlines():
    nll = line.split(' ')
    nlls[float(nll[0])] = float(nll[1])

if len(nlls) < 2:
  print 'NLL points less than 2, exiting.'
  sys.exit(1)

ordnll = sorted(nlls.keys())

#pnll = array('f')
#vnll = array('f')
pnll = []
vnll = []
for nll in ordnll:
    print nll, nlls[nll]
    pnll.append(nll)
    vnll.append(nlls[nll])

##precision lost in transformation to array
#for i in range(len(pnll)): print pnll[i], vnll[i]

### truncating NLL curve ###
truncs = {}
truncount = 0
tmplist = []
slope = 0
for i in range(len(nlls)):
  phere = vnll[i]
  pnext = 0
  plast = 0
  if i == 0: plast = phere-(vnll[i+1]-phere)
  else: plast = vnll[i-1]
  if i == len(nlls)-1: pnext = phere
  else: pnext = vnll[i+1]

  diff = phere - plast
  if diff != 0: slope = 1 if diff > 0 else -1

  tmplist.append(pnll[i])
  if ((plast-phere)*(pnext-phere) >= 0 and (pnext-phere) != 0) or i == len(nlls)-1:
    truncs[truncount] = [slope, tmplist]
    truncount += 1
    tmplist = []
    slope = 0

### get best fit ###
ary_pnll = array('d')
ary_vnll = array('d')
for i in range(len(nlls)):
  ary_pnll.append(pnll[i])
  ary_vnll.append(vnll[i])
gr_bf = TGraph(len(nlls), ary_pnll, ary_vnll)
fpol2 = TF1('poly2', 'pol2', -0.03, 0.03)
gr_bf.Fit(fpol2, 'W')
p1 = fpol2.GetParameter(1)
p2 = fpol2.GetParameter(2)
print 'best fit:',-p1/(2*p2)
gr_bf.Draw()

### evaluating intervals ###
for Ord in truncs.keys():
  nllp, nllv = array('d'), array('d')
  for dval in truncs[Ord][1]:
    nllp.append(dval)
    nllv.append(nlls[dval])

  gr = TGraph(len(nllv), nllv, nllp)
  if truncs[Ord][0] == -1:
    #print 'best fit:', abs(gr.Eval(0))
    print '68%CL interval left:', gr.Eval(0.5)
    print '95%CL interval left:', gr.Eval(1.92)
  if truncs[Ord][0] == 1:
    print '68%CL interval right:', gr.Eval(0.5)
    print '95%CL interval right:', gr.Eval(1.92)
