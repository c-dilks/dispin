#!/usr/bin/env ruby

require 'fileutils'
require './DatasetLooper.rb'
looper = DatasetLooper.new

looper.allsetListLoopOnlyRGA do |dataset|
  
  # triple of datasets
  triple = [dataset] # RGA
  triple << looper.matchByTorus(dataset,looper.allsetListLoopOnlyRGB) # RGB
  triple << looper.matchByTorus(dataset,looper.allsetListLoopOnlyMC) # MC

  # need torus later
  torus = dataset.split('.').find{ |tok| tok.include?"bending" }

  # catTrees
  catTrees = triple.map{ |set| looper.catTreeBaseName(set)+".idx.root" }

  # loop through ivTypes
  DatasetLooper::BinHash.keys.each do |ivType|
    tableFiles = catTrees.map{ |catTree| "meanvmean/meanvmean.#{ivType}.#{catTree}" }
    outDir = "meanvmean/img.#{torus}.#{ivType}"
    Dir.mkdir(outDir) unless Dir.exist? outDir
    system "drawBinMeans.rb #{tableFiles.join ' '}"
    FileUtils.mv Dir.glob("meanvmean/canv*.png"), outDir, verbose: true
  end
end
