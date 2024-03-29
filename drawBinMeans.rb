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
pi = Math::PI

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
  :markerSize => 1.5,
  :lineSize   => 2,
}
grFormat[2][:color]=KMagenta+1 if fileList.length==3 # if 3 sets, change color of 3rd (MC)

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

# get first or last filled bin
def getFirstFilledBin(hist)
  (1..hist.GetNbinsX).each do |bn|
    return hist.GetBinCenter(bn) if hist.GetBinContent(bn)>0
  end
end
def getLastFilledBin(hist)
  (1..hist.GetNbinsX).to_a.reverse.each do |bn|
    return hist.GetBinCenter(bn) if hist.GetBinContent(bn)>0
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
        # boundHash[varN] = { :lbound=>key.GetXaxis.GetXmin, :ubound=>key.GetXaxis.GetXmax, } # use full histogram range
        boundHash[varN] = { :lbound=>getFirstFilledBin(key), :ubound=>getLastFilledBin(key), } # use only the range the data occupy
        if varN.match?(/^Phi/)
          boundHash[varN][:lbound] = -pi/2
          boundHash[varN][:ubound] = 3*pi/2
        end
      end
    end
    boundHash.each{ |k,v| puts "#{k.ljust(15)} = #{v}" } if Verbose
  end

  # add any TGraphErrors to mgrHash TMultiGraphs
  keyLoop(inFile) do |key|
    if key.ClassName=="TGraphErrors"

      # omit some graphs from being drawn
      draw = true
      [
        /^DY/,
        /^DepolA/,
        /^DepolC/,
        /^DepolW/,
        /^depol/,
        /^Helicity/,
        /^Y/,
      ].each do |rx| draw=false if key.GetName.match?(rx) end
      next unless draw

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
  mgr.SetMinimum boundHash[yvar][:lbound] # set axis scale
  mgr.SetMaximum boundHash[yvar][:ubound]
  canv = TCanvas.create(canvN,canvN,800,500)
  canv.SetGrid(1,1)
  canv.SetLeftMargin(0.20)
  canv.SetRightMargin(0.03)
  canv.SetTopMargin(0.03)
  canv.SetBottomMargin(0.15)
  mgr.Draw "APLE"
  canvList << canv
end

# output canvases ### NOTE: includes filter that excludes irrelevant plots
canvList.each do |canv|
  canvN = canv.GetName

  # filter out irrelevant plots
  next if canvN.include?"Helicity"
  next if canvN.include?"DYsgn"
  next if canvN.split('_').length > canvN.split('_').uniq.length # skip sanity checks (e.g., <x> vs. <x>)

  canv.SaveAs("meanvmean/#{canvN}.png")
end

run_app if Interactive
