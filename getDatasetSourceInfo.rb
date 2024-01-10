#!/usr/bin/env ruby
# get info about a dataset source

require './DatasetLooper.rb'
require 'awesome_print'

# print usage
if ARGV.length == 0
  $stderr.puts "Specify one of the following dataset sources as an argument:"
  DatasetLooper::DatasetSourceHash.each do |key,val|
    $stderr.puts "  #{key} #{val[:rating]=="GOOD" ? "   <-- good for analysis" : ""}"
  end
  exit 2
end

# get info for specified dataset
dataSourceName = ARGV[0]
unless DatasetLooper::DatasetSourceHash.include? dataSourceName
  $stderr.puts "ERROR: dataset source '#{dataSourceName}' not defined"
  exit 1
end
info = DatasetLooper::DatasetSourceHash[dataSourceName]

# if no field specified, print all info
if ARGV.length == 1
  $stderr.puts "No field specified, printing all fields for dataset source '#{dataSourceName}':"
  ap info
  exit 2
end

# otherwise print specified field info only
field = ARGV[1].to_sym
unless info.include? field
  $stderr.puts "ERROR: field '#{field}' not found for dataset source '#{dataSourceName}'"
  exit 1
end
puts info[field]
