#!/usr/bin/env ruby
# draw bin mean plots, produced from `TabulateBinMeans.cpp`

require 'RubyROOT'
include Root

### options ###
Verbose = false
FontSize = 0.06
Interactive = false # if true, draw, otherwise just run in background
###############

include RootApp if Interactive

# args
if ARGV.length<1
  puts """
  USAGE: #{$0} [FILES]...
  - the list of files you specify will be plotted together, stacked
  """
  exit 1
end
fileList = ARGV

# graph formatting
grFormat = {
  0 => { :color=>KRed-7,  :marker=>KFullCircle, :line=>KSolid  }, # RGA data
  1 => { :color=>KBlue+3, :marker=>KFullSquare, :line=>KSolid  }, # RGB data
  2 => { :color=>KRed-7,  :marker=>KOpenCircle, :line=>KDashed }, # RGA MC
  3 => { :color=>KBlue+3, :marker=>KOpenSquare, :line=>KDashed }, # RGB MC
  # additional settings
  :markerSize => 2,
  :lineSize   => 2,
}
gStyle.SetTitleSize(FontSize,"XYZ")
gStyle.SetLabelSize(FontSize,"XYZ")


# vars
fileNum = 0
mgrHash = {} # = { varName => TMultiGraph }
boundHash = {} # = { varName => { :lbound=> lowerBound, :ubound=>upperBound } }

# loopers
def keyLoop(rootFile) # loop through TKeys of a file, with auto-casting
  rootFile.GetListOfKeys.map(&:auto_cast).map(&:ReadObj).map(&:auto_cast).each do |key|
    yield key if block_given?
    # break # truncate prematurely for testing
  end
end

# loop over input files
fileList.map{ |inFileN| TFile.open(inFileN,"READ") }.each do |inFile|

  # list keys
  if Verbose
    puts "opened #{inFile.GetName}"
    keyLoop(inFile) { |key| puts "key=#{key.GetName.ljust(30)} class=#{key.ClassName}" }
  end

  # get bounds for each variable, by reading `tot_*` TH1Ds
  if fileNum==0
    keyLoop(inFile) do |key|
      if key.ClassName=="TH1D" and key.GetName.match?(/^tot_/)
        varN = key.GetName.sub(/^tot_/,"").sub(/Bin.*$/,"")
        boundHash[varN] = { :lbound=>key.GetXaxis.GetXmin, :ubound=>key.GetXaxis.GetXmax, }
      end
    end
    boundHash.each{ |k,v| puts "#{k.ljust(15)} = #{v}" } if Verbose
  end

  # add any TGraphErrors to mgrHash TMultiGraphs
  keyLoop(inFile) do |key|
    if key.ClassName=="TGraphErrors"
      key.SetMarkerColor grFormat[fileNum][:color]
      key.SetLineColor   grFormat[fileNum][:color]
      key.SetMarkerStyle grFormat[fileNum][:marker]
      key.SetLineStyle   grFormat[fileNum][:line]
      key.SetMarkerSize  grFormat[:markerSize]
      key.SetLineWidth   grFormat[:lineSize]
      key.SetLineWidth   grFormat[:lineSize]
      if fileNum==0
        mgr = TMultiGraph.new
        mgr.SetName key.GetName+"_mgr"
        mgr.SetTitle [
          '',#key.GetTitle,
          key.GetXaxis.GetTitle,
          key.GetYaxis.GetTitle
        ].join(';')
        mgrHash[key.GetName] = mgr
      end
      mgrHash[key.GetName].Add key
    end
  end

  fileNum += 1
end

# draw graphs
canvList = []
cgrList = []
mgrHash.each do |mgrN,mgr|
  canvN = "canv_#{mgrN}"
  yvar = mgrN.split('_').first
  # mgr.SetMinimum boundHash[yvar][:lbound] # set axis scale
  # mgr.SetMaximum boundHash[yvar][:ubound]
  canv = TCanvas.create(canvN,canvN,800,500)
  canv.SetLeftMargin(0.20)
  canv.SetRightMargin(0.03)
  canv.SetTopMargin(0.03)
  canv.SetBottomMargin(0.15)
  mgr.Draw "APLE"
  canvList << canv
end

canvList.each{ |canv| canv.SaveAs("meanvmean/#{canv.GetName}.png") }

run_app if Interactive
