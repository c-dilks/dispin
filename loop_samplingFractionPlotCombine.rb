#!/usr/bin/env ruby

require 'pry'
require 'awesome_print'
require 'pycall/import'
root = PyCall.import_module 'ROOT'
root.gROOT.SetBatch true
root.gStyle.SetOptStat false

## epochRange: epochLineLB <= runnum < epochLineUB
Epochs = [
  5420,
  5452,
  5510,
  5585,
  5700,
  6275,
  6400,
  6499,
  6600,
  6735,
  6758,
  6800,
  11200,
  11260,
  11300,
  11455,
]

# open files
inFiles = [
  'sf.rga.inbending.all.plots.root',
  'sf.rga.outbending.all.plots.root',
  'sf.rgb.inbending.all.plots.root',
  'sf.rgb.outbending.all.plots.root',
].map do |inFileN| root.TFile.new inFileN end
outFileN = 'SFplots.root'
outFile = root.TFile.new outFileN, 'recreate'

# get histograms: `histH` = inFiles mapped to list of hashes { sector=>histogram }
histH = inFiles.map do |inFile|
  (1..6).map do |sector|
    [
      sector,
      inFile.Get("sec#{sector}_pfx")
    ]
  end.to_h
end

# get full runnum range
runnumMin = 30000
runnumMax = 0
histH.each do |secH|
  hist = secH[1]
  runnumMin = [ runnumMin, hist.GetXaxis.GetXmin ].min.to_i
  runnumMax = [ runnumMax, hist.GetXaxis.GetXmax ].max.to_i
end

# create 'combined' histogram for each sector: combH = { sector=>comb }
combH = (1..6).map do |sector|
  comb = root.TH1D.new(
    "sec#{sector}",
    [ "mean sampling fraction vs. run number", "run number", "sampling fraction", ].join(';'),
    runnumMax-runnumMin, runnumMin, runnumMax
  )
  [sector,comb]
end.to_h

# fill combined histograms
sampFracMin = 1000.0
sampFracMax = 0.0
histH.each do |secH|
  secH.each do |sector,hist|
    comb = combH[sector]
    (1..hist.GetNbinsX).each do |bnHist|
      if hist.GetBinContent(bnHist)>0
        bnComb = comb.FindBin(hist.GetBinCenter bnHist)
        sf    = hist.GetBinContent bnHist
        sfErr = hist.GetBinError   bnHist
        comb.SetBinContent bnComb, sf
        comb.SetBinError   bnComb, sfErr
        sampFracMin = [ sf, sampFracMin ].min
        sampFracMax = [ sf, sampFracMax ].max
      end
    end
  end
end

## draw
sampFracRange = sampFracMax - sampFracMin
sampFracMin -= 0.1*sampFracRange
sampFracMax += 0.1*sampFracRange
canvAll = root.TCanvas.new 'canv_all', 'canv_all'
canvAll.SetGrid 1,1
canvSecH = (1..6).map do |sector|
  [
    sector,
    root.TCanvas.new("canv_sec#{sector}","canv_sec#{sector}")
  ]
end.to_h
canvSecH.values.each(&:SetGrid)
colors = { 
  1=>root.kBlack,
  2=>root.kRed,
  3=>root.kBlue,
  4=>root.kMagenta,
  5=>root.kGreen+1,
  6=>root.kOrange+1,
}
leg = root.TLegend.new 0.1, 0.7, 0.25, 0.9
combH.each do |sector,comb|
  comb.SetLineColor colors[sector]
  comb.SetLineWidth 3
  comb.GetYaxis.SetRangeUser sampFracMin, sampFracMax
  canvAll.cd
  comb.Draw 'e same'
  leg.AddEntry comb, "sector #{sector}", 'LE'
  canvSecH[sector].cd
  comb.Draw 'e'
end

## epoch lines
epochLines = Epochs.map do |epoch|
  epochL = root.TLine.new epoch, sampFracMin, epoch, sampFracMax
  epochL.SetLineWidth 2
  epochL.SetLineColor root.kOrange-7
  epochL
end
canvAll.cd
epochLines.each(&:Draw)
leg.Draw
canvSecH.values.each do |canv|
  canv.cd
  epochLines.each(&:Draw)
end

## write
canvAll.Write
canvSecH.values.each(&:Write)
combH.values.each(&:Write)

## close
outFile.Close
inFiles.each(&:Close)
puts "wrote #{outFileN}"
