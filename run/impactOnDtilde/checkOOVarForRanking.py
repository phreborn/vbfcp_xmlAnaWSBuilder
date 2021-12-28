#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys

from ROOT import *

gROOT.SetBatch()

#csvpath = "../../../../syst/yield_sys/ext_csv/"
csvpath = "/publicfs/atlas/atlasnew/higgs/hgg/chenhr/vbfcp/syst/yield/ext_csv/"
csvjer = "/publicfs/atlas/atlasnew/higgs/hgg/chenhr/vbfcp/syst/yield/ext_csv_jd/"
outdir = "plotForImpactCheck/"

dtildes = ['m00', 'm00010', 'm00015', 'm00020']

bins = {
1:"-99:-2",
2:"-2:-1",
3:"-1:0",
4:"0:1",
5:"1:2",
6:"2:99",
}

def readlines(fpath):
 with open(fpath, 'r') as f:
  return f.readlines()

def getNPMag(fpath, sys):
 for line in readlines(fpath):
  lattrib = line.split(',')
  sysname = lattrib[0]
  if sysname != sys: continue
  up = abs(float(lattrib[1]))
  dn = abs(float(lattrib[2]))
  #return up if up > dn else dn
  return float(lattrib[1])

def getYield(fpath, catbin):
 for line in readlines(fpath):
  lattr = line.split(',')
  if lattr[0] != catbin: continue
  return float(lattr[1])

syslist = []
with open(csvpath+'mag_yield_346214_m00_LT_b5.csv', 'r') as f:
  for line in f.readlines():
    syslist.append(line.split(',')[0])

Hists = {}
Hasis = {}
yfile = csvpath+"N_yield.csv"
#yfile = "/scratchfs/atlas/huirun/atlaswork/VBF_CP/syst/yield_sys/ext_csv/N_yield.csv"
#for sys in syslist:
for sys in ['JET_JER_EffectiveNP_11', 'JET_Flavor_Composition']:
  for cat in ['TT', 'TL', 'LT', 'LL']:
    c = TCanvas('c', '', 800, 600)
    lg = TLegend(0.6, 0.7, 0.93, 0.9)
    lg.SetBorderSize(0)
    lg.SetFillColorAlpha(kBlue, 0)
    countd = 0
    #for dtilde in ['m00', 'm01', 'p01', 'm10', 'p10']:
    for dtilde in dtildes:
      syscatd = sys+'_'+cat+'_'+dtilde
      hist = TH1F(sys+'_'+dtilde+'_'+cat, '', 6, 0, 6)
      hasi = TH1F('asimov_VBF_'+sys+'_'+dtilde+'_'+cat, '', 6, 0, 6)
      hggF = TH1F('ggF_variation_'+sys+'_'+dtilde+'_'+cat, '', 6, 0, 6)
      for bnum in range(len(bins)):
        bnum = bnum+1
        bname = 'b%i'%(bnum)
        catbin = 'VBF_'+dtilde+'_'+cat+'_'+bname
        catm00 = 'VBF_m00_'+cat+'_'+bname
        catggF = 'ggH_'+cat+'_'+bname
        csvfile = csvpath+'mag_yield_346214_'+dtilde+'_'+cat+'_'+bname+'.csv'
        csvggF = csvpath+'mag_yield_343981_SM_'+cat+'_'+bname+'.csv'
        if 'JER' in sys:
          csvfile = csvjer+'mag_yield_346214_'+dtilde+'_'+cat+'_'+bname+'.csv'
          csvggF = csvjer+'mag_yield_343981_SM_'+cat+'_'+bname+'.csv'
        magnitude = getNPMag(csvfile, sys)
        magggF = getNPMag(csvggF, sys)
        yvbf = getYield(yfile, catbin)
        yasi = getYield(yfile, catm00)
        yggF = getYield(yfile, catggF)
        print 'sys catbin yasi yvbf yggF magvbf magggF'
        print sys, catbin, yasi, yvbf, yggF, magnitude, magggF
        #hist.GetXaxis().SetBinLabel(bnum, bins[bnum])
        #hist.SetBinContent(bnum, yvbf*magnitude)
        hist.SetBinContent(bnum, (1+magnitude)*yvbf)
        hasi.SetBinContent(bnum, yasi)
        hggF.SetBinContent(bnum, yggF*magggF)
      print sys+'_'+dtilde+'_'+cat, hist.GetMean()-3
      fout = TFile('checkRanking_'+sys+'_'+dtilde+'_'+cat+'.root', 'recreate')
      fout.cd()
      hist.Write()
      hasi.Write()
      hggF.Write()
      fout.Close()
      hasi.Add(hggF, -1)
      hasi.SetLineColor(kRed)
      hasi.Scale(1/hasi.Integral())
      hist.SetStats(0)
      hist.Scale(1/hist.Integral())
      hist.SetLineColor(kBlue+7*countd)
      #hist.SetMaximum(1.01)
      #hist.SetMinimum(0.99)
      hist.Divide(hasi)
      hasi.Divide(hasi)
      Hists[syscatd] = hist
      Hasis[syscatd] = hasi
      #lg.AddEntry(hist, dtilde, 'l')
      #if countd == 1:
      #  hist.Draw('hist')
      #  lg.AddEntry(hasi, 'Asimov-ggF_vari', 'l')
      #  hasi.Draw('same hist')
      #else:
      #  hist.Draw('same hist')
      countd += 1
    countd = 0
    #for dtilde in ['m00', 'm01', 'p01', 'm10', 'p10']:
    for dtilde in dtildes:
      syscatd = sys+'_'+cat+'_'+dtilde
      lg.AddEntry(Hists[syscatd], dtilde, 'l')
      if countd == 0:
        Hists[syscatd].Draw('hist')
        Hasis[syscatd].Draw('same hist')
        lg.AddEntry(Hasis[syscatd], 'Asimov - ggF_vari', 'l')
      else: Hists[syscatd].Draw('same hist')
      countd += 1
    lg.Draw('same')
    c.SaveAs(outdir+'dComp_'+sys+'_'+cat+'.png')
