#!/usr/bin/env ruby
# wrapper for pwPlot.py to produce and organize output plots
require 'pp'
require 'fileutils'

require './DatasetLooper.rb'
looper = DatasetLooper.new

# settings #################
subDir     = "bruspin.volatile"
idString   = "final.feb3"
datasets   = ["rga", "rgb"]  ### TODO TODO TODO: update to use DatasetLooper
minimizers = ["minuit", "mcmccov"]
schemes    = [0,2,3]
Verbose = true
outputFormat = "png"

#########################################
# functions and stuff

# print a data structure (for debugging)
def printDebug(title,data)
  if Verbose
    print (title+": ").sub(/\n: /,":\n")
    pp data
  end
end

# mapping certain `BinHash` options to `pwPlot.py` options
pwOpts = {
  :xTitle  => '-x',
  :blTitle => '-e',
}

sep = "\n\n"+"#{'S'*50}\n"*3+"\n\n"

#######################################################
# build pwPlot.py commands, stacking datasets

pwPlotCmds = []
outputDirs = []
DatasetLooper::BinHash.keys.product(minimizers,schemes).each do |ivType,minimizer,scheme|

  puts "\n#{"="*30} PLOT: #{[ivType,minimizer,scheme].join ' '}" if Verbose

  # list of bruDirs, one for each dataset
  bruDirs = datasets.map do |dataset|
    "#{subDir}/" + [idString,dataset,ivType,minimizer].join('.')
  end
  outputDirs << bruDirs[0] # (only need the first one, where output files are produced)
  printDebug("bruDirs\n",bruDirs)

  # map each bruDir to a list of asym*.root files within, then zip together those lists
  bruFilesSet = bruDirs.map { |bruDir| Dir.glob(bruDir+"/asym*.root").sort }.inject(:zip)
  printDebug("bruFilesSet\n",bruFilesSet)

  # loop over files in `bruFilesSet`
  bruFilesSet.each do |bruFiles|

    # check for missing files
    if bruFiles.include? nil
      $stderr.puts "WARNING: skip missing (nil) files: #{bruFiles}"
      next
    end

    # get binlist number
    blList = bruFiles.map do |bruFile|
      bruFile.split(/_|\./).find{ |t| t.include? "BL" }.delete("BL")
    end.uniq
    $stderr.puts "WARNING: blList.length>1" if blList.length>1
    bl = blList.first

    # set title options for pwPlot.py
    titleOpts = DatasetLooper::BinHash[ivType].map do |opt,val|
      title = val.gsub("__BL__","bin #{bl}") if val.is_a? String
      pwOpt = pwOpts[opt]
      pwOpt += "'#{title}'" unless pwOpt==nil
    end
    printDebug("titleOpts",titleOpts)

    # build pwPlot command
    pwPlotCmd = [
      "./pwPlot.py",
      "-s#{scheme}",
      "-o#{outputFormat}",
      *titleOpts,
      *bruFiles,
    ].join(' ')
    pwPlotCmds << pwPlotCmd
    printDebug("pwPlotCmd\n",pwPlotCmd)
  end
end
puts sep
printDebug("pwPlot commands",pwPlotCmds)


#####################################
# execute pwPlot.py
puts sep
pwPlotCmds.each{ |cmd| system cmd }


#####################################
# collect output files
puts sep
resultDir = "#{subDir}/#{idString}.results"
Dir.mkdir(resultDir) unless Dir.exist? resultDir
outputDirs.each do |outputDir|
  Dir.glob("#{outputDir}/*.#{outputFormat}").each do |outputFile| 
    destFile = "#{resultDir}/" + outputFile.sub(/^.*#{idString}./,'').gsub('/','_')
    FileUtils.mv outputFile, destFile, verbose: true
  end
end
puts sep
puts "results collected:   #{resultDir}/*.#{outputFormat}"
