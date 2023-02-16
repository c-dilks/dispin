#!/usr/bin/env ruby

require 'fileutils'

# dataset='rgc'
dataset='22gev'

outDir = "outroot.#{dataset}.proton.all"
inDirs = Dir.glob "outroot.#{dataset}.proton.*HelProd"

FileUtils.mkdir_p outDir
inDirs.each do |inDir|
  Dir.glob("#{inDir}/*.root").each do |inFile|
    linkName = outDir + '/' +
      inDir.sub(/^outroot./,'') + '.' + File.basename(inFile)
    FileUtils.ln_sf "../#{inFile}", linkName, verbose: true 
  end
end
