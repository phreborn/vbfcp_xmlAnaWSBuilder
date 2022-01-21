#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys

for i in range(0, 50):
  d_nlls=[]
  with open("../autonll/dNLL_Toy%i_statOnly.log"%(i), 'r') as fin:
    for line in fin.readlines():
      line = line.replace('\n', '')
      lcont = line.split(' ')
      d_nlls.append([float(lcont[0]), float(lcont[1])])

  minpos=0
  count=0
  for dnll in d_nlls:
    if dnll[1] == 0: minpos = count
    count+=1

  #print 'Toy%i'%(i),minpos,d_nlls[minpos][0],d_nlls[minpos][1]

  print 'Toy%i'%(i)
  with open("autonll/dNLL_Toy%i_statOnly.log"%(i), 'w') as fout:
    count=0
    for dnll in d_nlls:
      if abs(minpos-count) <= 3:
        print dnll[0], dnll[1]
        fout.write('%f %f\n'%(dnll[0], dnll[1]))
      count+=1
