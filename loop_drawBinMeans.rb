#!/usr/bin/env ruby

require 'fileutils'
require './DatasetLooper.rb'
require 'pry'

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dihadronSym = ARGV[0].to_sym
looper = DatasetLooper.new(dihadronSym)

# clean up old pngs
FileUtils.rm Dir.glob("meanvmean/*.png"), verbose: true

# loop over RGA datasets
looper.allsetListLoopOnlyRGA do |rgaSet|
  
  # stack of datasets, matching the RGA set to RGB and MC
  rgbSet = looper.matchByTorus(rgaSet,looper.allsetListLoopOnlyRGB)
  stack = [rgaSet,rgbSet] # RGA and RGB
  stack << looper.matchByTorus(rgaSet,looper.allsetListLoopOnlyMC) # MC for RGA
  stack << looper.matchByTorus(rgbSet,looper.allsetListLoopOnlyMC) # MC for RGB
  stack.uniq!

  # need torus later
  torus = rgaSet.split('.').find{ |tok| tok.include?"bending" }

  # catTrees
  catTrees = stack.map{ |set| DatasetLooper.catTreeBaseName(set)+".idx.root" }

  # loop through ivTypes
  looper.binHash.keys.each do |ivType|
    rootInFiles = catTrees.map{ |catTree| "meanvmean/meanvmean.#{ivType}.#{catTree.sub(/^.*\//,'')}" }
    system "./drawBinMeans.rb #{rootInFiles.join ' '}"
    Dir.glob("meanvmean/canv*.png") do |png|
      FileUtils.mv png, png.sub(/canv/,"img.#{torus}.#{ivType}"), verbose: true
    end
  end
end
