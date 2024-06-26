#!/usr/bin/env ruby
# calculate systematic uncertainty from polarization
# - uses catTree branches `Pol` and `PolErr` to obtain the yield-weighted averages
# - specify multiple catTrees to get the uncertainty for each

require 'awesome_print'
require 'pry'
require './DatasetLooper.rb'
require 'RubyROOT'
include Root

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dihadronSym = ARGV[0].to_sym
dl = DatasetLooper.new(dihadronSym)

#################################################
DatasetList = dl.allsetListLoopOnlyData
Nbins       = 2000
PolMax      = 1
#################################################

# loop over catTrees
outHash = Hash.new
calcHash = Hash.new
DatasetList.each do |dataset|

  # check existence
  catTreeFileN = DatasetLooper.catTreeBaseName(dataset)+".idx.root"
  puts "READ #{catTreeFileN}"
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

    # set output hashes
    outHash[dataset] = h[:asymmetrySystematicUncertainty]
    calcHash[dataset] = h

  end
end

# print final systematic
puts "\n\npolarization scale systematic uncertainty for each dataset:".upcase
ap outHash

# sysunc file
basename   = File.basename($0,".rb")
sysuncName = "sysunc/polarization/polarization.dat"
sysuncOut  = File.open(sysuncName,'w')
DatasetList.each do |dataset|
  next if outHash[dataset].nil?
  sysuncOut.puts "#{dataset} #{outHash[dataset]}"
end
sysuncOut.close
puts "#{sysuncName} written."

### latex table
texName = "sysunc/polarization/polarization.tex"
tex = File.open(texName,'w')
tex.puts "%%%%% generated by #{$0} on #{Time.now} %%%%%"
tex.puts '''
\begin{table}[h]
\centering
\begin{tabular}{|c|c|c|}
\hline
\textbf{Data Set} & \textbf{Mean Polarization} & \textbf{$\alu$ Systematic Uncertainty} \\\\\hline\hline
'''
def percent(val) "#{(100*val).round(2)}\\%" end
DatasetList.each do |dataset|
  next if outHash[dataset].nil?
  polarizationTex = "$" + [
    percent(calcHash[dataset][:meanPolarizationValue]),
    percent(calcHash[dataset][:meanPolarizationError]),
    percent(calcHash[dataset][:polCalibrationSystematic]),
  ].join(' \pm ') + "$"
  tex.puts [
    DatasetLooper.datasetTitle(dataset).sub(/ data set/,''),
    polarizationTex,
    "$#{percent(outHash[dataset])}$",
  ].join(' & ') + '\\\\\hline'
end
tex.puts """
\\end{tabular}
\\caption{Scale systematic uncertainties from polarization, for each data set. The mean polarization is given with statistical and systematic uncertainties.}
\\label{tab:#{basename}}
\\end{table}
"""
tex.close
puts "#{texName} written."
