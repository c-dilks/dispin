#!/usr/bin/env ruby
# run countEvents.cpp and produce tables for cross check

require 'pry'
require '../DatasetLooper.rb'
dl=DatasetLooper.new

### build list of files and run numbers
# subdir="2.28"
# timothyFiles = Dir.glob("#{subdir}/timothy*.txt")
# chrisFiles = timothyFiles.map{|f|f.sub('timothy','chris')}
# runNums = chrisFiles.map{ |f| File.basename(f,'.txt').split('_').last }
###
# subdir="4.12"
# subdir="4.14"
# subdir="4.19"
subdir="4.25"
timothyFiles = Dir.glob("#{subdir}/timothy*.txt")
chrisFiles = timothyFiles.map{|f|f.sub('timothy','chris')}
runNums = chrisFiles.map{ |f| File.basename(f,'.txt').split('_')[1] }

# find outroot file for each run number
pwd = Dir.pwd
Dir.chdir '..'
outrootDirs = dl.allsetListLoop.map{ |dataset| "outroot.#{dataset}" }
outrootFilesFull = outrootDirs.map{ |outrootDir| Dir.glob("#{outrootDir}/*.root") }.flatten
outrootFiles = runNums.map{ |runNum|
  outrootFilesFull.find{ |outrootFile|
    outrootFile.include? runNum
  }
}

# produce tables
outrootFiles.zip(chrisFiles).each{ |outrootFile,chrisFile|
  system "countEvents.exe #{outrootFile} 0x34 #{pwd}/#{chrisFile}"
}
Dir.chdir pwd
