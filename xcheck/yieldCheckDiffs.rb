#!/usr/bin/env ruby
# calculate fractional difference of yields for specified lines

require 'pry'
require 'fileutils'

if ARGV.length<5
  $stderr.puts """
  USAGE: #{$0} [file] [start-line-no] [end-line-no] [column A] [column B] [deleteLastColumns]
  - [start-line-no] and [end-line-no] should indicate the table from [file], including its header rows
  - [columnA] and [columnB] are column numbers of the table (starting from zero)
  - calculates |columnA-columnB| / columnA, appending (or replacing) the last column with this number
  - [deleteLastColumns]: delete this many columns from the right (default = 0)
  """
  exit 2
end
fileN = ARGV[0]
from, to, colA, colB = ARGV[1..4].map &:to_i
delete_last_columns = ARGV[5].nil? ? 0 : ARGV[5].to_i

in_table = File.readlines(fileN)[from-1..to-1].map &:chomp
puts "\nINPUT TABLE:\n\"\"\""
puts in_table
puts "\"\"\"\n\n"

tmp = File.open 'tempo', 'w'
in_table.each_with_index do |line,i|
  toks = line.split('|')[1..-1]
  delete_last_columns.times{ |i| toks.delete_at -1 }
  case i
  when 0
    a, b = [colA,colB].map{ |col| toks[col] }
    puts "COMPARING:\n#{a}\n#{b}"
    diff_header = "DIFF(#{a},#{b})".gsub(/ /,'')
    toks << "DIFF(#{a},#{b})".gsub(/ /,'')
  when 1
    toks << "---"
  else
    a, b = [colA,colB].map{ |col| toks[col].to_f }
    diff = "%.4f%%" % [ 100.0 * (a-b).abs / a ]
    toks << diff
  end
  tmp.puts ['',*toks,''].join ' | '
end
tmp.close
puts "\nRESULT:\n\n"
system 'cat tempo|column -t'
FileUtils.rm 'tempo'
