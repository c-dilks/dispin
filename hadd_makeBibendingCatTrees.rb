#!/usr/bin/env ruby
# hadd inbending and outbending trees together, to make "bi-bending" trees
# - also has the ability to balance inbending/outbending yield ratios in MC, in
#   order to better match the data
# - several confimations will be asked before any execution

require 'fileutils'

if ARGV.length!=1
  puts """
  USAGE: $0 [search pattern regexp = (rga,rgb,mc)]
  - uses search pattern to identify which catTrees to hadd, then hadds them
  - it will look for trees which have not been indexed, hadd them, then 
    re-index the hadded tree
  - if pattern=mc, you will have the option to balance yields
  """
  exit 2
end
pattern = ARGV[0]

# search for source files
sources = Dir.glob("catTree*.root")
  .grep(/#{pattern}/)
  .grep_v(/bibending/)
  .grep_v(/idx.root$/)
  .grep_v(/TRUNCATED/)
puts "\nhadd sources:"
puts sources

# build target file name
targetTokens = sources.map{ |source| source.split('.') }.flatten
target = targetTokens.map{|tok|tok.gsub(/.*bending$/,'bibending')}.uniq.join('.')
puts "\nhadd target:"
puts target


# confirm?
def ask(question)
  print "\n#{question}\n> "
  $stdin.gets.chomp
end
correct = ask("\nDoes this look correct? [y/N]")=="y"
puts "you answered " + (correct ? "yes" : "no; abort hadd")
exit unless correct

# yield balancing
useBalance = false
if pattern.include?"mc"
  useBalance = ask("\nThese are MC data. Do you want to balance yields to match real data? [y/N]")=="y"
  if useBalance

    # get ioRatios = inbending/outbending from MC and data
    matchDataset = ask("\nBalance MC to which dataset: rga, rgb, something else?")
    ioRatioData = `root -b -q 'inbendingOutbendingYieldRatio.C("#{matchDataset}",1)'`.split("\n").last.to_f
    exit 1 if $?.to_i>0
    ioRatioMC = `root -b -q 'inbendingOutbendingYieldRatio.C("mc",1)'`.split("\n").last.to_f
    exit 1 if $?.to_i>0
    balanceFactor = ioRatioData / ioRatioMC
    puts """
    data inbending/outbending = #{ioRatioData}
      MC inbending/outbending = #{ioRatioMC}
    balanceFactor = %.4f
    """ % [balanceFactor]

    # determine which MC tree to truncate
    if balanceFactor>1
      torusToTruncate = 'outbending'
      truncateFactor = 1.0 / balanceFactor
    else
      torusToTruncate = 'inbending'
      truncateFactor = balanceFactor
    end
    treeToTruncate = sources.find{ |source| source.include? torusToTruncate }
    puts """
    TRUNCATE #{treeToTruncate}
      to %.2f%% of the original
    """ % [truncateFactor*100]

    # truncate
    puts "truncating....."
    truncation = `truncateTree.rb #{treeToTruncate} #{truncateFactor}`
    exit 1 if $?.to_i>0
    puts truncation
    truncatedTree = truncation.split("\n").last.split(" ").last

    # update hadd target and sources
    sources.map!{ |source| source==treeToTruncate ? truncatedTree : source }
    target.sub!('.mc.',".mc#{matchDataset.chars.last}.")
    puts '='*30+"\n\n"
    puts "\nupdated hadd sources:"
    puts sources
    puts "\nupdated hadd target:"
    puts target
    puts "\ntruncated tree is #{truncatedTree},\nwhich will be REMOVED after hadd!"
    correct = ask("\nDoes this look correct? [y/N]")=="y"
    puts "you answered " + (correct ? "yes" : "no; abort hadd")
    exit unless correct
  end
end

# hadd
system "hadd -f #{target} #{sources.join(' ')}"

# cleanup truncated files
FileUtils.rm truncatedTree if useBalance

# re-index
system "root -b -q 'IndexCatTree.C(\"#{target}\")'"
