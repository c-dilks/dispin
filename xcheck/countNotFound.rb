#!/usr/bin/env ruby
# count fraction events marked as "not found"

require 'pry'

subdir="2.28"
tables = Dir.glob("#{subdir}/compare*.txt").sort
totals = tables.map{ |tab| `cat #{tab} | wc -l`.to_i-1 }
notFounds = tables.map{ |tab| `grep -c 'not found' #{tab}`.to_i }

tables.zip(totals,notFounds) do |tab,total,notFound|
  puts "%.4f%% not found in #{tab}" % [100*notFound.to_f/total.to_f]
end
