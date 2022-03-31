#!/usr/bin/env ruby
# draw <depolarization> vs <bin mean>

require 'pry'
require 'RubyROOT'
include Root

Interactive = true # if true, draw, otherwise just run in background
include RootApp if Interactive

if ARGV.length<1
  puts "USAGE #{$0} [TabulateBinMeans.exe output root file]"
  exit 2
end
inFileN = ARGV[0]

depolList = [ 'depol2', 'depol3' ]
aveList = [ 'ave1', 'ave2', 'ave3' ]
grFormat = {
  0 => { :color=>KRed-7,   :marker=>KFullCircle,     },
  1 => { :color=>KBlue+3,  :marker=>KFullSquare,     },
  2 => { :color=>KGreen+1, :marker=>KFullTriangleUp, },
}

TFile.open(inFileN,"READ") do |inFile|
  multiGraphs = depolList.map do |depol|
    mgr = TMultiGraph.new
    mgr.SetName depol
    xname = ''
    aveList.each_with_index do |ave,idx|
      gr = inFile.Get("#{depol}_via_#{ave}")
      gr.SetMarkerStyle grFormat[idx][:marker]
      gr.SetMarkerColor grFormat[idx][:color]
      gr.SetLineColor   grFormat[idx][:color]
      gr.SetMarkerSize  2
      mgr.Add gr
      xname = gr.GetXaxis.GetTitle
    end
    mgr.SetTitle [
      depol.sub(/depol/,'depol. for twist '),
      xname,
      'depol.',
    ].join(';')
    mgr
  end
  multiGraphs.each_with_index do |mgr,idx|
    canv = TCanvas.create("canv#{idx}","canv#{idx}",800,500)
    canv.SetLeftMargin 0.15
    canv.SetGrid 1,1
    mgr.Draw "APLE"
    mgr.GetYaxis.SetRangeUser 0.6, 0.9
  end
end
  
run_app if Interactive
