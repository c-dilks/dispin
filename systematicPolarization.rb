#!/usr/bin/env ruby
# calculate systematic uncertainty from polarization
# - uses catTree branches `Pol` and `PolErr` to obtain the yield-weighted averages
# - specify multiple catTrees to get the uncertainty for each

require 'awesome_print'
require 'pry'
require './DatasetLooper.rb'
require 'RubyROOT'
include Root
dl = DatasetLooper.new

#################################################
DatasetList = dl.allsetListLoopOnlyData
Nbins       = 2000
PolMax      = 1
#################################################

# loop over catTrees
outputSysUnc = Hash.new
DatasetList.each do |dataset|

  # check existence
  catTreeFileN = DatasetLooper.catTreeBaseName(dataset)+".idx.root"
  if not File.exist? catTreeFileN
    $stderr.puts "ERROR: #{catTreeFileN} not found"
    next
  end

  # open tree file
  TFile.open(catTreeFileN,'READ') do |catTreeFile|

    # open tree, check for overflows
    catTree = catTreeFile.Get('tree').auto_cast
    histPrefix = catTreeFileN
      .gsub(/\.root/,'')
      .gsub(/\.idx/,'')
      .gsub(/\./,'_')
      .gsub(/\//,'_')
    unless catTree.GetMinimum("Pol").abs<PolMax and catTree.GetMaximum("Pol").abs<PolMax and
        catTree.GetMinimum("PolErr").abs<PolMax and catTree.GetMaximum("PolErr").abs<PolMax
      $stderr.puts "ERROR: overflow"
    end

    # fill histograms (weighted by inbending/outbending weight, if applicable)
    polValHist = TH1D.create(histPrefix+'_polVal',histPrefix+'_polVal',Nbins,-PolMax,PolMax)
    polErrHist = TH1D.create(histPrefix+'_polErr',histPrefix+'_polErr',Nbins,-PolMax,PolMax)
    catTree.Project(polValHist.GetName,"Pol","Weight")
    catTree.Project(polErrHist.GetName,"PolErr","Weight")

    # calculations; using a Hash to simplify printing
    h = Hash.new
    h[:treeFile]                       = catTreeFileN
    h[:meanPolarizationValue]          = polValHist.GetMean
    h[:meanPolarizationError]          = polErrHist.GetMean
    h[:polCalibrationSystematic]       = 0.02 # polarization systematic uncertainty from target polarization (analyzing power) and detector geometry
    h[:totalPolarizationError]         = Math.hypot(h[:meanPolarizationError],h[:polCalibrationSystematic]) # add uncertainties in quadrature
    h[:asymmetrySystematicUncertainty] = h[:totalPolarizationError] / h[:meanPolarizationValue] # deltaALU / ALU = deltaP / P
    ap h

    # set final uncertainty
    outputSysUnc[dataset] = h[:asymmetrySystematicUncertainty]

  end
end

puts "\n\npolarization scale systematic uncertainty for each dataset:".upcase
ap outputSysUnc
