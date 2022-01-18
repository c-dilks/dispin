#!/usr/bin/env ruby
# make pull distributions

require 'RubyROOT'
include Root
gStyle.SetOptFit(1)

if ARGV.length < 1
  puts "USAGE #{$0} [pullresults directory]"
  exit 2
end
brudir = ARGV[0]

####################################################################
# PullSet: for storage of pull distributions
# - there will be one instance per modulation
# - a list of pull distributions is stored, one distribution per bin
class PullSet

  NBINS = 50
  PULLMAX = 10.0

  def initialize(name,title,numBins,padNum)
    @numBins = numBins
    @padNum = padNum
    @name = name
    @title = title
    @binValList = makeBinList{ |bin| 0.0 }
    @pullDistList = makeBinList do |bin|
      TH1D.create("#{@name}_bin#{bin}","#{title.gsub(/ vs\..*$/," pulls")} :: bin #{bin}",NBINS,-PULLMAX,PULLMAX)
    end
  end

  # return a list, with each element specified by a block, given argument `bin`
  def makeBinList
    @numBins.times.map do |bin|
      yield bin
    end
  end

  # fill pull distribution, and store binVal
  def fillPullDist(bin,binVal,pull)
    @pullDistList[bin].Fill(pull)
    @binValList[bin] = binVal
  end

  # perform fits
  def fit
    puts "fitting #@name"
    @fitList = makeBinList do |bin|
      fit = TF1.new("#{@name}_fit_bin#{bin}","gaus",-PULLMAX,PULLMAX)
      setpars = lambda do |parName,init,lb,ub|
        iPar = fit.GetParNumber(parName)
        fit.SetParameter(iPar,init)
        fit.SetParLimits(iPar,lb,ub)
      end
      setpars.call( "Mean",  0.0, -PULLMAX, PULLMAX )
      setpars.call( "Sigma", 1.0, 0.01,     PULLMAX )
      @pullDistList[bin].Fit(fit,"0BQ","",-PULLMAX,PULLMAX)
      fit
    end
  end

  # create a graph of fit result vs. binVal, given a parameter name
  def fitResultsGraph(parName)
    gr = TGraphErrors.create(
      @binValList,
      @fitList.map{ |fit| fit.GetParameter(fit.GetParNumber(parName)) },
      Array.new(@numBins,0.0),
      @fitList.map{ |fit| fit.GetParError(fit.GetParNumber(parName)) }
    )
    gr.SetName "#{@name}_#{parName}_graph"
    gr.SetTitle @title.gsub(" vs."," pull #{parName.downcase}s vs.")
    gr
  end

  attr_accessor :padNum
  attr_accessor :pullDistList
  attr_accessor :fitList

end
pullSetHash = Hash.new # modulation => PullSet
####################################################################


# open output file
TFile.open("#{brudir}/pulls.root","RECREATE") do |outFile|

  # loop over injectionTest files ################################
  # - get pull graphs
  # - fill pull distributions
  File.readlines("#{brudir}/files.list").map{ |infileN| TFile.open(infileN.chomp,"READ") }.each do |inFile|
    # puts "reading #{inFile.GetName}"

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
        pullDistT = pullGr.GetTitle.gsub(/pull /,"")
        pullSetHash[moduName] = PullSet.new( pullDistN, pullDistT, pullGr.GetN, pad.GetNumber )
      end
      ps = pullSetHash[moduName]

      # fill pull distribution
      pullGr.GetN.times do |bin|
        ret,binVal,pull = pullGr.GetPoint(bin)
        ps.fillPullDist(bin,binVal,pull)
        if pull.abs>PullSet::PULLMAX then $stderr.puts "ERROR: overflow pull=#{pull} for modulation #{moduName} bin #{bin}" end
      end

    end # loop over pull graphs

  end # loop over injectionTest files


  # fit and draw pull distributions ##################################################

  # make canvases
  makeCanvas = Proc.new do |canvN|
    ncol = 4
    nrow = (pullSetHash.length-1)/ncol + 1 # cf. drawBru.C values
    canv = TCanvas.create(canvN,canvN,600*ncol,300*nrow)
    canv.Divide(ncol,nrow)
    canv
  end
  fitParamCanvHash = [:Mean,:Sigma].map{ |par| [par,makeCanvas.call("#{par.to_s.downcase}Canv")] }.to_h
  fitParamIdealVal = {:Mean=>0.0,  :Sigma=>1.0}
  fitParamGraphCol = {:Mean=>KRed, :Sigma=>KGreen+2}

  # loop over modulations
  pullCanvList = []
  pullSetHash.each do |modu,ps|

    # fit pull distributions
    ps.fit

    # make pulldists canvas list (one canvas per bin)
    unless pullCanvList.length>0
      pullCanvList = ps.makeBinList{ |bin| makeCanvas.call("pullCanv_#{modu}_bin#{bin}") }
    end

    # draw pull distributions and fits
    ps.pullDistList.each_with_index do |dist,i|
      pullCanvList[i].cd(ps.padNum)
      dist.SetFillColor KAzure+10
      dist.SetLineWidth 0
      dist.Draw
      fit = ps.fitList[i]
      fit.SetLineColor KBlack
      fit.SetLineWidth 3
      fit.Draw "SAME"
      [-1.0,1.0].each do |v|
        line = TLine.new(v,0.0,v,dist.GetMaximum())
        line.SetLineColor KRed
        line.SetLineWidth 3
        line.Draw
      end
    end

    # draw fit results graphs
    fitParamCanvHash.each do |parSym,canv|
      idealVal = fitParamIdealVal[parSym]
      canv.cd(ps.padNum)
      canv.GetPad(ps.padNum).SetGrid(0,1)
      graph = ps.fitResultsGraph(parSym.to_s)
      graph.SetMarkerColor fitParamGraphCol[parSym]
      graph.SetLineColor fitParamGraphCol[parSym]
      graph.SetMarkerStyle KFullCircle
      graph.GetYaxis.SetRangeUser(idealVal-2,idealVal+2)
      graph.Draw "APE"
      line = TLine.new( graph.GetXaxis.GetXmin, idealVal, graph.GetXaxis.GetXmax, idealVal )
      line.SetLineColor KBlack
      line.SetLineWidth 3
      line.SetLineStyle KDashed
      line.Draw
    end

  end # end loop over pullSetHash

  # make pngs and write canvases
  outFile.cd
  saveCanvas = Proc.new do |canv|
    canv.SaveAs("#{brudir}/#{canv.GetName}.png")
    canv.Write
  end
  pullCanvList.each{ |canv| saveCanvas.call(canv) }
  fitParamCanvHash.each{ |parSym,canv| saveCanvas.call(canv) }

  # write
  puts "\nwrote #{outFile.GetName}"
  puts "\npngs saved: #{brudir}/*.png"

end # close output file
