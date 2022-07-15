#!/usr/bin/env ruby

require 'pry'

dirs = [
  'baryonTrees.22gev',
  'baryonTrees.12gev.rgcMC',
]

parents = {
  113  => 'rho',
  223  => 'omega',
  2224 => 'deltaPP',
}

inFiles = dirs.map do |dir|
  Dir.glob "#{dir}/*.root"
end.flatten

parents.keys.product(inFiles).each do |pid,inFile|
  system [
    './systematicBaryonDecay2.rb',
    inFile,
    inFile.sub(/root$/,parents[pid]),
    pid
  ].join(' ')
end
