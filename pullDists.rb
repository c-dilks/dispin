#!/usr/bin/env ruby
# make pull distributions

require 'RubyROOT'
include Root

if ARGV.length < 1
  puts "USAGE #{$0} [directory]"
  exit 0
end
brudir = ARGV[0]

# loop over injectionTest result files
inList = File.readlines("#{brudir}/files.list").map(&:chomp)
TFile.open("#{brudir}/pulls.root","RECREATE") do
  inList.each do |inFileN|

    # open injectionTest result file
    TFile.open(inFileN,"READ") do |inFile|
      canvPull = inFile.Get('canvPull')

      # loop over pull graphs
      canvPull.GetListOfPrimitives.map(&:auto_cast).each do |pad|
        pullGr = pad.GetListOfPrimitives.find{|prim| prim.GetName.match /^pull_/}
        break unless pullGr
        pullGr = pullGr.auto_cast
        puts [pullGr.GetName,pullGr.class].join " "

        pullGr.GetN.times do |i|
          i,x,y = pullGr.GetPoint(i)
          puts [x,y].join " "
        end
      end

    end
  end
end
