#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys

from ROOT import *
from math import sin
from array import array

#gROOT.SetBatch()

with open("intvalwidth.txt", 'w') as fout:
  for itoy in range(0, 500):
    print "Toy%i"%(itoy)

    dirname = "../autonll/"
    #inNLL = dirname + "dNLL_testest.log"
    inNLL = dirname + "dNLL_Toy%i_statOnly.log"%(itoy)
  
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
        #print nll, nlls[nll]
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
  
    ### evaluating intervals ###
    for Ord in truncs.keys():
      nllp, nllv = array('d'), array('d')
      for dval in truncs[Ord][1]:
        nllp.append(dval)
        nllv.append(nlls[dval])
  
      gr = TGraph(len(nllv), nllv, nllp)
      if truncs[Ord][0] == -1:
        #print 'best fit:', abs(gr.Eval(0))
        l1sigma = gr.Eval(0.5)
        l2sigma = gr.Eval(1.92)
        print '68%CL interval left:', l1sigma
        print '95%CL interval left:', l2sigma
      if truncs[Ord][0] == 1:
        r1sigma = gr.Eval(0.5)
        r2sigma = gr.Eval(1.92)
        print '68%CL interval right:', r1sigma
        print '95%CL interval right:', r2sigma

    fout.write("%f %f\n"%(r1sigma-l1sigma, r2sigma-l2sigma))
