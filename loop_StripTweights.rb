#!/usr/bin/env ruby

require './DatasetLooper.rb'
require 'awesome_print'
looper = DatasetLooper.new

cmds = []
looper.allsetListLoopOnlyBibending do |dataset|
  args = [
    DatasetLooper.catTreeBaseName(dataset)+".idx.root",
    'Weight',
    'IO',
    'tree',
  ]
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end }
  ap args
  brufit = "root -b -q $BRUFIT/macros/LoadBru.C"
  cmds << "#{brufit} 'StripTweights.C(#{args.join ','})'"
end
ap cmds

cmds.each{|cmd|system cmd} # single-threaded
