#!/usr/bin/env ruby

require './DatasetLooper.rb'
require 'thread/pool'
require 'awesome_print'
looper = DatasetLooper.new

cmds = []
looper.allsetListLoop do |dataset|
  next if dataset.include?('bibending')
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
