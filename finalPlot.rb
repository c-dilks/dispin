#!/usr/bin/env ruby
# wrapper for pwPlot.py to produce and organize output plots
require 'awesome_print'
require 'fileutils'

require './DatasetLooper.rb'

# args
if ARGV.length!=1
  $stderr.puts "USAGE #{$0} [DIHADRON]"
  DatasetLooper.printDihadrons
  exit 2
end
dihadronSym = ARGV[0].to_sym
looper = DatasetLooper.new(dihadronSym)

# settings #################
subDir     = "bruspin"
idString   = "TRANSVERSITY2022"
datasets   = looper.allsetListLoopOnlyData
minimizers = [
  "minuit",
  # "mcmccov"
]
schemes = {
  :pm => [0,2,3],
  :p0 => [12,13],
  :m0 => [12,13],
}
tori = [
  "inbending",
  "outbending",
  "bibending",
]
Verbose = true
outputFormat = "png"
UseCustomHighDimTitles = true # if true, use the following custom `highDimTitles` for 2D and 3D binning schemes
highDimTitles = {
  0 => '$M_h<0.63$ GeV',
  1 => '$M_h>0.63$ GeV',
}

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
looper.binHash.keys.product(tori,minimizers,schemes[dihadronSym]).each do |ivType,torus,minimizer,scheme|

  ivName = looper.binHash[ivType][:name]
  puts "\n#{"="*30} PLOT: #{[torus,ivName,minimizer,scheme].join ' '}" if Verbose

  # list of bruDirs, one for each dataset, filtered for the given torus polarity
  legendLabels = []
  bruDirs = datasets.select{|set|set.include?torus}.map do |dataset|
    legendLabels << DatasetLooper::datasetTarget(dataset)
    "#{subDir}/" + [idString,dataset,ivName,minimizer].join('.')
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
      bruFile.split(/_|\./).find{ |t| t.include? "BL" }.delete("BL").to_i
    end.uniq
    $stderr.puts "WARNING: blList.length>1" if blList.length>1
    bl = blList.first

    # set title options for pwPlot.py
    titleOpts = looper.binHash[ivType].map do |opt,val|
      title = ''
      if val.is_a? String
        if UseCustomHighDimTitles
          title = val.include?('__BL__') ? highDimTitles[bl] : val
        else
          title = val.gsub("__BL__","bin #{bl+1}")
        end
      end
      pwOpt = pwOpts[opt]
      unless pwOpt==nil
        if val.is_a? String
          pwOpt += "'#{title}'"
        else
          pwOpt += "#{val}"
        end
      end
    end.compact
    printDebug("titleOpts",titleOpts)

    # build pwPlot command
    pwPlotCmd = [
      "./pwPlot.py",
      "-s#{scheme}",
      "-o#{outputFormat}",
      "-l'#{legendLabels.join(';')}'",
      ['TRANSVERSITY2022'].include?(idString) ? '-w' : '',
      *titleOpts,
      *bruFiles,
    ].join(' ')
    pwPlotCmds << pwPlotCmd
    printDebug("pwPlotCmd\n",pwPlotCmd)
  end
end
puts sep
printDebug("pwPlot commands",pwPlotCmds)

# exit # premature exit, for testing


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
