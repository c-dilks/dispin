#!/usr/bin/env ruby

require './DatasetLooper.rb'
require 'thread/pool'
require 'awesome_print'

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dihadronSym = ARGV[0].to_sym
looper = DatasetLooper.new(dihadronSym)

cmds = []
looper.allsetListLoop do |dataset|
  looper.binHash.each do |ivType,binOpts|
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
