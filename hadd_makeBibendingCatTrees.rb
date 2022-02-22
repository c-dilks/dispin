#!/usr/bin/env ruby

if ARGV.length!=1
  puts """
  USAGE: $0 [search pattern regexp]
  - uses search pattern to identify which catTrees to hadd, then hadds them
  - it will look for trees which have not been indexed, hadd them, then 
    re-index the hadded tree
  """
  exit 2
end
pattern = ARGV[0]

# search for source files
sources = Dir.glob("catTree*.root")
  .grep(/#{pattern}/)
  .grep_v(/idx.root$/)
puts "\nhadd sources:"
puts sources

# build target file name
targetTokens = sources.map{ |source| source.split('.') }.flatten
target = targetTokens.map{|tok|tok.gsub(/.*bending$/,'bibending')}.uniq.join('.')
puts "\nhadd target:"
puts target

# confirm?
puts "\nDoes this look correct? [y/N]"
confirmation = $stdin.gets.chomp
correct = confirmation=="y"
puts "you answered " + (correct ? "yes" : "no; abort hadd")
exit unless correct

# hadd
system "hadd #{target} #{sources.join(' ')}"

# re-index
system "root -b -q 'IndexCatTree.C(\"#{target}\")'"
