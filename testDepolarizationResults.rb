#!/usr/bin/env ruby
# tests depolarization methods
# - first, edit behavior of drawBru.C, for how you want to handle depolarization,
#   in particular, the `version` number in the `getDepolarization()` call
# - then run this script
# - input fit result directories are looped over below, given by `inputGlob`
#   - drawBru.C will be re-executed for each
#   - output files will be copied to the output directory specified by the prefix
require 'fileutils'

##############################################
inputGlob = "bruspin.volatile/bruspin.minuit.ave1.*" # cf. regex in `outDir`
##############################################

### DISABLED: safer to copy full bruDir, leaving the original as backup
# if ARGV.length<1
#   puts "ERROR: specify output prefix"
#   exit 2
# end

outName = ARGV.first

# loop over input fit result directories
Dir.glob(inputGlob).each do |dir|
  system "root -b -q $BRUFIT/macros/LoadBru.C 'drawBru.C(\"#{dir}\",\"minuit\")'"
  #   outDir = dir.sub(/nodepol/,outName)
  #   FileUtils.mkdir_p outDir
  #   FileUtils.cp Dir.glob("#{dir}/asym*.root"), outDir, verbose: true
end
