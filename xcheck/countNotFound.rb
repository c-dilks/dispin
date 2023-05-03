#!/usr/bin/env ruby
# count fraction events marked as "not found"
# - "0% not found in" ..._human0_human1_... means all of human0's events were found hin human1's set;
#   - check the other way around (..._human1_human0_...)
#     - "0% not found " means the two sets are identical
#     - ">0% not found " means that human1 has events that human0 does, therefore human0's set is a subset of human1's
#   

require 'pry'

subdir="4.25"

# count number of events in each table
puts "Number of events in each file:"
inTables = Dir.glob("#{subdir}/*.txt").reject{|tab|tab.include?("/compare")}.sort
system "grep --color=auto -cE '.' #{inTables.join(" ")}"

# count the number of "not founds" in each table
puts "\nFraction of events that are not found (grep -c 'not found'):"
tables = Dir.glob("#{subdir}/compare*.txt").sort
totals = tables.map{ |tab| `cat #{tab} | wc -l`.to_i-1 }
notFounds = tables.map{ |tab| `grep -c 'not found' #{tab}`.to_i }
tables.zip(totals,notFounds) do |tab,total,notFound|
  puts "%.4f%% not found in #{tab}" % [100*notFound.to_f/total.to_f]
end
