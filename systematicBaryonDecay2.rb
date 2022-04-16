#!/usr/bin/env ruby
# step 2: calculate systematic uncertainty from baryonic decays

require 'pry'
require 'awesome_print'
require 'matplotlib/pyplot'
plt = Matplotlib::Pyplot
require 'numpy'
np = Numpy
require 'pycall/import'
include PyCall::Import
pyimport 'uproot',  as: :up
pyimport 'awkward', as: :ak
pyimport 'mplhep',  as: :hep


#### example: histogram a branch
# t = file['tree_1']
# ap t.keys.to_a
# b = t.arrays
# ap b[0].tolist.to_h
# plt.hist b['parentPid'], bins: 400
# plt.show

#### example: plot t2h with mplhep
# h = file['iv_1']
# hep.hist2dplot(h.to_numpy)
# plt.show

########################################################

# open baryonTree file
file = up.open 'baryonTrees/tree.DIS_pass1_1003_1008.hipo.root'

# read keys into hashes : binNum => object
treeHash = Hash.new
ivDistHash = Hash.new
dimensions = 0
file.keys.each do |key|
  binNum = key.split(';').first.split('_').last.to_i
  if key.match?(/^tree_/)
    treeHash[binNum] = file[key]
  elsif key.match?(/^iv_/)
    ivDistHash[binNum] = file[key]
    dimensions = file[key].counts.ndim
  end
end


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
  -323 => { :name=>'K^{*-}',      :species=>:meson  },
  -313 => { :name=>'K^{*0}',      :species=>:meson  },
  -213 => { :name=>'\rho^-',      :species=>:meson  },
  -1   => { :name=>'dbar',        :species=>:quark  },
  91   => { :name=>'gen=91',      :species=>:gen    },
  92   => { :name=>'gen=92',      :species=>:gen    },
  113  => { :name=>'\rho^0',      :species=>:meson  },
  213  => { :name=>'\rho^+',      :species=>:meson  },
  221  => { :name=>'\eta',        :species=>:meson  },
  223  => { :name=>'\omega',      :species=>:meson  },
  310  => { :name=>'K_S^0',       :species=>:meson  },
  313  => { :name=>'K^{*0}',      :species=>:meson  },
  323  => { :name=>'K^{*+}',      :species=>:meson  },
  331  => { :name=>'\eta\'',      :species=>:meson  },
  333  => { :name=>'\phi',        :species=>:meson  },
  1114 => { :name=>'\Delta^-',    :species=>:baryon },
  2114 => { :name=>'\Delta^0',    :species=>:baryon },
  2212 => { :name=>'p',           :species=>:baryon },
  2214 => { :name=>'\Delta^+',    :species=>:baryon },
  2224 => { :name=>'\Delta^{++}', :species=>:baryon },
  3112 => { :name=>'\Sigma^-',    :species=>:baryon },
  3114 => { :name=>'\Sigma^{*-}', :species=>:baryon },
  3122 => { :name=>'\Lambda',     :species=>:baryon },
  3222 => { :name=>'\Sigma^+',    :species=>:baryon },
  3224 => { :name=>'\Sigma^{*+}', :species=>:baryon },
}

# PID Lut PROCs
# filter PID Lut: return list of PIDs that pass `propFilter`
# - `propFilter` should be a block with argument `properties` that returns a boolean
filterPidLut = Proc.new do |&propFilter|
  pidLut.filter do |pid,props|
    propFilter.call(props)
  end
end
# filter by species
filterPidLutBySpecies = Proc.new do |species|
  filterPidLut.call{ |p| p[:species]==species }
end
# print list of baryons
puts "list of baryons:"
ap filterPidLutBySpecies.call(:baryon)
  .map{ |pid,props| [ pid, props[:name] ] }
  .to_h

#####

# get list of parent PIDs from treeHash
parentPidList = treeHash.values.map do |tree|
  np.unique( tree.arrays['parentPid'].to_numpy.flatten ).to_a
end.flatten.sort.uniq
puts "parent PIDs list:"
ap parentPidList

# add missing particles to PID table
parentPidList.each do |pid|
  if pidLut[pid].nil?
    pidLut[pid] = { :name=>"PID=#{pid}", :species=>:unknown }
  end
end
puts "unknown parent PIDs:"
ap filterPidLutBySpecies.call(:unknown).keys


########################################################

# figure out how many higher dimensional bins there are (BL=BinList number)
def toBL(binNum) binNum>>4 end
def toBN(binNum) binNum & 0xF end
binNumList = treeHash.keys
blList = binNumList.map do |binNum| toBL(binNum) end.uniq # list of higher dimensional bins

# figure out how many IV0 bins there are
nBins = binNumList.map do |binNum| toBN(binNum) end.uniq.length

# build yieldHash:
#    BL (binNum>>4) => species => PID => count vs. bin mean
yieldHash = Hash.new
blList.each do |bl| yieldHash[bl] = Hash.new end
blList.product(speciesList).each do |bl,species|
  yieldHash[bl][species] = Hash.new
  filterPidLutBySpecies.call(species).keys.each do |pid|
    yieldHash[bl][species][pid] = Hash.new
    yieldHash[bl][species][pid][:yield] = Array.new(nBins,0)
  end
end

# TODO: create mask for cases where hadrons come from the same parent Idx
arr = treeHash[0].arrays['parentIdx'].to_numpy
sameParentMask = arr[0..,0] == arr[0..,1]


binding.pry
file.close
