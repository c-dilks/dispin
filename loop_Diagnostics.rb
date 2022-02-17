#!/usr/bin/env ruby
# run DrawDiagnosticsDists.C for a list of files

require 'thread/pool'
require './DatasetLooper.rb'
looper = DatasetLooper.new

# define thread pool
poolSize = [`nproc`.to_i-2,1].max # nCPUs-2
puts "poolSize = #{poolSize}"
pool = Thread.pool(poolSize)

# run diagnostics.exe for each dataset
looper.loopSubsets do |dataset|
  args = [
    "\"outroot.#{dataset}/*.root\"",
    "plots.#{dataset}.root",
  ]
  pool.process do
    system "diagnostics.exe #{args.join ' '}"
  end
end

# execution
pool.shutdown
puts "all done!"
