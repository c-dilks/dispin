#!/usr/bin/env ruby
# plot sampling fraction vs. run number, to determine sampling fraction "epochs"
# - run samplingFractionTreeMaker.exe to produce the input file

require 'pry'
require 'awesome_print'
require 'pycall/import'
root = PyCall.import_module 'ROOT'
root.gROOT.SetBatch true # suppress (broken) interactive graphics
root.gStyle.SetOptStat false

## arguments
if ARGV.length<1
  puts "USAGE #{$0} [samplingFractionTreeMaker.exe output file]"
  exit 2
end
inFileN = ARGV[0]

## read input file
inFile  = root.TFile.new inFileN
tr = inFile.Get 'tree'
tr.Print

## set output file
outFileN = inFileN.sub(/\.root$/,'.plots.root')
outFile = root.TFile.new outFileN, 'RECREATE'

## set ranges
puts "determining run number range..."
runnumMin = tr.GetMinimum('runnum').to_i
runnumMax = tr.GetMaximum('runnum').to_i + 1
sampFracMin = 0.0
sampFracMax = 0.5

## fill histograms
hists = (1..6).map do |sec|
  puts "projecting sector #{sec}..."
  histN = "sec#{sec}"
  histT = [ "mean sampling fraction vs. run number", "run number", "sampling fraction", ].join(';')
  hist = root.TH2D.new( histN, histT, runnumMax-runnumMin, runnumMin, runnumMax, 50, sampFracMin, sampFracMax)
  tr.Project( histN, 'sampFrac:runnum', "sector==#{sec}")
  hist
end

## make profiles
profs = hists.map(&:ProfileX)

## draw
canvAll = root.TCanvas.new 'all', 'all'
canvAll.SetGrid 1,1
colors = [ 
  root.kBlack,
  root.kRed,
  root.kBlue,
  root.kMagenta,
  root.kGreen+1,
  root.kOrange+1
]
leg = root.TLegend.new 0.1, 0.7, 0.25, 0.9
(1..6).zip profs, colors do |sec,prof,color|
  prof.SetLineColor color
  prof.SetLineWidth 3
  prof.GetYaxis.SetRangeUser sampFracMin, sampFracMax
  prof.Draw 'same'
  leg.AddEntry prof, "sector #{sec}", 'LE'
end
leg.Draw

## write
canvAll.Write
profs.map(&:Write)

## close
outFile.Close
inFile.Close
puts "wrote #{outFileN}"
