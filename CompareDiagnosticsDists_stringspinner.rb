#!/usr/bin/env ruby
# compare various diagnostic distributions for stringspinner

require 'awesome_print'
require 'pycall/import'
r = PyCall.import_module 'ROOT'

r.gStyle.SetOptStat 0
r.gROOT.SetBatch true

# NOTE: this script takes NO ARGUMENTS, instead, see the following
# to decide the input files, their plot styles, etc.
##################################################################################
# get the list of files, and decide legend titles and plot styles
file_list = Dir.glob("plots.outroot.sss*.root").map do |file_name|
  # make the title
  file_id    = file_name.gsub(/^plots.*sss\./,'').gsub(/\.root$/,'')
  glgt_mag   = file_id.sub(/\.thetaLT.*/,'').split('_').last.to_f
  glgt_arg   = file_id.sub(/^.*thetaLT/,'').split('_').last.to_f
  f_L        = glgt_mag**2 / ( 2 + glgt_mag**2)
  file_title = "f_{L} = #{f_L.round 2}, #theta_{LT} = #{glgt_arg}"
  # pick color
  color = r.kBlack
  case f_L
  when 0..0.25
    color = r.kRed
  when 0.25..0.75
    color = r.kGreen + 1
  when 0.75..1
    color = r.kBlue
  end
  # pick style
  style = r.kFullCross
  case glgt_arg
  when -Math::PI..-Math::PI/4
    style = r.kFullTriangleDown
  when -Math::PI/4..Math::PI/4
    style = r.kFullCircle
  when Math::PI/4..Math::PI
    style = r.kFullTriangleUp
  end
  # hash
  {
    :name  => file_name,
    :title => file_title,
    :id    => file_id,
    :color => color,
    :style => style,
  }
end
ap file_list
##################################################################################

# list of plots we want to draw
plot_list = [
  'MhDist',
  'Q2Dist',
  'XDist',
  'PhPerpDist',
  'ZpairDist',
  'PhiHDist',
  'PhiRDist',
  'thetaDist',
].map do |plot_name|
  {
    :name => plot_name,
    :canv => r.TCanvas.new("canv_#{plot_name}", "canv_#{plot_name}", 2400, 1800),
  }
end

# make plots
legend = r.TLegend.new 0.1, 0.1, 0.9, 0.9
first_file = true
file_list.each do |file_hash|
  file_obj = r.TFile.new file_hash[:name], 'READ'
  # get the electron yield, for normalization
  ele_yield = file_obj.Get('dihadronCntDist').GetEntries
  # draw each plot
  first_plot = true
  plot_list.each do |plot_hash|
    plot_hash[:canv].cd
    plot = file_obj.Get plot_hash[:name]
    plot.Scale 1.0 / ele_yield
    plot.SetName "#{plot.GetName}_#{file_hash[:id]}"
    plot.SetMarkerStyle file_hash[:style]
    plot.SetMarkerColor file_hash[:color]
    plot.SetMarkerSize 1.5
    plot.GetXaxis.SetRangeUser 0.25, 1.0
    plot.Draw first_file ? 'p hist' : 'p hist same'
    legend.AddEntry plot, file_hash[:title], 'p' if first_plot
    first_plot = false
  end
  first_file = false
end

# output plots
plot_list.each do |plot_hash|
  plot_hash[:canv].SaveAs "ssscomp_#{plot_hash[:name]}.png"
end
legend_canv = r.TCanvas.new 'legend', 'legend', 600, 600
legend.Draw
legend_canv.SaveAs 'legend.png'
