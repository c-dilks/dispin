#!/usr/bin/env ruby
# compare tables from countEvents.cpp to any other table with the same format

require 'pry'
require 'awesome_print'
require 'matplotlib/pyplot'
plt = Matplotlib::Pyplot

# list of humans: each event in human[0]'s table will be searched for in human[1]'s
# (human[0] should have the smaller table, if cross checking a subset)
humans = [ "timothy", "chris"   ]
humans = [ "chris",   "timothy" ]

# build list pairs of files to compare
subdir="2.28"
tableFiles = humans.map{ |human|
  Dir.glob("#{subdir}/#{human}*.txt").sort
}.inject(:zip)
puts "COMPARISONS"
ap tableFiles

# columns, in the same order as ../countEvents.cpp
colSyms = [
  :runnum,
  :evnum,
  :helicity,
  :Q2,
  :W,
  :x,
  :y,
  :Zpair,
  :PhPerp,
  :hadXFpip,
  :hadXFpim,
  :Mmiss,
  :PhiH,
  :PhiR,
  :theta,
  :KA,
  :KC,
  :KW,
]

# hash: column sym=>index
@colIdxs = Hash.new
colSyms.each_with_index{ |sym,idx| @colIdxs[sym]=idx }

# return hash: sym => value
def getvals(cols)
  @colIdxs.map{ |sym,idx|
    [ sym, cols.at(idx).to_f ]
  }.to_h
end

# plot formatting
pltc = 4 # number of plot columns
pltr = (colSyms.length-1)/pltc+1
plt.rcParams.update(
  {
    "font.size"   => 8,
    "figure.figsize" => [2*pltr,2*pltc],
  }
)

# cross check
tableFiles.each{ |table|

  # start output file
  outFileN = table[0].sub(/#{humans[0]}/,"compare")
  outFile = File.open(outFileN,'w')
  outFile.puts '%12s '*colSyms.length % colSyms.map(&:to_s) # header

  # histograms (Arrays)
  diffHists = colSyms.map{ |sym| [sym,Array.new] }.to_h

  # loop through human0's table
  File.readlines(table[0]).each{ |line0|

    # get human0's columns:
    cols0 = line0.split(' ') # list of strings, one element per column
    vals0 = getvals(cols0) # hash: sym => value

    # find matching evnum in human1's table: grep for the event number, then make sure
    # the matching number is in the event number column
    runnum = cols0.at(@colIdxs[:runnum]) # (faster to grep for string)
    evnum = cols0.at(@colIdxs[:evnum])
    line1grep = File.readlines(table[1])
      .map(&:chomp)
      .grep(/#{evnum}/)
      .select{ |res| evnum==res.split(' ').at(@colIdxs[:evnum]) }
    found = line1grep.length>0

    # comparison
    valsDiffRounded = []
    comments = []
    if found
      cols1candidates = line1grep.map{|line|line.split(' ')}

      # if more than one match is found, proximity match
      if cols1candidates.length>1
        comments << "prox matched"
        cols1 = cols1candidates.min{ |a,b|
          valsA = getvals(a)
          valsB = getvals(b)
          Math.hypot( valsA[:Zpair]-vals0[:Zpair], valsA[:PhPerp]-vals0[:PhPerp] ) <=>
          Math.hypot( valsB[:Zpair]-vals0[:Zpair], valsB[:PhPerp]-vals0[:PhPerp] )
        }
      else
        cols1 = cols1candidates.first
      end

      # difference of columns
      colsDiff = cols0.map(&:to_f)
        .zip(cols1.map(&:to_f))
        .map{ |v| v[0]-v[1] }
      valsDiff = getvals(colsDiff)

      # make sure any angles are within [-pi,pi]
      [:PhiH,:PhiR,:theta].each do |ang|
        valsDiff[ang]-=2*Math::PI while valsDiff[ang]>Math::PI
        valsDiff[ang]+=2*Math::PI while valsDiff[ang]<-Math::PI
      end

      # differences -> rounded strings
      # replace the runnum and evnum differences (==0) by the actual runnum and evnum
      valsDiffRounded = colSyms.map{ |sym| valsDiff[sym].round(10).to_s }
      valsDiffRounded.at(@colIdxs[:runnum]).replace(runnum)
      valsDiffRounded.at(@colIdxs[:evnum]).replace(evnum)

      # fill histograms
      valsDiff.each{ |sym,val| diffHists[sym] << val }

    else
      valsDiffRounded = [runnum,evnum]
      comments << 'not found'
    end


    # append to output file, including comment
    comparisonStr = '%12s '*valsDiffRounded.length % valsDiffRounded
    comparisonStr += " // #{comments.join('; ')}" if comments.length>0
    outFile.puts comparisonStr
  }
  outFile.close
  puts "wrote #{outFileN}"

  # plot differences
  fig,axs = plt.subplots(pltr, pltc)
  fig.tight_layout(h_pad: 4)
  colSyms.each_with_index do |sym,idx|
    ax = axs[idx/pltc,idx%pltc]
    ax.set_title sym.to_s
    ax.hist(diffHists[sym], bins: 100)
  end
  plt.savefig("test.png")
  exit
}
