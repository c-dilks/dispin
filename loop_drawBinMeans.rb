#!/usr/bin/env ruby

require 'fileutils'
require './DatasetLooper.rb'
looper = DatasetLooper.new

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
  DatasetLooper::BinHash.keys.each do |ivType|
    rootInFiles = catTrees.map{ |catTree| "meanvmean/meanvmean.#{ivType}.#{catTree}" }
    system "./drawBinMeans.rb #{rootInFiles.join ' '}"
    Dir.glob("meanvmean/canv*.png") do |png|
      FileUtils.mv png, png.sub(/canv/,"img.#{torus}.#{ivType}"), verbose: true
    end
  end
end
