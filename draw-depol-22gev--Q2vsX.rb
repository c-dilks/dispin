#!/usr/bin/env ruby

# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2022 Christopher Dilks
# from https://github.com/eic/epic-analysis/blob/main/macro/depolarization/draw_Q2vsX.rb, modified for `diagnostics.cpp` plots

## plot depolarization vs (x,Q2)
## run after analysis_depolarization.C -> postprocess_depolarization.C

require 'pycall/import'
require 'pry'
r = PyCall.import_module 'ROOT'
r.gROOT.SetBatch true
r.gStyle.SetOptStat 0
Palette = 55

if ARGV.length<1
  puts "USAGE: #{$0} [out/___.canvas.root file]"
  exit 2
end
rootFileName = ARGV[0]

# target mass and beam energy
TargetMass = 0.938
beamEnergy = 0
case rootFileName
when /12gev/
  beamEnergy = 10.6
when /22gev/
  beamEnergy = 22.0
else
  $stderr.puts "WARNING: unknown beam energy"
end

# read plots
rootFile = r.TFile.new rootFileName
rootDir = rootFile.Get 'depolarizationFactors'
depolPlotList = PyCall.iterable(rootDir.GetListOfKeys)
  .select{ |key| key.GetName.match? /vsQ2vsX$/ }
  .map &:ReadObj
q2vsX = rootFile.Get 'Q2vsX'

# make TProfile2D plots
depolProfileList = depolPlotList.map do |depolPlot|
  prof = depolPlot.Project3DProfile 'yx'
  prof.GetXaxis.SetRangeUser depolPlot.GetXaxis.GetXmin, depolPlot.GetXaxis.GetXmax;
  prof.GetYaxis.SetRangeUser depolPlot.GetYaxis.GetXmin, depolPlot.GetYaxis.GetXmax;
  prof.GetZaxis.SetRangeUser depolPlot.GetZaxis.GetXmin, depolPlot.GetZaxis.GetXmax;
  prof.GetXaxis.SetTitle depolPlot.GetXaxis.GetTitle;
  prof.GetYaxis.SetTitle depolPlot.GetYaxis.GetTitle;
  prof.GetZaxis.SetTitle depolPlot.GetZaxis.GetTitle;
  prof
end

#########################################################################

# get total number of events
nTotal = q2vsX.Integral

# calculate impact on uncertainty
#   plotting the quantity 1 / [ sqrt(N_i/N_total) * depol ], so that one only
#   needs to divide it by sqrt(N_total) == sqrt(crossSec*lumi) in order to get the
#   expected statistical uncertainty. In other words, what is plotted is
#   proportional to the expected stat. unc.
uncPlotMax = 0
uncPlotMin = 9.0 #fixed
uncPlotList = depolProfileList.map do |depolProfile|
  depolPlot = depolProfile.ProjectionXY # convert TProfile2D -> TH2
  uncPlot   = depolPlot.Clone "unc_#{depolProfile.GetName}"
  uncPlot.SetTitle [ depolProfile.GetTitle, depolProfile.GetXaxis.GetTitle, depolProfile.GetYaxis.GetTitle ].join(';')
  (1..depolPlot.GetNbinsX).each do |bx|
    (1..depolPlot.GetNbinsY).each do |by|
      uncPlot.SetBinContent bx, by, 0 # clear the histogram (since it is a Clone)
      depol     = depolPlot.GetBinContent bx, by
      n         = q2vsX.GetBinContent bx, by
      sqrtNfrac = Math.sqrt n.to_f/nTotal
      unless depol==0 or sqrtNfrac==0
        impact = 1/(depol*sqrtNfrac)
        uncPlot.SetBinContent bx, by, impact
      end
    end
  end
  uncPlotMax = [uncPlotMax,uncPlot.GetMaximum].max
  uncPlot
end

#########################################################################

