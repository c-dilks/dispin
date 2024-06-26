#!/usr/bin/env ruby
# step 2: calculate systematic uncertainty from bin migration

require 'pry'
require 'awesome_print'

### mpl
require 'matplotlib/pyplot'
plt = Matplotlib::pyplot

### numpy
require 'numpy'
np = Numpy

### additional modules
require 'pycall/import'
include PyCall::Import
pyimport 'uproot', as: :up
pyimport 'mplhep', as: :hep

#####################################
# setup

# options
SaveFigure = true

# arguments
if ARGV.length < 1
  $stderr.puts "USAGE: #{$0} [migration tree root file] [output file prefix (optional)] [iv0 name] [iv1 name] [iv2 name]"
  $stderr.puts "       (iv names are also optional, can include latex)"
  exit 2
end
inFileN = ARGV[0]
puts "reading migration tree #{inFileN}..."
outFilePrefix = inFileN.sub(/\.root$/,'')
outFilePrefix = ARGV[1] if ARGV.length > 1
ivName = 3.times.map do |i| (ARGV.length > i+2) ? ARGV[i+2] : "iv#{i}" end

# open baryonTree file, with PyRoot and Uproot
inFile = up.open inFileN
genVrec = inFile['genVrec']
ivTr = inFile['ivTr']

# get bin numbers
binNumList2D = genVrec.axes.to_a.map do |hax| hax.labels.map(&:to_i) end # read TH2 bin labels -> bin numbers
binNumList = binNumList2D.first
def toBL(binNum) binNum>>4 end     # decoders
def toBN(binNum) binNum & 0xF end  #
blList = binNumList.map do |binNum| toBL(binNum) end.uniq        # list of higher dimensional bins
nBins  = binNumList.map do |binNum| toBN(binNum) end.uniq.length # number of IV0 bins
nBinsTotal = nBins * blList.length

# get IV0 means
ivMeanH = binNumList.map do |binNum|
  binNumMask = ivTr.arrays["binNumRec"] == binNum
  [
    binNum,
    np.mean( ivTr.arrays['ivVal'][binNumMask][0..,0] )
  ]
end.to_h


#####################################
# plot generated bin vs. reconstructed bin

## convert yields in each bin to the fraction of events reconstructed in the bin from the
## given generated bin
# - x axis (0) is reconstructed bin, y axis (1) is generated bin
genVrecNP,genVrecXbins,genVrecYbins = genVrec.to_numpy
# migration fraction: divide each element of the 2D array by the sum of its column (need `keepdims` for broadcasting)
fMigNP = genVrecNP / genVrecNP.sum(axis: 1, keepdims: true)

# draw genVrec
if SaveFigure
  plt.rcParams.update({
    'font.size'    => 10,
    'savefig.bbox' => 'tight',
    'figure.dpi'   => 200,
    # 'text.usetex'         => true,
    # 'font.family'         => 'serif',
    # 'font.serif'          => ['Times'],
    # 'text.latex.preamble' => '\usepackage{amssymb}'
  })
end
genVrecFig, genVrecAxs = plt.subplots(
  figsize: [8,6],
  squeeze: false,
)
genVrecAx = genVrecAxs[0,0]
hep.hist2dplot(
  np.round(fMigNP, decimals: 3),
  genVrecXbins,
  genVrecYbins,
  ax:     genVrecAx,
  cmap:   'Reds',
  norm:   Matplotlib::colors.LogNorm.new, # logarithmic color scale
  labels: true,
)
genVrecAx.set_xlabel 'reconstructed bin'
genVrecAx.set_ylabel 'generated bin'
genVrecAx.set_title  'Fraction in reconstructed bin from generated bin'

# set human-readable tick labels
binLabelsHuman = binNumList2D.map do |hax|
  hax.map do |binNum|
    ret = "#{ivName[0]} bin #{toBN(binNum)+1}"
    ret += ",\n#{ivName[1]} bin #{toBL(binNum)+1}" if blList.length>1 # TODO: only good up to 2D
    ret
  end
end
genVrecAx.set_xticks(
  genVrecXbins+0.5, # shift tick marks to bin centers
  labels: binLabelsHuman[0]+[''], # add empty string (fenceposting)
  rotation: 90,
)
genVrecAx.set_yticks(
  genVrecYbins+0.5,
  labels: binLabelsHuman[1]+[''],
)
genVrecAx.set_xlim right: genVrecXbins[-1] # remove white space (caused by the tick mark shift)
genVrecAx.set_ylim top:   genVrecYbins[-1]


#####################################
# plot adjacent IV0 bin migration fractions

adjFig, adjAxs = plt.subplots(
  2, 2,
  figsize: [8,6],
  squeeze: false,
)
colorH  = { 0=>'Green', 1=>'Red', 2=>'Blue' }
markerH = { 0=>'o',     1=>'s',   2=>'d'    }
plotH = {
  :same  => { :ax=>adjAxs[0,0], :var=>"$f_k^{~k}$"     },
  :above => { :ax=>adjAxs[1,0], :var=>"$f_{k+1}^{~k}$" },
  :below => { :ax=>adjAxs[1,1], :var=>"$f_{k-1}^{~k}$" },
}
adjAxs[0,1].set_visible false

