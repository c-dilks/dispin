#!/usr/bin/env ruby
require './DatasetLooper.rb'

if ARGV.empty?
  puts "USAGE: #{$0} [outroot dir]"
  exit 2
end
outroot_dir = ARGV.first

run_list = Dir.glob("#{outroot_dir}/*.root")
  .map{ |name| File.basename name }
  .map{ |name| name.gsub /^.*_/, '' }
  .map{ |name| name.gsub /\..*$/, '' }
  .map(&:to_i)

puts "#{run_list}"
