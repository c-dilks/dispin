#!/usr/bin/env ruby
# run allsetSymlinker.sh

require './DatasetLooper.rb'
if ARGV.length<1
  $stderr.puts "USAGE: #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
Dihadron = ARGV[0]
looper = DatasetLooper.new(Dihadron.to_sym)

looper.allsetListLoop do |dataset|
  next if dataset.include?('bibending')
  dataset.sub!("#{Dihadron}.",'')
  system "allsetSymlinker.sh #{dataset.gsub(/\.all$/,'')}"
end
