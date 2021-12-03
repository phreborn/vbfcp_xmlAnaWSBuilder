#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys

from ROOT import *
from math import sin
from array import array

gROOT.SetBatch()

dirname = "updatedBDT/"
inNLL = dirname + "dNLL_noLL_allSys.log"

#{
#gr = TGraph(inNLL, '%lg %lg')
#------- or --------
pnll = array('f')
vnll = array('f')
with open(inNLL, 'r') as f:
  for line in f.readlines():
    nll = line.split(' ')
    pnll.append(float(nll[0]))
    vnll.append(float(nll[1]))

gr = TGraph(len(pnll), pnll, vnll)
#}

c = TCanvas('c', '', 200, 10, 1400, 1120)

gr.Draw()

c.SaveAs(dirname+"python_NLL.png")
c.SaveAs(dirname+"python_NLL.pdf")
