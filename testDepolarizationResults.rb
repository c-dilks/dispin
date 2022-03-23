#!/usr/bin/env ruby
# tests depolarization methods
# - first, edit behavior of drawBru.C, for how you want to handle depolarization
# - then run this script
# - input fit result directories are looped over below
#   - drawBru.C will be re-executed for each
#   - output files will be copied to the output directory specified by the prefix
require 'fileutils'

#######################
subDir = "bruspin.work"
#######################

if ARGV.length<1
  puts "ERROR: specify output prefix"
  exit 2
end

outName = ARGV.first

# loop over input fit result directories
Dir.glob("#{subDir}/nodepol*").each do |dir|
  system "root -b -q $BRUFIT/macros/LoadBru.C 'drawBru.C(\"#{dir}\",\"minuit\")'"
  outDir = dir.sub(/nodepol/,outName)
  FileUtils.mkdir_p outDir
  FileUtils.cp Dir.glob("#{dir}/asym*.root"), outDir, verbose: true
end
