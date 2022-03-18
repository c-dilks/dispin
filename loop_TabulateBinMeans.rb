#!/usr/bin/env ruby

require './DatasetLooper.rb'
require 'thread/pool'
require 'awesome_print'
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
    ap args
    cmds << "TabulateBinMeans.exe #{args.join ' '}"
  end
end

pool = Thread.pool(`nproc`.to_i-2)
cmds.each{|cmd|pool.process{system cmd}} # multi-threaded
# cmds.each{|cmd|system cmd} # single-threaded
pool.shutdown
