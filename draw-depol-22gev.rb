#!/usr/bin/env ruby
## plot depolarization profile plots on the same canvas;
## run on output from diagnostics.exe

require 'pry'
require 'awesome_print'
require 'pycall/import'
r = PyCall.import_module 'ROOT'
r.gROOT.SetBatch true
r.gStyle.SetOptStat 0

## list of plots.root files
datasets = {
  '22gev'         => { :color=>r.kRed-7 },
  '12gev.rgcMC'   => { :color=>r.kBlue+3 },
  # '12gev.rgaData' => { :color=>r.kBlue+3 },
  # '12gev.rgaMC'   => { :color=>r.kBlue+3 },
}

## open plots.root files
datasets.each do |datasetN,dataset|
  dataset[:file] = r.TFile.new "plots.#{datasetN}.root"
end

## list of plots
depolList = [
  'A',
  'BA',
  'CA',
  'VA',
  'WA',
  'Epsilon',
]
kinList = [
  'Q2',
]
plotList = depolList
  .product(kinList)
  .map do |depol,kin| "kf#{depol}vs#{kin}" end
ap plotList

## read plots to hash: { plotName => { dataset => plot } }
plotHash = plotList.map do |plotN|
  plots = datasets.map do |datasetN,dataset|
    plot = dataset[:file].FindObjectAny plotN
    [datasetN,plot]
  end.to_h
  [plotN,plots]
end.to_h
ap plotHash

## draw plots
plotHash.each do |plotN,plots|
  canvN = "#{plotN}_canv"
  canv = r.TCanvas.new canvN, canvN, 800, 600
  canv.SetLogx
  canv.SetGrid 1,1
  plots.each do |datasetN,plot|
    dataset = datasets[datasetN]
    prof = plot.ProfileX [plotN,datasetN].join '_'
    title = prof.GetTitle.gsub(/\(y\)/,'')
    prof.SetTitle title
    prof.SetLineColor dataset[:color]
    prof.SetLineWidth 3
    prof.GetXaxis.SetLabelSize 0.05
    prof.GetYaxis.SetLabelSize 0.05
    prof.GetXaxis.SetTitle "Q^{2} [GeV]"
    if plotN.match?(/WA|CA/)
      prof.GetYaxis.SetRangeUser 0, 1.2
    elsif plotN.match?(/Epsilon/)
      prof.GetYaxis.SetRangeUser 0, 1.5
    else
      prof.GetYaxis.SetRangeUser 0, 2.5
    end
    prof.Draw 'same'
  end
  canv.Print "#{canvN}.png"
end
