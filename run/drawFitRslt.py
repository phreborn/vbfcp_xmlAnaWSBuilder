#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys

from ROOT import *
from math import sin
from array import array
import argparse

gROOT.SetBatch()

gROOT.LoadMacro("AtlasStyle.C")
gROOT.LoadMacro("AtlasUtils.C")
gROOT.LoadMacro("AtlasLabels.C")

SetAtlasStyle()

gStyle.SetErrorX(0.5); # !!! must after SetAtlasStyle()
gStyle.SetOptStat(0)

parser = argparse.ArgumentParser()
parser.add_argument('-p', '--para', type=str, default='mu_VBF_RW')

result = parser.parse_args()

paraname = result.para

def transD(dstr):
 nstr = '-999'
 if 'm' in dstr:
  nstr = dstr.replace('m', '-0.')
 if 'p' in dstr:
  nstr = dstr.replace('p', '+0.')
 return float(nstr)

dval = []
with open("../../../syst/Dtilde", 'r') as f:
 for line in f.readlines():
  if '#' in line: continue
  line = line.replace('\n', '')
  lattrs = line.split(' ')
  for attr in lattrs:
   dval.append(attr)

pairs = {}
dnums = []
for dstr in dval:
 f = TFile("outAllCats_allSys/out_%s.root"%(dstr), 'read')
 fr = f.Get('fitResult')
 dnum = transD(dstr)
 dnums.append(dnum)

 print dstr, dnum

 paralist = fr.floatParsFinal()
 #for para in paralist:
 # pname = para.GetName()
 # if pname != paraname: continue
 # paraval = para.getVal()
 # print pname, paraval
 # pairs[dnum] = paraval
 para = paralist.find(paraname)
 paraval = para.getVal()
 paraerr = para.getError()
 print paraname, paraval, '+/-', paraerr
 pairs[dnum] = [paraval, paraerr]

xps, yps = array('f'), array('f')
exps, eyps = array('f'), array('f')
dums = sorted(dnums)
for d in dums:
 xps.append(d)
 yps.append(pairs[d][0])
 exps.append(0)
 eyps.append(pairs[d][1])

c = TCanvas("c", "canvas", 800, 600);

# also have TGraphAsymmErrors
curve = TGraphErrors(len(xps), xps, yps, exps, eyps)

curve.GetXaxis().SetTitle('#tilde{d}')
curve.GetYaxis().SetTitle(paraname)
curve.Draw('ap')

c.SaveAs("fitParDepDtilde/%s.png"%(paraname))
