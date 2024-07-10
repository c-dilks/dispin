#!/usr/bin/env ruby
# compare various diagnostic distributions for stringspinner

require 'awesome_print'
require 'pycall/import'
r = PyCall.import_module 'ROOT'

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
].map do |plot_name|
  {
    :name => plot_name,
    :canv => r.TCanvas.new("canv_#{plot_name}", "canv_#{plot_name}", 800, 600),
  }
end

# make plots
first = true
fixme normalizer
file_list.each do |file_hash|
  file_obj = r.TFile.new file_hash[:name], 'READ'
  plot_list.each do |plot_hash|
    plot_hash[:canv].cd
    plot = file_obj.Get plot_hash[:name]
    plot.SetName "#{plot.GetName}_#{file_hash[:id]}"
    plot.SetMarkerStyle file_hash[:style]
    plot.SetMarkerColor file_hash[:color]
    plot.Draw first ? 'p' : 'p same'
  end
  first = false
end

# output plots
plot_list.each do |plot_hash|
  plot_hash[:canv].SaveAs "ssscomp_#{plot_hash[:name]}.png"
end
