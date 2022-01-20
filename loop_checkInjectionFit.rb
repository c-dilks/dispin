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
puts "bruDirList = #{bruDirList}"

# run checkInjectionFit.C on each file
listFiles = []
failures = []
bruDirList.each do |bruDir|

  # get injection number
  injNum = (bruDir.chars - bruDirGlob.chars).join.to_i
  puts "-------->>>>> CHECK INJECTION #{injNum}"

  # get list of asym*.root files
  asymFiles = Dir["#{bruDir}/asym_*.root"].reject{|f| f.include? "injectionTest"}.sort

  # start list files
  unless listFiles.length>0
    listFiles = asymFiles.map do |f|
      outDirFull = "#{outDir}_BL#{f.gsub(/^.*BL/,"").chomp(".root").to_i}" # e.g., bruspin.x.pullresults_BL0
      Dir.mkdir(outDirFull) unless Dir.exists? outDirFull
      File.open("#{outDirFull}/files.list","w")
    end
  end

  # check for asym files
  if asymFiles.length==0
    failures << injNum
    next
  end

  # run checkInjectionFit.C on each asym file
  failed = false
  asymFiles.each do |f|
    system "root -b -q 'checkInjectionFit.C(\"#{f}\",\"#{injFile}\",#{injNum})'"
    if !$?.success?
      failures << injNum
      failed = true
    end
  end
  next if failed

  # append each output file name to each list file
  listFiles.zip(asymFiles){ |listFile,asymFile| listFile.puts asymFile.gsub(/\.root$/,".injectionTest.root") }

end
puts "\nfailed injections: #{failures.uniq}"
puts "\nlist files:"
puts listFiles.map(&:inspect)
listFiles.each(&:close)
