#!/usr/bin/env ruby
# step 2: calculate systematic uncertainty from baryonic decays
#
# - WARNING: this script tests using uproot and pyroot, and thus has some additional dependencies
#

require 'pry'
require 'awesome_print'
require 'json'

### import matplotlib
# - matplotlib interactivity + pyroot interactivity = segfaults galore
# - workaround: switch to non-interactive matplotlib backend 'agg'
# - see https://sft.its.cern.ch/jira/browse/ROOT-7909
require 'matplotlib'
Matplotlib.use('Agg') # workaround
require 'matplotlib/pyplot'
plt = Matplotlib::Pyplot

### import numpy
require 'numpy'
np = Numpy

### import pyroot and uproot via pycall
require 'pycall/import'
include PyCall::Import
pyimport 'ROOT',    as: :root
pyimport 'uproot',  as: :up

########################################################

# uproot notes
##### uproot example: histogram a branch
## t = file['tree_1']
## ap t.keys.to_a
## b = t.arrays
## ap b[0].tolist.to_h
## plt.hist b['parentPid'], bins: 400
## plt.show
#
##### uproot example: plot t2h with mplhep
## pyimport 'mplhep',  as: :hep
## h = file['iv_1']
## hep.hist2dplot(h.to_numpy)
## plt.show
#

########################################################

if ARGV.length < 1
  $stderr.puts "USAGE: #{$0} [baryonTrees/ root file] [output file prefix (optional)]"
  exit 2
end
inFileN = ARGV[0]
puts "reading baryon tree #{inFileN}..."
outFilePrefix = inFileN.sub(/\.root$/,'')
outFilePrefix = ARGV[1] if ARGV.length > 1

# open baryonTree file, with PyRoot and Uproot
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

# species list
speciesList = [
  :gen,
  :quark,
  :meson,
  :baryon,
  :unknown,
]

# PID lookup table
# - map PID (KF code) to a list of `properties`
pidLut = {
  -3122 => { :name=>'\bar{\Lambda}', :species=>:baryon },
  -323  => { :name=>'K^{*-}',        :species=>:meson  },
  -313  => { :name=>'K^{*0}',        :species=>:meson  },
  -213  => { :name=>'\rho^-',        :species=>:meson  },
  -1    => { :name=>'dbar',          :species=>:quark  },
  91    => { :name=>'gen=91',        :species=>:gen    },
  92    => { :name=>'gen=92',        :species=>:gen    },
  113   => { :name=>'\rho^0',        :species=>:meson  },
  213   => { :name=>'\rho^+',        :species=>:meson  },
  221   => { :name=>'\eta',          :species=>:meson  },
  223   => { :name=>'\omega',        :species=>:meson  },
  310   => { :name=>'K_S^0',         :species=>:meson  },
  313   => { :name=>'K^{*0}',        :species=>:meson  },
  323   => { :name=>'K^{*+}',        :species=>:meson  },
  331   => { :name=>'\eta\'',        :species=>:meson  },
  333   => { :name=>'\phi',          :species=>:meson  },
  1114  => { :name=>'\Delta^-',      :species=>:baryon },
  2114  => { :name=>'\Delta^0',      :species=>:baryon },
  2212  => { :name=>'p',             :species=>:baryon },
  2214  => { :name=>'\Delta^+',      :species=>:baryon },
  2224  => { :name=>'\Delta^{++}',   :species=>:baryon },
  3112  => { :name=>'\Sigma^-',      :species=>:baryon },
  3114  => { :name=>'\Sigma^{*-}',   :species=>:baryon },
  3122  => { :name=>'\Lambda',       :species=>:baryon },
  3214  => { :name=>'\Sigma^{*0}',   :species=>:baryon },
  3222  => { :name=>'\Sigma^+',      :species=>:baryon },
  3224  => { :name=>'\Sigma^{*+}',   :species=>:baryon },
  3312  => { :name=>'\Xi^-',         :species=>:baryon },
  3324  => { :name=>'\Xi^{*0}',      :species=>:baryon },
}

# get list of parent PIDs from treeHash
parentPidList = treeHash.values.map do |tree|
  np.unique( tree.arrays['parentPid'].to_numpy.flatten ).to_a
end.flatten.sort.uniq

# add missing particles to PID table
parentPidList.each do |pid|
  if pidLut[pid].nil?
    pidLut[pid] = { :name=>"PID=#{pid}", :species=>:unknown }
  end
end
unknownPids = pidLut.select do |pid,props|
  props[:species]==:unknown
end.keys

########################################################

# decode binning
# - BL = BinList number, for higher dimension bins (IV1,IV2)
# - BN = bin number, 1st dimension (IV0)
def toBL(binNum) binNum>>4 end
def toBN(binNum) binNum & 0xF end
blList = treeHash.keys.map do |binNum| toBL(binNum) end.uniq # list of higher dimensional bins
nBins  = treeHash.keys.map do |binNum| toBN(binNum) end.uniq.length # number of IV0 bins

