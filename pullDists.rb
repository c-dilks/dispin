#!/usr/bin/env ruby
# make pull distributions

require 'RubyROOT'
include Root

if ARGV.length < 1
  puts "USAGE #{$0} [directory]"
  exit 0
end
brudir = ARGV[0]

####################################################################
# pullSet: for storage of pull distributions, for each modulation
class PullSet

  NBINS = 100
  PULLMAX = 5

  def initialize(name,title,numBins,pad)
    @numBins = numBins
    @pad = pad
    @pullDistList = makeList do |bin|
      TH1D.create(name+"_bin#{bin}",title+" :: bin #{bin}",NBINS,-PULLMAX,PULLMAX)
    end
  end

  # return a list, with each element specified by a block, given argument `bin`
  def makeList
    @numBins.times.map do |bin|
      yield bin
    end
  end

  attr_accessor :numBins
  attr_accessor :pad
  attr_accessor :pullDistList

end
pullSetHash = Hash.new # modulation => PullSet
####################################################################


# open output file
TFile.open("#{brudir}/pulls.root","RECREATE") do |outFile|

  # loop over injectionTest files
  # - get pull graphs
  # - fill pull distributions
  File.readlines("#{brudir}/files.list").map{ |infileN| TFile.open(infileN.chomp,"READ") }.each do |inFile|
    puts "reading #{inFile.GetName}"

    # loop over pull graphs, in the 'canvPull' canvas
    inFile.Get('canvPull').GetListOfPrimitives.map(&:auto_cast).each do |pad|

      # get pull graph
      pullGr = pad.GetListOfPrimitives.find{|prim| prim.GetName.match /^pull_/}
      break unless pullGr
      pullGr = pullGr.auto_cast

      # get modulation name
      moduName = pullGr.GetName.gsub(/^.*gr_/,"").gsub(/_BL.*$/,"")

      # create PullSet instance, for this modulation
      unless pullSetHash[moduName]
        pullDistN = "pull_dist_"+moduName
        pullDistT = pullGr.GetTitle.gsub(/pull /,"").gsub(/ vs\..*$/," pulls")
        pullSetHash[moduName] = PullSet.new( pullDistN, pullDistT, pullGr.GetN, pad.GetNumber )
      end

      # fill pull distribution
      pullGr.GetN.times do |i|
        pullDist = pullSetHash[moduName].pullDistList[i]
        pullDist.Fill(pullGr.GetPointY(i))
      end

    end # loop over pull graphs

  end # loop over injectionTest files


  # draw pull distributions
  pullCanvList = []
  pullSetHash.each do |modu,ps|

    # make canvas list (one canvas per bin)
    unless pullCanvList.length>0
      ncol = 4
      nrow = pullSetHash.length/ncol + 1 # cf. drawBru.C values
      pullCanvList = ps.makeList do |bin|
        canvN = "pullCanv_#{modu}_bin#{bin}"
        canv = TCanvas.create(canvN,canvN,600*ncol,300*nrow)
        canv.Divide(ncol,nrow)
        canv
      end
    end

    # draw
    ps.pullDistList.each_with_index do |dist,i|
      pullCanvList[i].cd(ps.pad)
      dist.Draw
    end

  end # end loop over pullSetHash

  # write
  outFile.cd
  pullCanvList.each(&:Write)
  puts "wrote #{outFile.GetName}"

end # open output file