## get adjacent iv0 bins
fMigAdjH = Hash.new
nBinsTotal.times.each do |k|
  binNum = binNumList[k]
  bn = toBN binNum
  fMigAdjH[binNum] = Hash.new
  fMigAdjH[binNum][:above] = fMigNP[k,k+1] if bn+1<nBins
  fMigAdjH[binNum][:same]  = fMigNP[k,k]
  fMigAdjH[binNum][:below] = fMigNP[k,k-1] if bn>0
end

## build adjGrPointsH = Hash { :above => { bl => { :ivMean=>[], :fMig=>[] } }, :below => {...} }
adjGrPointsH = Hash.new
[:above,:same,:below].each do |key|
  adjGrPointsH[key] = blList.map do |bl|
    [ bl, { :ivMean=>[], :fMig=>[] } ]
  end.to_h
end
fMigAdjH.each do |binNum,adjH|
  bn = toBN binNum
  bl = toBL binNum
  [:above,:same,:below].each do |key|
    unless fMigAdjH[binNum][key].nil?
      adjGrPointsH[key][bl][:ivMean] << ivMeanH[binNum]
      adjGrPointsH[key][bl][:fMig]   << fMigAdjH[binNum][key]
    end
  end
end

#  draw adjGr plots
adjGrPointsH.each do |key,blH|
  ax = plotH[key][:ax]

  # loop through BL values
  blH.each do |bl,points|
    ax.errorbar(
      points[:ivMean],
      points[:fMig],
      color:      colorH[bl],
      ecolor:     colorH[bl],
      marker:     markerH[bl],
      linestyle:  'None',
      elinewidth: 2,
      markersize: 5,
      capsize:    2,
      zorder:     10+bl,
      label:      "#{plotH[key][:var]} in #{ivName[1]} bin #{bl+1}",
    )
  end

  ax.grid(
    true,
    'major',
    'both',
    color:     'xkcd:light grey',
    linewidth: 0.5,
    zorder:    0,
  )
  ax.set_title  "#{plotH[key][:var]} vs. mean #{ivName[0]} in bin k"
  ax.set_xlabel "mean reconstructed #{ivName[0]}" unless key==:same
  ax.set_ylabel "#{plotH[key][:var]}"
  ax.legend if blList.length>1

  # axis sharing
  ax.sharex(plotH[:same][:ax])
  ax.sharey(plotH[:above][:ax]) if key==:below
end

# re-scale vertical axis lims
plotH.each do |key,plot|
  bot,top = plot[:ax].get_ylim
  if key==:same
    plot[:ax].set_ylim bottom: 1-1.1*(1-bot), top: 1
  else
    plot[:ax].set_ylim bottom: 0,             top: 1.1*top
  end
end




######################################################################
# save figure
adjFig.tight_layout
genVrecFig.tight_layout
if SaveFigure
  adjFig.savefig("#{outFilePrefix}.adj.png")
  puts "produced #{outFilePrefix}.adj.png"
  genVrecFig.savefig("#{outFilePrefix}.genVrec.png")
  puts "produced #{outFilePrefix}.genVrec.png"
end

######################################################################
## NPZ output
np.savez("#{outFilePrefix}.npz", fMig: fMigNP)
puts "produced #{outFilePrefix}.npz"
### example access:
# npzFile = np.load("#{outFilePrefix}.npz")
# fMigNP = npzFile['fMig']

######################################################################
######################################################################
######################################################################
## calculate systematic uncertainty
## TODO: this is a sketch, assuming some asymmetry values; move this calculation
##       to a downstream script that reads the true asymmetries
asymNP = np.array [0.02,0.04,0.06,0.02,0.04,0.06]
asymSysUncH = nBinsTotal.times.map do |k|
  numer = np.sum( fMigNP[k,0..] * (asymNP[k]-asymNP) )  # sum_i{ f_i * (A_k - A_i) }
  denom = fMigNP[k,k] * asymNP[k]                       # f_k * A_k
  [
    binNumList[k],
    (denom.abs>0) ? (numer/denom).abs : 0.0
  ]
end.to_h
puts "FAKE UNCERTAINTY:"
ap asymSysUncH
$stderr.puts """
WARNING WARNING WARNING: since this systematic requires asymmetries, the full
calculation is not yet complete!

"""
######################################################################
######################################################################
######################################################################

# plt.show unless SaveFigure
unless SaveFigure
  FastInteractive = 1
  if FastInteractive # "fast": press any key to close windows
    [
      adjFig,
      genVrecFig,
    ].each do |fig|
      plt.waitforbuttonpress(0) 
      plt.close(fig)
    end
  else
    plt.show # "slow": must close windows manually
  end
end
