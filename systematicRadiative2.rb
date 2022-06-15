#!/usr/bin/env ruby
# step 2: calculate systematic uncertainty from radiative corrections

require 'pry'
require 'awesome_print'
require 'json'

### matplotlib
require 'matplotlib/pyplot'
plt = Matplotlib::Pyplot

### numpy
require 'numpy'
np = Numpy

### additional modules
require 'pycall/import'
include PyCall::Import
pyimport 'ROOT',   as: :root
pyimport 'uproot', as: :up
#pyimport 'mplhep', as: :hep

###################################

# arguments
if ARGV.length < 2
  $stderr.puts "USAGE: #{$0} [radiative/ root file] [ivType] [output file prefix (optional)]"
  exit 2
end
inFileN = ARGV[0]
ivType = ARGV[1].to_i
puts "reading radiative tree #{inFileN}..."
outFilePrefix = inFileN.sub(/\.root$/,'')
outFilePrefix = ARGV[2] if ARGV.length > 2

# open radiative root file
inFilePy = root.TFile.new inFileN, 'READ'
inFileUp = up.open inFileN

# read trees and histograms
treeHash = Hash.new    # { binNum => tree }
ivMeanHash = Hash.new  # { binNum => mean IV value }
ivName = Array.new(2,'')
inFileUp.keys.each do |key|
  binNum = key.split(';').first.split('_').last.to_i
  if key.match?(/^tree_/)
    ### read trees with uproot, to use numpy methods
    treeHash[binNum] = inFileUp[key]
  elsif key.match?(/^iv_/)
    ### read histograms with pyroot, so we can just use ROOT to get their means
    ivDist = inFilePy.Get key
    ivMeanHash[binNum] = ivDist.GetMean 1
    ivName[0] = ivDist.GetXaxis.GetTitle
    ivName[1] = ivDist.GetYaxis.GetTitle if ivDist.GetDimension>1
  end
end

inFilePy.Close # done with pyroot


########################################################

# proc: get counts for given bin and region
counts = Proc.new do |binNum,reg|
  treeHash[binNum].arrays["count_#{reg.to_s}"][0].to_f
end


########################################################

# decode binning
# - BL = BinList number, for higher dimension bins (IV1,IV2)
# - BN = bin number, 1st dimension (IV0)
def toBL(binNum) binNum>>4 end
def toBN(binNum) binNum & 0xF end
blList = treeHash.keys.map do |binNum| toBL(binNum) end.uniq # list of higher dimensional bins
nBins  = treeHash.keys.map do |binNum| toBN(binNum) end.uniq.length # number of IV0 bins

########################################################

## calculate systematic uncertainty

# read measured asymmetries
# TODO: this is a sketch, assuming some asymmetry values; need a script to read true asymmetries
asymHash = Hash.new
asymHash[:meas] = treeHash.keys.map do |binNum| [binNum,0.02] end.to_h # make them all 2%

# asymmetries from invalid regions (determined externally, hard-coded here)
# TODO: measure these!
asymHash[:exc] = treeHash.keys.map do |binNum| [binNum,0.01] end.to_h # make them all 1%
asymHash[:mix] = treeHash.keys.map do |binNum| [binNum,0.01] end.to_h # make them all 1%
asymHash[:fir] = treeHash.keys.map do |binNum| [binNum,0.01] end.to_h # make them all 1%

# loop over trees for each bin, calculating fraction of counts for each
# return a Hash with all the necessary values for each bin
# sysHash { binNum => { ... } }
sysHash = treeHash.map do |binNum,tree|

  ### begin resulting Hash
  res = Hash.new
  res[:ivMean] = ivMeanHash[binNum]

  ### calculate fractions
  res[:nTotal] = counts.call(binNum,:ful)
  if res[:nTotal]>0
    res[:excFrac] = counts.call(binNum,:exc) / res[:nTotal]
    res[:mixFrac] = counts.call(binNum,:mix) / res[:nTotal]
    res[:firFrac] = counts.call(binNum,:fir) / res[:nTotal]
    res[:excFracErr] = 0.0 # TODO
    res[:mixFracErr] = 0.0 # TODO
    res[:firFracErr] = 0.0 # TODO
  else
    res[:excFrac] = 0.0
    res[:mixFrac] = 0.0
    res[:firFrac] = 0.0
    res[:excFracErr] = 0.0
    res[:mixFracErr] = 0.0
    res[:firFracErr] = 0.0
  end

  ### calculate "true" asymmetry
  res[:asymMeas] = asymHash[:meas][binNum]
  res[:asymTrue] = ( asymHash[:meas][binNum] - res[:excFrac]*asymHash[:exc][binNum] - res[:mixFrac]*asymHash[:mix][binNum] ) / 
    ( 1 - res[:excFrac] - res[:mixFrac] ) # use exc & mix
  # res[:asymTrue] = ( asymHash[:meas][binNum] - res[:firFrac]*asymHash[:fir][binNum] ) / 
  #   ( 1 - res[:firFrac] ) # use fir

  ### calculate systematic uncertainty
  if res[:asymMeas] != 0
    res[:sysUnc] = (res[:asymTrue]-res[:asymMeas]).abs / res[:asymMeas]
    res[:sysUncErr] = 0.0 # TODO
  else
    $stderr.puts "ERROR: asymMeas=0 will cause division by zero"
    res[:sysUnc] = 0
    res[:sysUncErr] = 0
  end

  ### decode binNum
  bl = toBL binNum
  bn = toBN binNum

  [binNum,res]
