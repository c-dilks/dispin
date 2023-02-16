#!/usr/bin/env ruby
# draw an arbitrary legend

require 'pycall/import'
r = PyCall.import_module 'ROOT'
r.gROOT.SetBatch true

legend = r.TLegend.new 0.1, 0.1, 0.9, 0.9

graphs = {
  :new => { :title=>'22 GeV',   :style=>r.kFullSquare, :color=>r.kRed  },
  :old => { :title=>'10.6 GeV', :style=>r.kFullCircle, :color=>r.kBlue },
}
graphs.each do |key,hash|
  hash[:graph] = r.TGraph.new
  hash[:graph].SetMarkerStyle hash[:style]
  hash[:graph].SetMarkerColor hash[:color]
  hash[:graph].SetMarkerSize 3
  legend.AddEntry hash[:graph], hash[:title], 'p'
end

canv = r.TCanvas.new 'legend', 'legend', 600, 300
legend.Draw
canv.SaveAs 'legend.png'
