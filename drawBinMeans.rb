#!/usr/bin/env ruby
# draw bin mean plots, produced from `TabulateBinMeans.cpp`

require 'RubyROOT'
include Root
include RootApp

# args
if ARGV.length<1
  puts """
  USAGE: #{$0} [FILES]...
  - the list of files you specify will be plotted together, stacked
  """
  exit 1
end
fileList = ARGV

# vars
firstFile = true
mgrHash = {} # = { varName => TMultiGraph }
boundHash = {} # = { varName => { :lb=> lowerBound, :ub=>upperBound } }

# loopers
def keyLoop(rootFile) # loop through TKeys of a file, with auto-casting
  rootFile.GetListOfKeys.map(&:auto_cast).map(&:ReadObj).map(&:auto_cast).each do |key|
    yield key if block_given?
    break # truncate prematurely for testing
  end
end

# loop over input files
fileList.map{ |inFileN| TFile.open(inFileN,"READ") }.each do |inFile|
  puts "opened #{inFile.GetName}"

  # list keys
  keyLoop(inFile) { |key| puts "key=#{key.GetName.ljust(30)} class=#{key.ClassName}" }

  # get bounds for each variable, by reading `tot_*` TH1Ds
  if firstFile
    keyLoop(inFile) do |key|
      if key.ClassName=="TH1D" and key.GetName.match?(/^tot_/)
        varN = key.GetName.sub(/^tot_/,"").sub(/Bin.*$/,"")
        boundHash[varN] = { :lb=>key.GetXaxis.GetXmin, :ub=>key.GetXaxis.GetXmax, }
      end
    end
    boundHash.each{ |k,v| puts "#{k.ljust(15)} = #{v}" }
  end

  # add any TGraphErrors to mgrHash TMultiGraphs
  keyLoop(inFile) do |key|
    if key.ClassName=="TGraphErrors"
      key.SetMarkerStyle KFullCircle
      mgrHash[key.GetName] = TMultiGraph.new if firstFile
      mgrHash[key.GetName].Add key
    end
  end

  firstFile = false
end


# draw graphs
canvList = mgrHash.map do |mgrN,mgr|
  canvN = "canv_#{mgrN}"
  canv = TCanvas.create(canvN,canvN,1000,800)
  mgr.Draw "APE"
  canv
end

run_app
