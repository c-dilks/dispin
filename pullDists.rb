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
      canvPull.GetListOfPrimitives().each do |pad|
        pad.auto_cast.GetListOfPrimitives().each do |prim|
          if prim.GetName().match /^pull_/
            pullGr = prim.auto_cast
            puts [pullGr.GetName(),pullGr.class].join " "
          end
        end
      end

    end
  end
end
