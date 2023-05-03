#!/usr/bin/env ruby
# compare tables from countEvents.cpp to any other table with the same format
#
# - specific for comparing intermediate values in fiducial cuts
#

require 'pry'
require 'thread/pool'
require 'matplotlib/pyplot'

plt = Matplotlib::Pyplot
pool = Thread.pool(`nproc`.to_i-2)

##### SETTINGS #########################################
# truncation limit: large files will take a long time, consider truncating them during reading
# - if nonzero, will only read the first #{limiter} lines
# - set to '-1' to read all of the lines
limiter = -1 
# list of humans: each event in human[0]'s table will be searched for in human[1]'s
# - output files will be named "...human0_human1..."
humans = []
humans << "chris"
humans << "timothy"
########################################################

# build list pairs of files to compare
subdir="4.20"
tableFiles = humans.map{ |human|
  Dir.glob("#{subdir}/#{human}*.txt").sort
}.inject(:zip)
puts "COMPARISONS"

# columns, in the same order as ../countEvents.cpp
colSyms = [
 :evnum,
 :x,
 :y,
 :calcMin,
 :calcMax,
 :region,
 :sector,
 :pid,
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
    "figure.figsize" => [2*pltc,2*pltr],
  }
)

# cross check
linesArr = Array.new(2)
xcheck = Proc.new{ |tablePair|
  puts "-> compare:"
  puts tablePair

  # start output file
  outFileN = tablePair[0].sub(/#{humans[0]}/,"compare_#{humans.join('_')}")
  outFile = File.open(outFileN,'w')
  outFile.puts '%12s '*colSyms.length % colSyms.map(&:to_s) # header

  # histograms (Arrays)
  diffHists = colSyms.map{ |sym| [sym,Array.new] }.to_h

  # read lines of each tablePair, truncating to #{limiter} lines
  linesArr = tablePair.map{ |table|
    File.readlines(table)[0..limiter].map(&:chomp)
  }

  # loop through human0's tablePair
  cnt=0
  linesArr[0].each{ |line0|
    puts "progress: #{100*cnt.to_f/linesArr[0].length.to_f}%" if cnt%1000==1

    # get human0's columns:
    cols0 = line0.split(' ') # list of strings, one element per column
    vals0 = getvals(cols0) # hash: sym => value

    # find matching evnum in human1's tablePair: grep for the event number, then make sure
    # the matching number is in the event number column
    pid = cols0.at(@colIdxs[:pid]) # (faster to grep for string)
    evnum = cols0.at(@colIdxs[:evnum])
    region = cols0.at(@colIdxs[:region])
    line1grep = linesArr[1]
      .grep(/#{evnum}/)
      .select{ |res| evnum  == res.split(' ').at(@colIdxs[:evnum])  }
      .select{ |res| pid    == res.split(' ').at(@colIdxs[:pid])    }
      .select{ |res| region == res.split(' ').at(@colIdxs[:region]) }
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
          Math.hypot( valsA[:x]-vals0[:x], valsA[:y]-vals0[:y] ) <=>
          Math.hypot( valsB[:x]-vals0[:x], valsB[:y]-vals0[:y] )
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
      # [:PhiH,:PhiR,:theta].each do |ang|
      #   valsDiff[ang]-=2*Math::PI while valsDiff[ang]>Math::PI
      #   valsDiff[ang]+=2*Math::PI while valsDiff[ang]<-Math::PI
      # end

      # differences -> rounded strings
      # replace the pid and evnum differences (==0) by the actual pid and evnum
      valsDiffRounded = colSyms.map{ |sym| valsDiff[sym].round(10).to_s }
      valsDiffRounded.at(@colIdxs[:pid]).replace(pid)
      valsDiffRounded.at(@colIdxs[:evnum]).replace(evnum)

      # fill histograms
      valsDiff.each{ |sym,val| diffHists[sym] << val }

    else
      valsDiffRounded = [evnum,pid,region]
      comments << 'not found ( evnum pid region )'
    end


    # append to output file, including comment
    comparisonStr = '%12s '*valsDiffRounded.length % valsDiffRounded
    comparisonStr += " // #{comments.join('; ')}" if comments.length>0
    outFile.puts comparisonStr
    cnt += 1
  }
  outFile.close
  puts "wrote #{outFileN}"

  # plot differences
  fig,axs = plt.subplots(pltr, pltc)
  fig.tight_layout(h_pad: 4)
  colSyms.each_with_index do |sym,idx|
    ax = axs[idx/pltc,idx%pltc]
    ax.set_title sym.to_s
    ax.set_yscale 'log'
    ax.hist(diffHists[sym], bins: 100)
    ax.set_ylim bottom: 0.1
  end
  pngN =outFileN.sub(/\.txt$/,'.png')
  plt.savefig(pngN)
  puts "wrote #{pngN}"
}


# execution
tableFiles.each{ |tablePair|
  # pool.process{ xcheck.call(tablePair) } #FIXME: matplotlib GUI must be in main thread, for now multi-threading is not possible
  xcheck.call(tablePair) # single-threaded
}
pool.shutdown