########################################################


# loop over trees for each bin, calculating fraction of dihadrons from baryonic decays;
# return a Hash with all the necessary values for each bin
# baryonHash { binNum => { :ivMean=>iv_value, :fBaryon=>fBaryon_value, ... } }
baryonHash = treeHash.map do |binNum,tree|

  ### begin resulting Hash
  res = Hash.new
  res[:ivMean] = ivMeanHash[binNum]
  res[:nTotal] = tree.num_entries.to_f

  ### read branches
  parentIdxArr = tree.arrays['parentIdx']
  parentPidArr = tree.arrays['parentPid']
  bl = toBL binNum # decode binNum
  bn = toBN binNum

  ### count number of dihadrons where one or both parents are baryons
  # - Hash method .[] does not accept vectorized input; cannot use np.vectorize; instead, use native Array.count
  res[:nBaryon] = parentPidArr.to_numpy.to_a.count do |hadParentPids|
    hadParentPids.any?{ |pid| pidLut[pid][:species] == :baryon }
  end.to_f

  ### compute f_baryon and error
  if res[:nTotal]>0 and res[:nBaryon]>0
    res[:fBaryon] = res[:nBaryon] / res[:nTotal]
    res[:fBaryonErr] = Math.sqrt( res[:fBaryon]*(1-res[:fBaryon]) / res[:nTotal] ) # two outcomes -> bernoulli process -> binomial error propagation
  else
    res[:fBaryon]    = 0.0
    res[:fBaryonErr] = 0.0
  end

  ### compute systematic on aLU
  res[:aluSysUnc]    = res[:fBaryon]    / ( 1 - res[:fBaryon] )
  res[:aluSysUncErr] = res[:fBaryonErr] / ( 1 - res[:fBaryon] )**2

  [binNum,res]
end.to_h

inFileUp.close # done with uproot

### get list of calculation keys in baryonHash
calcKeys = baryonHash[baryonHash.keys.first].keys

######################################################

### data structure for plot points
# plotPoints { BL => { :ivMean=>iv_array, :fBaryon=>fBaryon_array, ... } }
plotPoints = blList.map do |bl|
  arrs = calcKeys.map do |sym|
    [ sym, Array.new(nBins,0.0) ]
  end.to_h
  [bl,arrs]
end.to_h

### fill plot points
baryonHash.each do |binNum,props|
  bl = toBL binNum
  bn = toBN binNum
  calcKeys.each do |key|
    plotPoints[bl][key][bn] = props[key]
  end
end

### draw plots
plotSymList = [:fBaryon,:aluSysUnc]
nrows = blList.length
ncols = plotSymList.length
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
  plotSymList.each_with_index do |sym,col|
    axHash[sym] = axs[row,col]
  end

  # proc: add a graph to an axis
  makePlot = Proc.new do |ax,ySym,ySymErr,color|
    ax.errorbar(
      points[:ivMean],
      points[ySym],
      yerr:       points[ySymErr],
      marker:     'o',
      color:      color,
      ecolor:     color,
      linestyle:  'None',
      elinewidth: 2,
      markersize: 3,
      capsize:    2,
      zorder:     10,
    )
    ax.grid(
      true,
      'major',
      'both',
      color:     'xkcd:light grey',
      linewidth: 0.5,
      zorder:    0,
    )
    ax.set_title  "$#{ivName[1]}$ Bin #{bl+1}" if blList.length>1
    ax.set_xlabel "$#{ivName[0]}$"
  end

  makePlot.call axHash[:fBaryon],   :fBaryon,   :fBaryonErr,   'b'
  makePlot.call axHash[:aluSysUnc], :aluSysUnc, :aluSysUncErr, 'r'
  
  axHash[:fBaryon].set_ylim   0.0, 0.08
  axHash[:aluSysUnc].set_ylim 0.0, 0.08
  axHash[:fBaryon].set_ylabel   '$f_B$'
  axHash[:aluSysUnc].set_ylabel '$\Delta_B$'

  row += 1
end
fig.tight_layout

### save output PNG and JSON
##
plt.savefig("#{outFilePrefix}.png")
puts "produced #{outFilePrefix}.png"
##
File.open("#{outFilePrefix}.json",'w') do |j| j.puts baryonHash.to_json end
puts "produced #{outFilePrefix}.json"
## example: parse JSON to Hash
# File.open("#{outFilePrefix}.json") do |j| 
#   parsedHash = JSON.parse(j.read)
#   ap parsedHash
# end

### print a warning if there were unknown parent PIDs
if unknownPids.length>0
  $stderr.puts "WARNING: unknown parent PIDs:"
  ap unknownPids
end

