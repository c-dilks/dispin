#!/usr/bin/env ruby

require './DatasetLooper.rb'
require 'thread/pool'
looper = DatasetLooper.new

cmds = []
looper.allsetListLoop do |dataset|
  DatasetLooper::BinHash.each do |ivType,binOpts|
    args = [
      '-c',
      "-f #{DatasetLooper.catTreeBaseName(dataset)}.idx.root",
      "-i #{ivType}",
      "-n #{binOpts[:bins].join ' '}",
    ]
    cmds << "TabulateBinMeans.exe #{args.join ' '}"
  end
end
puts cmds

pool = Thread.pool(`nproc`.to_i-2)
cmds.each{|cmd|pool.process{system cmd}} # multi-threaded
# cmds.each{|cmd|system cmd} # single-threaded
pool.shutdown
