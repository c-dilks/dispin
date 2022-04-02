#!/usr/bin/env ruby
# hadd inbending and outbending trees together, to make "bi-bending" trees
# - also has the ability to balance inbending/outbending yield ratios in MC, in
#   order to better match the data
# - several confimations will be asked before any execution

require './DatasetLooper.rb'
require 'fileutils'
require 'awesome_print'
require 'pry'
require 'RubyROOT'
include Root

####### SETTINGS ########
SubDir = 'catTrees'
#########################

if ARGV.length!=2
  puts """
  USAGE: $0 [DIHADRON] [SEARCH PATTERN REGEXP = (rga,rgb,mc)]"""
  DatasetLooper.printDihadrons
  puts """  PATTERN:
  - uses search pattern to identify which catTrees to hadd, then hadds them
  - it will look for trees which have not been indexed, hadd them, then 
    re-index the hadded tree
  - if pattern=mc, you will have the option to balance yields
  """
  exit 2
end
Dihadron,Pattern = ARGV

# search for source files
sources = Dir.glob("#{SubDir}/catTree*.root")
  .grep(/#{Pattern}/)
  .grep_v(/bibending/)
  .grep_v(/idx.root$/)
  .grep_v(/TRUNCATED/)
  .grep_v(/REWEIGHTED/)
puts "\nhadd sources:"
puts sources

# build target file name
targetTokens = sources.map{ |source| source.split('.') }.flatten
target = targetTokens.map{|tok|tok.gsub(/.*bending$/,'bibending')}.uniq.join('.')
puts "\nhadd target:"
puts target

# confirm?
def ask(question)
  print "\n#{question}\n> "
  $stdin.gets.chomp
end
correct = ask("\nDoes this look correct? [y/N]")=="y"
puts "you answered " + (correct ? "yes" : "no; abort hadd")
exit unless correct


# yield balancing ##################################

# get yields for each torus, given a dataset name
def getYields(dataset)
  tori = [:inbending,:outbending]
  yields = Hash.new
  catTreePrefix = DatasetLooper.catTreeBaseName("#{dataset}.#{Dihadron}")
  tori.each{ |torus| 
    catTreeFile = "#{SubDir}/#{catTreePrefix}.#{torus.to_s}.all.root"
    puts "getting yields for #{catTreeFile}"
    TFile.open(catTreeFile, "READ"){ |inFile|
      yields[torus] = inFile.Get('tree').auto_cast.GetEntries.to_f
    }
  }
  yields
end

# ask if and how we want to do balancing
useWeighting = ask("\nDo you want re-weight data to balance relative inbending to outbending yields? [y/N]")=="y"
useTruncation = false
if Pattern.include?"mc" and not useWeighting
  useTruncation = ask("\nThese are MC data. Would you rather truncate MC data for yield balancing? If you say yes, re-weighting will not be done. [y/N]")=="y"
end
useWeight=false if useTruncation


# balance yields by truncating the larger MC data set
if useTruncation

  # get inbending and outbending yields
  matchDataset  = ask("\nBalance MC to which dataset: rga, rgb, something else?")
  yieldsData    = getYields(matchDataset)
  yieldsMC      = getYields('mc')
  ioRatioData   = yieldsData[:inbending] / yieldsData[:outbending]
  ioRatioMC     = yieldsMC[:inbending]   / yieldsMC[:outbending]
  balanceFactor = ioRatioData            / ioRatioMC
  puts """
  data inbending/outbending = #{ioRatioData}
    MC inbending/outbending = #{ioRatioMC}
  balanceFactor = %.4f
  """ % [balanceFactor]

  # determine which MC tree to truncate
  if balanceFactor>1
    torusToTruncate = 'outbending'
    truncateFactor = 1.0 / balanceFactor
  else
    torusToTruncate = 'inbending'
    truncateFactor = balanceFactor
  end
  treeToTruncate = sources.find{ |source| source.include? torusToTruncate }
  puts """
  TRUNCATE #{treeToTruncate}
    to %.2f%% of the original
  """ % [truncateFactor*100]
  unless (0..1)===truncateFactor
    $stderr.puts "ERROR: truncateFactor must be in [0,1]"
    exit 1
  end

  # truncation
  puts "truncating....."
  truncatedTree = treeToTruncate.sub(/\.root$/,'.TRUNCATED.root')
  puts "truncatedTree=#{truncatedTree}"
  TFile.open(treeToTruncate,"READ"){ |inFile|
    TFile.open(truncatedTree,"RECREATE"){ |outFile|
      inTree = inFile.Get('tree').auto_cast
      entries = inTree.GetEntries
      truncatedEntries = (truncateFactor * inTree.GetEntries).round
      puts "original tree entries  = #{entries}"
      puts "truncated tree entries = #{truncatedEntries}"
      outTree = inTree.CloneTree(truncatedEntries)
      outTree.Write
    }
  }
  puts "\n - PRODUCED  #{truncatedTree}"

  # update hadd target and sources
  sources.map!{ |source| source==treeToTruncate ? truncatedTree : source }
  target.sub!('.mc.',".mc#{matchDataset.chars.last}.")
  puts '='*30+"\n\n"
  puts "\nupdated hadd sources:"
  puts sources
  puts "\nupdated hadd target:"
  puts target
  puts "\ntruncated tree is #{truncatedTree},\nwhich will be REMOVED after hadd!"
  correct = ask("\nDoes this look correct? [y/N]")=="y"
  puts "you answered " + (correct ? "yes" : "no; abort hadd")
  exit unless correct
end


# balance yields by re-weighting
if useWeighting

  # get inbending and outbending yields and calucate weights
  yields = getYields(Pattern)
  weights = {
    :inbending  => ( yields[:inbending] + yields[:outbending] ) / ( 2*yields[:inbending]  ),
    :outbending => ( yields[:inbending] + yields[:outbending] ) / ( 2*yields[:outbending] ),
  }
  print "\nyields: "
  ap yields
  print "\nfractional yields: "
  ap yields.map{ |t,y| [t,y/yields.values.sum] }.to_h
  print "weights: "
  ap weights

  # re-weight
  cmds = []
  weights.each{ |torus,weight|
    treeToReweight = sources.find{ |source| source.include? torus.to_s }
    cmds << ".x ReweightCatTree.C(\"#{treeToReweight}\",#{weight})"
  }
  print "\nreweight commands: "
  ap cmds
  correct = ask("\nDoes this look correct? [y/N]")=="y"
  exit unless correct
  cmds.each{ |cmd| gROOT.ProcessLine("#{cmd}") } # Kernel.system hangs with RubyROOT; must use gROOT instead (which locks mutex, preventing multi-threading)

  # update hadd target and sources
  sources.map!{ |source| source.sub(/\.root$/,'.REWEIGHTED.root') }
  puts '='*30+"\n\n"
  puts "\nupdated hadd sources:"
  puts sources
  puts "\nupdated hadd target:"
  puts target
  puts "\nreweighted trees are #{sources},\nwhich will be REMOVED after hadd!"
  correct = ask("\nDoes this look correct? [y/N]")=="y"
  puts "you answered " + (correct ? "yes" : "no; abort hadd")
  exit unless correct
end

# hadd
gROOT.ProcessLine ".! hadd -f #{target} #{sources.join(' ')}"

# cleanup truncated and reweighted files
puts "cleanup..."
FileUtils.rm truncatedTree if useTruncation
FileUtils.rm sources if useWeighting

# re-index
gROOT.ProcessLine ".x IndexCatTree.C(\"#{target}\")"

# strip weights
if useWeighting
  targetIdx = target.sub(/root$/,'idx.root')
  gROOT.ProcessLine ".x #{ENV["BRUFIT"]}/macros/LoadBru.C"
  gROOT.ProcessLine ".x StripTweights.C(\"#{targetIdx}\",\"Weight\",\"IO\",\"tree\")"
end
