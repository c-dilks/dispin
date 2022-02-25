#!/usr/bin/env ruby
#
require 'thread/pool'
require './DatasetLooper.rb'
require 'pp'
looper = DatasetLooper.new

# define thread pool
poolSize = [`nproc`.to_i-2,1].max # nCPUs-2
puts "poolSize = #{poolSize}"
pool = Thread.pool(poolSize)

# run TripleCatTreeDists.C for each dataset
cmds = []
looper.allsetListLoopOnlyRGA do |dataset|
  
  # triple of datasets
  triple = [dataset] # RGA
  triple << looper.matchByTorus(dataset,looper.allsetListLoopOnlyRGB) # RGB
  triple << looper.matchByTorus(dataset,looper.allsetListLoopOnlyMC) # MC

  # catTrees
  catTrees = triple.map{ |set| looper.catTreeBaseName(set)+".idx.root" }

  # output directory
  outdir = "cattreetriples/" + dataset.split('.').find{|tok|tok.include?'bending'}

  # root command
  args = [
    *catTrees,
    outdir,
  ]
  puts "#{args}"
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  cmds << "root -b -q 'TripleCatTreeDists.C(#{args.join(',')})'"
end

# execution
cmds.each{|cmd|pool.process{system cmd}} # multi-threaded
# cmds.each{|cmd|system cmd} # single-threaded
pool.shutdown
puts "all done!"
