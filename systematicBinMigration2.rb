#!/usr/bin/env ruby
# step 2: calculate systematic uncertainty from bin migration

require 'pry'
require 'awesome_print'
require 'json'

### mpl
require 'matplotlib/pyplot'
plt = Matplotlib::Pyplot

### numpy
require 'numpy'
np = Numpy

### additional modules
require 'pycall/import'
include PyCall::Import
pyimport 'uproot',  as: :up
pyimport 'mplhep',  as: :hep

#####################################

# arguments
if ARGV.length < 1
  $stderr.puts "USAGE: #{$0} [migration/ root file] [output file prefix (optional)] [iv0 name] [iv1 name] [iv2 name]"
  $stderr.puts "       (iv names are also optional)"
  exit 2
end
inFileN = ARGV[0]
puts "reading baryon tree #{inFileN}..."
outFilePrefix = inFileN.sub(/\.root$/,'')
outFilePrefix = ARGV[1] if ARGV.length > 1
ivName = 3.times.map do |i| (ARGV.length > i+2) ? ARGV[i+2] : "iv#{i}" end

# open baryonTree file, with PyRoot and Uproot
inFile = up.open inFileN
genVrec = inFile['genVrec']

# get bin numbers
binNumList = genVrec.axes.to_a.map do |hax| hax.labels.map(&:to_i) end # read TH2 bin labels -> bin numbers
def toBL(binNum) binNum>>4 end     # decoders
def toBN(binNum) binNum & 0xF end  #
blList = binNumList.first.map do |binNum| toBL(binNum) end.uniq        # list of higher dimensional bins
nBins  = binNumList.first.map do |binNum| toBN(binNum) end.uniq.length # number of IV0 bins

# create human-readable bin labels
binLabelsHuman = binNumList.map do |hax|
  hax.map do |binNum|
    ret = "#{ivName[0]} bin #{toBN binNum}"
    ret += ", #{ivName[1]} bin #{toBL binNum}" if blList.length>1 # TODO: only good up to 2D
    ret
  end
end

# draw genVrec
genVrecNP = genVrec.to_numpy
genVrecFig, genVrecAx = plt.subplots
hep.hist2dplot(
  genVrecNP,
  ax:     genVrecAx,
  labels: true,
)
genVrecAx.set_xticks genVrecNP[1]+0.5 # shift tick marks to bin centers
genVrecAx.set_yticks genVrecNP[2]+0.5
genVrecAx.set_xlim right: genVrecNP[1][-1] # remove white space (caused by the tick mark shift)
genVrecAx.set_ylim top:   genVrecNP[2][-1]
genVrecAx.set_xticklabels binLabelsHuman[0]+[''] # labels need one more empty string (fenceposting)
genVrecAx.set_yticklabels binLabelsHuman[1]+['']

# TODO: maybe make the TH2D have binning (nBins, -0.5, nBins-0.5)

plt.show
