#!/usr/bin/env ruby
# draw an arbitrary legend

require 'pycall/import'
r = PyCall.import_module 'ROOT'
r.gROOT.SetBatch true

legend = r.TLegend.new 0.1, 0.1, 0.9, 0.9

# asymmetry graphs
graphs = {
  :asymmetry => {
    :rga => { :title=>'Proton Target',   :style=>r.kFullCircle, :color=>r.kRed-7  },
    :rgb => { :title=>'Deuteron Target', :style=>r.kFullSquare, :color=>r.kBlue+3 },
    :mc  => { :title=>'Monte Carlo',     :style=>r.kOpenCircle, :color=>r.kBlack  },
  },
  :CompareCatTreeDists => {
    :a => { :title=>'StringSpinner', :style=>r.kFullTriangleUp,   :color=>r.kGreen+1  },
    :b => { :title=>'CLAS12 MCgen',  :style=>r.kFullTriangleDown, :color=>r.kViolet+2 },
  },
}

if ARGV.empty?
  $stderr.puts "USAGE: #{$0} [key]"
  $stderr.puts " keys:"
  graphs.each do |key,hash|
    $stderr.puts "   - #{key.to_s}"
  end
  exit 2
end
user_key = ARGV[0].to_sym

unless graphs.has_key? user_key
  $stderr.puts "ERROR: key '#{user_key}' not defined"
  exit 1
end

graphs[user_key].each do |key,hash|
  hash[:graph] = r.TGraph.new
  hash[:graph].SetMarkerStyle hash[:style]
  hash[:graph].SetMarkerColor hash[:color]
  hash[:graph].SetMarkerSize 3
  legend.AddEntry hash[:graph], hash[:title], 'p'
end

canv = r.TCanvas.new 'legend', 'legend', 600, 300
legend.Draw
canv.SaveAs 'legend.png'
