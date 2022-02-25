#!/usr/bin/env ruby

require 'fileutils'
require './DatasetLooper.rb'
looper = DatasetLooper.new

looper.allsetListLoopOnlyRGA do |rgaSet|
  
  # stack of datasets
  rgbSet = looper.matchByTorus(rgaSet,looper.allsetListLoopOnlyRGB)
  stack = [rgaSet,rgbSet] # RGA and RGB
  stack << looper.matchByTorus(rgaSet,looper.allsetListLoopOnlyMC) # MC for RGA
  stack << looper.matchByTorus(rgbSet,looper.allsetListLoopOnlyMC) # MC for RGB
  stack.uniq!

  # need torus later
  torus = rgaSet.split('.').find{ |tok| tok.include?"bending" }

  # catTrees
  catTrees = stack.map{ |set| looper.catTreeBaseName(set)+".idx.root" }

  # loop through ivTypes
  DatasetLooper::BinHash.keys.each do |ivType|
    rootInFiles = catTrees.map{ |catTree| "meanvmean/meanvmean.#{ivType}.#{catTree}" }
    outDir = "meanvmean/img.#{torus}.#{ivType}"
    Dir.mkdir(outDir) unless Dir.exist? outDir
    system "drawBinMeans.rb #{rootInFiles.join ' '}"
    FileUtils.mv Dir.glob("meanvmean/canv*.png"), outDir, verbose: true
  end
end
