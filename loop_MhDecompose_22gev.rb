#!/usr/bin/env ruby
# run MhDecompose.exe for a list of files

require 'thread/pool'
require 'awesome_print'

# define thread pool
poolSize = [`nproc`.to_i-2,1].max # nCPUs-2
puts "poolSize = #{poolSize}"
pool = Thread.pool(poolSize)

# datasets: short name => outroot dir name
datasetHash = {
  '22gev'         => '22gev.proton.all',
  '12gev.rgcMC'   => 'rgc.proton.all',
  # '12gev.rgaData' => 'rga.inbending.sp19.subset',
  # '12gev.rgaMC'   => 'rga.mc',
}

# run diagnostics.exe for each dataset
cmds = []
datasetHash.each do |name,outrootSuffix|
  cmds << [
    'MhDecompose.exe',
    "\"outroot.#{outrootSuffix}/*.root\"",
    "plots.#{name}.root",
    '0x34',
  ].join(' ')
end

# execution
ap cmds
cmds.each{|cmd|pool.process{system cmd}}
pool.shutdown
