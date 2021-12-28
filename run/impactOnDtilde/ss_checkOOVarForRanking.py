#!/usr/bin/python
# -*- coding: UTF-8 -*-

import sys

from ROOT import *

gROOT.SetBatch()

#csvpath = "../../../../syst/yield_sys/ext_csv/"
csvpath = "/publicfs/atlas/atlasnew/higgs/hgg/chenhr/vbfcp/syst/yield/ext_csv/"
csvjer = "/publicfs/atlas/atlasnew/higgs/hgg/chenhr/vbfcp/syst/yield/ext_csv_jd/"
sspath = "../../../../syst/shape_sys/bkg_SS.csv"
outdir = "plotForImpactCheck/"

dtildes = ['m00', 'p001', 'p002', 'p003']
#cats = ['TT', 'TL', 'LT', 'LL']
cats = ['TT']
systs = ['TT_b1', 'TT_b6']

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

def getSS(fpath, cat):
  for line in readlines(fpath):
    if cat not in line: continue
    return float(line.split(',')[2])

syslist = []
with open(csvpath+'mag_yield_346214_m00_LT_b5.csv', 'r') as f:
  for line in f.readlines():
    syslist.append(line.split(',')[0])

Hists = {}
Hasis = {}
yfile = csvpath+"N_yield.csv"
#for sys in syslist:
for sys in systs:
  for cat in cats:
    c = TCanvas('c', '', 800, 600)
    lg = TLegend(0.5, 0.5, 0.83, 0.7)
    lg.SetBorderSize(0)
    lg.SetFillColorAlpha(kBlue, 0)
    countd = 0
    #for dtilde in ['m00', 'm01', 'p01', 'm10', 'p10']:
    for dtilde in dtildes:
      syscatd = sys+'_'+cat+'_'+dtilde
      hist = TH1F(sys+'_'+dtilde+'_'+cat, '', 6, 0, 6)
      hasi = TH1F('asimov_VBF_'+sys+'_'+dtilde+'_'+cat, '', 6, 0, 6)
      # real SS variation hist, not ggF variation hist
      hggF = TH1F('SS_variation_'+sys+'_'+dtilde+'_'+cat, '', 6, 0, 6)
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
        #magnitude = getNPMag(csvfile, sys)
        #magggF = getNPMag(csvggF, sys)
        yvbf = getYield(yfile, catbin)
        yasi = getYield(yfile, catm00)
        #hist.GetXaxis().SetBinLabel(bnum, bins[bnum])
        #hist.SetBinContent(bnum, yvbf*magnitude)
        hist.SetBinContent(bnum, yvbf)
        hasi.SetBinContent(bnum, yasi)
        if bname in sys:
          ss = getSS(sspath, cat+'_'+bname)
          hggF.SetBinContent(bnum, ss)
          print 'sys catbin yasi yvbf ss'
          print sys, catbin, yasi, yvbf, ss
        else:
          hggF.SetBinContent(bnum, 0)
          print 'sys catbin yasi yvbf ss'
          print sys, catbin, yasi, yvbf, 'Non'
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
      hist.SetMaximum(1.15)
      hist.SetMinimum(0.85)
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
        lg.AddEntry(Hasis[syscatd], 'Asimov - SS_vari', 'l')
      else: Hists[syscatd].Draw('same hist')
      countd += 1
    lg.Draw('same')
    c.SaveAs(outdir+'dComp_'+sys+'_'+cat+'.png')
