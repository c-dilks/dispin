#!/usr/bin/env ruby
#
require 'thread/pool'
require './DatasetLooper.rb'
require 'awesome_print'
looper = DatasetLooper.new

# define thread pool
poolSize = [`nproc`.to_i-2,1].max # nCPUs-2
puts "poolSize = #{poolSize}"
pool = Thread.pool(poolSize)

# run StackCatTreeDists.C for each dataset
cmds = []
looper.allsetListLoopOnlyRGA do |rgaSet|
  
  # stack of datasets
  rgbSet = looper.matchByTorus(rgaSet,looper.allsetListLoopOnlyRGB)
  stack = [rgaSet,rgbSet] # RGA and RGB
  stack << looper.matchByTorus(rgaSet,looper.allsetListLoopOnlyMC) # MC for RGA
  stack << looper.matchByTorus(rgbSet,looper.allsetListLoopOnlyMC) # MC for RGB
  stack.uniq!

  # catTrees
  catTrees = stack.map{ |set| DatasetLooper.catTreeBaseName(set)+".idx.root" }
  (4-stack.length).times{ catTrees << '' }

  # output directory
  outdir = "cattreestacks/" + rgaSet.split('.').find{|tok|tok.include?'bending'}

  # root command
  args = [
    *catTrees,
    outdir,
    'Weight',
  ]
  ap args
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  cmds << "root -b -q 'StackCatTreeDists.C(#{args.join(',')})'"
end

# execution
cmds.each{|cmd|pool.process{system cmd}} # multi-threaded
# cmds.each{|cmd|system cmd} # single-threaded
pool.shutdown
puts "all done!"