end.to_h

inFileUp.close # done with uproot

### get list of calculation keys in sysHash
calcKeys = sysHash[sysHash.keys.first].keys

######################################################

### data structure for plot points
# plotPoints { BL => { :ivMean=>iv_array, :excFrac=>excFrac_array, ... } }
plotPoints = blList.map do |bl|
  arrs = calcKeys.map do |sym|
    [ sym, Array.new(nBins,0.0) ]
  end.to_h
  [bl,arrs]
end.to_h

### fill plot points
sysHash.each do |binNum,props|
  bl = toBL binNum
  bn = toBN binNum
  calcKeys.each do |key|
    plotPoints[bl][key][bn] = props[key]
  end
end

### fractions list (the ones we care about)
plotLabels = {
  :excFrac => '$f_{\gamma}^{M_X\in(0.75,1.15)}$',
  :mixFrac => '$f_{\gamma}^{M_X\in(1.15,1.5)}$',
  :firFrac => '$f_{\gamma}^{M_X<1.5}$',
  :sysUnc  => '$\Delta_{\gamma}$',
}

### draw plots
nrows = blList.length
ncols = 2
plt.rcParams.update({
  'font.size'           => 12,
  'figure.figsize'      => [4*ncols,3*nrows],
  'figure.dpi'          => 300,
  'savefig.bbox'        => 'tight',
  'text.usetex'         => true,
  'font.family'         => 'serif',
  'font.serif'          => ['Times'],
  'text.latex.preamble' => '\usepackage{amssymb}'
})
fig, axs = plt.subplots(nrows, ncols, squeeze: false)
row = 0
plotPoints.each do |bl,points|
  
  # get axis hash for this `bl`
  axHash = Hash.new
  [ :excFrac, :mixFrac, :firFrac, ].each do |sym|
    axHash[sym] = axs[row,0]
  end
  axHash[:sysUnc] = axs[row,1]

  # proc: add a graph to an axis
  makePlot = Proc.new do |ySym,ySymErr,color,marker,fillstyle|
    axHash[ySym].errorbar(
      points[:ivMean],
      points[ySym],
      yerr:       points[ySymErr],
      marker:     marker,
      fillstyle:  fillstyle,
      color:      color,
      ecolor:     color,
      linestyle:  'None',
      elinewidth: 2,
      markersize: 3,
      capsize:    2,
      zorder:     10,
      label:      plotLabels[ySym],
    )
    axHash[ySym].grid(
      true,
      'major',
      'both',
      color:     'xkcd:light grey',
      linewidth: 0.5,
      zorder:    0,
    )
    axHash[ySym].set_title  "$#{ivName[1]}$ Bin #{bl+1}" if blList.length>1
    axHash[ySym].set_xlabel "$#{ivName[0]}$"
  end

  makePlot.call :excFrac, :excFracErr, 'k', 'o', 'full'
  makePlot.call :mixFrac, :mixFracErr, 'r', 'o', 'full'
  makePlot.call :firFrac, :firFracErr, 'b', 's', 'full'
  makePlot.call :sysUnc,  :sysUncErr,  'g', 'o', 'full'
  
  [:excFrac, :mixFrac, :firFrac].each do |sym|
    axHash[sym].set_ylim 0.0, 0.1
    axHash[sym].set_ylabel '$f_{\gamma}$'
  end
  axHash[:sysUnc].set_ylim 0.0, 0.1
  axHash[:sysUnc].set_ylabel plotLabels[:sysUnc]

  axHash[:excFrac].legend(
    bbox_to_anchor: [-0.3,1],  # puts legend to the left of the first subplot
  )

  row += 1
end
fig.tight_layout

### save output PNG and JSON
##
plt.savefig("#{outFilePrefix}.png")
puts "produced #{outFilePrefix}.png"
##
File.open("#{outFilePrefix}.json",'w') do |j| j.puts sysHash.to_json end
puts "produced #{outFilePrefix}.json"
## example: parse JSON to Hash
# File.open("#{outFilePrefix}.json") do |j| 
#   parsedHash = JSON.parse(j.read)
#   ap parsedHash
# end
