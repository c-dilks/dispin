#!/usr/bin/env ruby
# grep for [ERR_PATTERN] in farm_out *.err files, then
# grep for [LOG_PATTERN] in corresponding farm_out *.out files

if ARGV.length<2
  puts """
  USAGE: #{$0} [ERR_PATTERN] [LOG_PATTERN] [CASE_INSENSITIVE]

  grep for [ERR_PATTERN] in farm_out *.err files, then
  grep for [LOG_PATTERN] in corresponding farm_out *.out files

  if there is a third argument [CASE_INSENSITIVE], grep will
  run with -i option

  """
  exit 2
end
patErr,patLog = ARGV
grep = ARGV.length>=3 ? "grep -i" : "grep"

# execute a command, print and return results
def exe(cmd)
  puts "\n-------->>> #{cmd}"
  ret = `#{cmd}`
  puts ret
  ret
end

# get list of *.err files with [ERR_PATTERN]
grepErr = exe "errorPrint.sh | #{grep} #{patErr}"
errFiles = grepErr.gsub(/:.*$/,"").split(/\n/).uniq
puts "\n#{"="*40}"

# get list of corresponding *.out files
outFiles = errFiles.map{ |f| f.gsub(/\.err$/,".out") }

# grep for [LOG_PATTERN] in *.out files
outFiles.each{ |log| exe "#{grep} #{patLog} #{log}" }
