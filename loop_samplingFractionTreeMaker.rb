#!/usr/bin/env ruby
#
# IMPORTANT: this will run one FULL dataset per thread, which can take a while
# and will load large TChains; RGB inbending is too large of a chain to run
# correctly, and will be SKIPPED by this script; use instead
# `condor.samplingFractionTreeMaker.sh`, then hadd the resulting root files

require './DatasetLooper.rb'
require 'thread/pool'
require 'awesome_print'
looper = DatasetLooper.new

cmds = []
looper.allsetListLoop do |dataset|
  next if dataset.include?('bibending')
  next if dataset.include?('rgb.inbending')
  args = [
    'samplingFractionTreeMaker.exe',
    "-d outroot.#{dataset}",
    "-o sf.#{dataset}.root",
  ]
  ap args
  cmds << args.join(' ')
end

pool = Thread.pool(`nproc`.to_i-2)
cmds.each{|cmd|pool.process{system cmd}} # multi-threaded
# cmds.each{|cmd|system cmd} # single-threaded
pool.shutdown
