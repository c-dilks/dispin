#!/usr/bin/env ruby
# wrapper for pwPlot.py to produce and organize output plots
# for the depolarization averaging study
require 'awesome_print'
require 'fileutils'
require 'pry'

require './DatasetLooper.rb'
looper = DatasetLooper.new

# settings #################
subDir     = "bruspin.depolStudy"
idStrings  = [
  "raw",   # depolarization not used anywhere
  "depol", # depolarization in likelihood PDF
  "ave1",  # divide raw result by <W>/<A>
  "ave2",  # divide raw result by <W/A>
  "ave3",  # divide raw result by divide by W(<y>)/A(<y>)
]
dateString = "mar28"
datasets   = looper.allsetListLoopOnlyData.select{ |dataset| dataset.include?('bibending') }
minimizers = [
  "minuit",
  # "mcmccov"
]
schemes = [0,2,3]
Verbose = true
outputFormat = "png"

#########################################
# functions and stuff

# print a data structure (for debugging)
def printDebug(title,data)
  if Verbose
    print (title+": ").sub(/\n: /,":\n")
    ap data
  end
end

# mapping certain `binHash` options to `pwPlot.py` options
pwOpts = {
  :xTitle       => '-x',
  :blTitle      => '-e',
  :xTranslation => '-t',
}

sep = "\n\n"+"#{'S'*50}\n"*3+"\n\n"

#######################################################
# build pwPlot.py commands, stacking datasets

pwPlotCmds = []
outputDirs = []
looper.binHash.keys.product(datasets,minimizers,schemes).each do |ivType,dataset,minimizer,scheme|

  puts "\n#{"="*30} PLOT: #{[dataset,ivType,minimizer,scheme].join ' '}" if Verbose

  # list of bruDirs, one for each idString
  bruDirs = idStrings.map do |idString|
    "#{subDir}/#{[idString,dateString,dataset,ivType,minimizer].join('.')}"
  end
  outputDirs << bruDirs[0] # (only need the first one, where output files are produced)
  printDebug("bruDirs\n",bruDirs)

  # map each bruDir to a list of asym*.root files within, then zip together those lists
  bruFilesSet = bruDirs.map { |bruDir| Dir.glob(bruDir+"/asym*.root").sort }.inject(:zip).map(&:flatten)
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

    # set extra options for pwPlot.py
    extraOpts = looper.binHash[ivType].map do |opt,val|
      title = val.gsub("__BL__","bin #{bl}") if val.is_a? String
      pwOpt = pwOpts[opt]
      unless pwOpt==nil
        if val.is_a? String
          pwOpt += "'#{title}'"
        else
          pwOpt += "#{val}"
        end
      end
    end
    printDebug("extraOpts",extraOpts)

    # build pwPlot command
    pwPlotCmd = [
      "./pwPlot.py",
      "-s#{scheme}",
      "-o#{outputFormat}",
      *extraOpts,
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
resultDir = "#{subDir}/#{dateString}.results"
FileUtils.mkdir_p resultDir
outputDirs.each do |outputDir|
  Dir.glob("#{outputDir}/*.#{outputFormat}").each do |outputFile| 
    destFile = "#{resultDir}/" + outputFile.sub(/^.*#{dateString}./,'').gsub('/','_')
    FileUtils.mv outputFile, destFile, verbose: true
  end
end
puts sep
puts "results collected:   #{resultDir}/*.#{outputFormat}"