# depolarization plots canvas
[depolProfileList, uncPlotList].zip(['depol','unc']).each do |plotList,plotName|

  # make canvas
  nrows = 3
  ncols = 3
  canv = r.TCanvas.new plotName, plotName, nrows*1000, ncols*800
  canv.SetTopMargin  0.25
  canv.SetLeftMargin 0.25
  canv.Divide ncols, nrows

  plotList.each_with_index do |plot,i|

    # get maximum
    contents = (1..plot.GetNbinsX).map do |bx|
      (1..plot.GetNbinsY).map do |by|
        plot.GetBinContent bx, by
      end
    end.flatten.select{|e|e>0}
    min = contents.min
    max = contents.max
    plot.SetMinimum min
    plot.SetMaximum max
    # plot.GetZaxis.SetRangeUser min,max

    # get depol variable name; associate to pad number
    varName = plot.GetName.split('vs').first.sub /.*kf/, 'depol'
    padHash = {
      :depolA  => 1,
      :depolB  => 2,
      :depolV  => 3,
      :depolBA => 5,
      :depolVA => 6,
      :depolCA => 8,
      :depolWA => 9,
      :epsilon => 12,
    }

    # function to format titles
    def formatTitle(s)
      s
        .gsub( /(epsilon)/,   'var\1(x,Q^{2},y)'  )
        .gsub( /(A|B|C|V|W)/, '\1(#varepsilon,y)' )
        .gsub( /\//,          ' / '               )
    end

    # draw
    next unless padHash.has_key? varName.to_sym
    pad = canv.GetPad padHash[varName.to_sym].to_i
    pad.cd
    pad.SetGrid 1, 1
    pad.SetLogx
    pad.SetLogy
    pad.SetLogz if plotName=='unc'
    pad.SetPhi   40
    pad.SetTheta 15
    pad.SetLeftMargin   0.25
    pad.SetBottomMargin 0.15
    pad.SetTopMargin    0.10
    pad.SetRightMargin  0.15
    [ plot.GetXaxis, plot.GetYaxis, plot.GetZaxis ].each do |ax|
      ax.SetTitleOffset 1.4
      ax.SetTitleSize   0.05
      ax.SetLabelSize   0.05
    end
    [ plot.GetXaxis, plot.GetYaxis ].each do |ax| ax.SetNdivisions 30 end
    plot.SetTitle formatTitle(plot.GetTitle.gsub(/ vs.*/,''))
    case plotName
    when 'depol'
      plot.SetTitle "        #{plot.GetTitle}"
    when 'unc'
      plot.SetTitle "    1 / [ N^{1/2} #{plot.GetTitle} ]"
      plot.SetMinimum uncPlotMin
      plot.SetMaximum uncPlotMax
    end
    plot.GetZaxis.SetTitle ''
    r.gStyle.SetPalette(plotName=='depol' ? r.kBlueRedYellow : Palette)
    plot.Draw 'colz'
  end

  # depolarization canvas decorations
  canv.cd
  lines = [
    r.TLine.new(0.70, 0, 0.70, 1),
    r.TLine.new(0, 0.33, 1, 0.33),
    r.TLine.new(0, 0.67, 1, 0.67),
  ].each do |l|
    l.SetLineWidth 2
    l.SetLineColor r.kGray+1
    l.SetNDC
    l.Draw
  end
  placePol = Proc.new do |padNum,pol|
    canv.cd padNum
    t = r.TLatex.new
    t.SetTextSize 0.08
    t.DrawLatexNDC 0.0, 0.5, pol
  end
  placePol.call 1, 'UU'
  placePol.call 4, 'UL, UT'
  placePol.call 7, 'LU, LL, LT'

  canv.SaveAs rootFileName.gsub(/root$/,"#{plotName}.Q2vsX.png")

end # [depolPlotLit, uncPlotList, ...].each

#########################################################################

# y contour lines
q2plotMin = q2vsX.GetYaxis.GetXmin
q2plotMax = q2vsX.GetYaxis.GetXmax
xPlotMin  = q2vsX.GetXaxis.GetXmin
xPlotMax  = q2vsX.GetXaxis.GetXmax
# s = 4 * energy.split('x').map(&:to_f).inject(:*) # = 4 * electornBeamEn * protonBeamEn (EIC)
s = (beamEnergy+TargetMass)**2 - beamEnergy**2 # CLAS
puts "Mandelstam s = #{s}\n\n"
ytex = Array.new
ylines = [ 0.3, 0.8 ].map do |y|
  xmin = [ q2plotMin/(y*s), xPlotMin ].max
  q2min = xmin*y*s
  q2max = [ xPlotMax*y*s, q2plotMax ].min
  xmax = q2max/(y*s)
  l = r.TLine.new xmin, q2min, xmax, q2max
  l.SetLineWidth 4
  l.SetLineStyle 2
  ytex << [
    0.3*Math.sqrt(l.GetX1*l.GetX2),
    0.32*Math.sqrt(l.GetY1*l.GetY2),
    "y = #{y}",
  ]
  l
end

# Q2 vs. X plot
canvD = r.TCanvas.new 'Q2vsX', 'Q2vsX', 1000, 800
canvD.SetRightMargin 0.12
canvD.SetGrid 1, 1
canvD.SetLogx
canvD.SetLogy
canvD.SetLogz
[ q2vsX.GetXaxis, q2vsX.GetYaxis ].each do |ax| ax.SetNdivisions 30 end
q2vsX.SetTitle q2vsX.GetTitle + " for #{beamEnergy} GeV e^{-}p#rightarrowe^{-}#pi^{+}#pi^{-}X"
r.gStyle.SetPalette Palette
q2vsX.Draw 'colz'

# y-lines
ylines.each &:Draw
latex = r.TLatex.new
l0 = ylines.first
ylineAngle = latex.SetTextAngle(35)
ytex.each{ |t| latex.DrawLatex *t }

canvD.SaveAs rootFileName.gsub(/root$/,"dist.Q2vsX.png")
