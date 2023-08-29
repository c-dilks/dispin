#!/usr/bin/env ruby
# get info about a dataset source

require './DatasetLooper.rb'

if ARGV.length == 0
  $stderr.puts "Specify one of the following dataset sources as an argument:"
  DatasetLooper::DatasetSourceHash.each do |key,val|
    $stderr.puts "  #{key} #{val[:rating]=="GOOD" ? "   <-- good for analysis" : ""}"
  end
  exit 2
end
if ARGV.length == 1
  $stderr.puts "Also specify a field name, one of:"
  DatasetLooper::DatasetSourceHash.values.first.keys.each do |key|
    $stderr.puts "  #{key}"
  end
  exit 2
end
dataSourceName = ARGV[0]
field          = ARGV[1].to_sym

if DatasetLooper::DatasetSourceHash.include? dataSourceName
  info = DatasetLooper::DatasetSourceHash[dataSourceName]
  if info.include? field
    puts info[field]
  else
    $stderr.puts "ERROR: field '#{field}' not found for dataset source '#{dataSourceName}'"
    exit 1
  end
else
  $stderr.puts "ERROR: dataset source '#{dataSourceName}' not defined"
  exit 1
end
