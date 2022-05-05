#!/usr/bin/env ruby
require 'thread/pool'

cmds = []
[
  'sf.mc.inbending.all.root',
  'sf.mc.outbending.all.root',
  'sf.rga.inbending.all.root',
  'sf.rga.outbending.all.root',
  'sf.rgb.inbending.all.root',
  'sf.rgb.outbending.all.root',
].each do |f|
  cmds << "samplingFractionPlot.rb #{f} > tmp/#{f}.log 2>&1"
end

pool = Thread.pool(`nproc`.to_i-2)
cmds.each{|cmd|pool.process{system cmd}} # multi-threaded
# cmds.each{|cmd|system cmd} # single-threaded
pool.shutdown
