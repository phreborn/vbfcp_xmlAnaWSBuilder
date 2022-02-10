#!/usr/bin/python
# -*- coding: UTF-8 -*-

from ROOT import *
from sys import exit
import os

try:
    import numpy as np
except:
    print("Failed to import numpy.")
    exit()

dirpath = "../../../StatisticsTools/root-files/p01/pulls/"
#dirpath = "../../../StatisticsTools/root-files/p0d25/pulls/"

outfile = "postNPErr.txt"
with open(outfile, 'w') as fout:
  for fname in os.listdir(dirpath):
    sysname = fname.replace('.root', '')
    f = TFile(dirpath+fname, 'r')
    tree = f.Get('result')
    array, label = tree.AsMatrix(columns=['nuis_hat', 'nuis_hi', 'nuis_lo'], return_labels=True)
    print sysname, array[0][0], array[0][1], array[0][2]
    fout.write('\n%s %f %f %f'%(sysname, array[0][0], array[0][1], array[0][2]))
