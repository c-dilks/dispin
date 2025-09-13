#!/usr/bin/env ruby
# give a list of epochs, choose a few runs per epoch which we will use
# to determine the sampling fraction cuts

require 'pry'
require 'awesome_print'
require 'numpy'

### list of epochs: must match those in loop_SamplingFractionPlotCombine.rb
epochs = [
  5420,
  5452,
  5510,
  5585,
  5700,
  6275,
  6400,
  6499,
  6600,
  6735,
  6758,
  6800,
  11200,
  11260,
  11300,
  11455,
].sort

epochs.prepend 0
epochs.append 1000000

### read QA information into hash { run => num_gold_files }
goldFileRuns = File.readlines('deps/clasqaDB/text/listOfGoldFiles.txt').map(&:chomp).map do |line|
  toks = line.split.first.to_i
end
qa = goldFileRuns.uniq.map do |run|
  [
    run,
    goldFileRuns.count(run)
  ]
end.to_h

### make list of good runs
goodRunList = qa.select{ |run,numGoldFiles| numGoldFiles>100 }.keys # require a minimum number of golden files
# goodRunList = qa.keys # take all

### loop through epochs, pick a few runs within each
epochHlist = Array.new
epochs[..-2].each_with_index do |lb,idx|

  ### get runs in this epoch, store in array
  ub = epochs[idx+1]
  epochRuns = goodRunList.select{ |run| run>=lb and run<ub }
  nRuns = epochRuns.length

  ### get array indices for a few runs, somewhat evenly spaced throughout the epoch
  idxs = []
  case nRuns
  when 0
    $stderr.puts "ERROR: epoch #{lb}-#{ub} has no runs from specified run list" if nRuns==0
  when 1..3
    idxs = (0..nRuns-1).to_a # take all
  when 4..8
    idxs = Numpy.linspace(0,nRuns-1,3).to_a.map(&:to_i) # take first, middle, last
  else
    idxs = Numpy.linspace(0,nRuns-1,5).to_a[1..-2].map(&:to_i) # linspace length 5, take middle 3
  end
  # puts "linspace: nRuns -> idxs ::: #{nRuns}  ->  #{idxs.join(',')}"

  ### map array indices to runs
  chosenRuns = idxs.map do |idx| epochRuns[idx] end

  ### add to epoch hash
  epochHlist << {
    :lb => lb,
    :ub => ub,
    :runs => chosenRuns,
  }
end

### print
# puts '-'*40
epochHlist.each_with_index do |epochH,idx|
  puts "#{idx} :: #{epochH[:lb]} <= run < #{epochH[:ub]} :: #{epochH[:runs].join(' ')}"
end
