#!/usr/bin/env ruby
# run checkInjectionFit.C in a loop, for all given injection models

# arguments
injFile = "injection.root"
if ARGV.length < 1
  $stderr.puts """
  USAGE: #{$0} [bruspin dir, with \"ITER\"] [injection model file]

    [bruspin dir]: fit results directory
    [bruspin dir] must have a substring, to indicate injection number, either:
    - \"inj0\", denoting the 0th injection (replaced with inj1, inj2, ...)
    - \"ITER\", denoting the injection number (replaced with 1, 2, ...)

    [injection model file]: root file with injection model
    - default = #{injFile}
  """
  exit 2
end
bruDirArg = ARGV[0]
injFile   = ARGV[1] if ARGV.length>=2


# get list of bruspin directories, and prepare output directory
if bruDirArg.include? "inj0"
  bruDirGlob = bruDirArg.gsub("inj0","inj*")
  outDir     = bruDirArg.gsub("inj0","pullresults")
elsif bruDirArg.include? "ITER"
  bruDirGlob = bruDirArg.gsub("ITER","*")
  outDir     = bruDirArg.gsub("ITER","pullresults")
else
  $stderr.puts "ERROR: bruspin dir does not contain iterator substring"
  exit 1
end
bruDirList = Dir[bruDirGlob].reject{|i|i.include?"pullresults"}
puts "outDir = #{outDir}"
puts "bruDirList = #{bruDirList}"
Dir.mkdir(outDir) unless Dir.exists? outDir


# run checkInjectionFit.C on each file
rootListFile = File.open("#{outDir}/files.list","w")
bruDirList.each do |bruDir|

  # get injection number
  injNum = (bruDir.chars - bruDirGlob.chars)[0].to_i

  # loop over asym*.root files in this bruspin dir
  Dir["#{bruDir}/asym_*.root"].reject{|f| f.include? "injectionTest"}.each do |asymFile|
    system "root -b -q 'checkInjectionFit.C(\"#{asymFile}\",\"#{injFile}\",#{injNum})'"
    rootListFile.puts asymFile.gsub(/\.root$/,".injectionTest.root")
  end

end
rootListFile.close
