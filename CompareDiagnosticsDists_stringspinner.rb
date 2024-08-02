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
file_list = [
  {
    :name  => 'plots.outroot.sss.GLGT_1.4.thetaLT_0.testAandBparams.root',
    :title => 'S.S. kT off',
    :id    => 'origin',
    :color => r.kRed,
    :style => r.kFullCircle,
  },
  {
    :name  => 'plots.sss.test_kt_0.64.root',
    :title => 'S.S. sigmaKThard = 0.64',
    :id    => 'origin',
    :color => r.kGreen+1,
    :style => r.kFullCircle,
  },
  {
    :name  => 'plots.sss.root',
    :title => 'S.S. new',
    :id    => 'origin',
    :color => r.kBlue,
    :style => r.kFullCircle,
  },
  {
    :name  => 'plots.mcgen.root',
    :title => 'CLASDIS',
    :id    => 'origin',
    :color => r.kBlack,
    :style => r.kFullCross,
    :special => true,
  },
]
# Dir.glob("plots.outroot.sss*.root").reject{|f|f.include?'test'}.each do |file_name|
#   # make the title
#   file_id    = file_name.gsub(/^plots.*sss\./,'').gsub(/\.root$/,'')
#   glgt_mag   = file_id.sub(/\.thetaLT.*/,'').split('_').last.to_f
#   glgt_arg   = file_id.sub(/^.*thetaLT/,'').split('_').last.to_f
#   f_L        = glgt_mag**2 / ( 2 + glgt_mag**2)
#   file_title = "f_{L} = #{f_L.round 2}, #theta_{LT} = #{glgt_arg}"
#   # pick color
#   color = r.kBlack
#   case f_L
#   when 0..0.25
#     color = r.kRed
#   when 0.25..0.75
#     color = r.kGreen + 1
#   when 0.75..1
#     color = r.kBlue
#   end
#   # pick style
#   style = r.kFullCross
#   case glgt_arg
#   when -Math::PI..-Math::PI/4
#     style = r.kFullTriangleDown
#   when -Math::PI/4..Math::PI/4
#     style = r.kFullCircle
#   when Math::PI/4..Math::PI
#     style = r.kFullTriangleUp
#   end
#   # hash
#   file_list << {
#     :name  => file_name,
#     :title => file_title,
#     :id    => file_id,
#     :color => color,
#     :style => style,
#   }
# end
ap file_list
##################################################################################

# list of plots we want to draw
plot_list = {
  'MhDist' => {},
  'Q2Dist' => {:logx=>false, :logy=>false},
  'XDist' => {:logx=>false, :logy=>false},
  'PhPerpDist' => {:logx=>false, :logy=>false},
  'ZpairDist' => {},
  'PhiHDist' => {},
  'PhiRDist' => {},
  'thetaDist' => {},
  '/symmetry/symHadP' => {},
  '/symmetry/symHadPt' => {},
  '/symmetry/symHadPperp' => {},
  '/symmetry/symHadZ' => {},
  '/symmetry/symHadTheta' => {},
}.map do |plot_name,settings|
  res = {
    :name => plot_name,
    :canv => r.TCanvas.new("canv_#{plot_name}", "canv_#{plot_name}", 1600, 1200),
    :max => 0.0,
  }
  settings.each{ |k,v| res[k] = v }
  res
end

# make plots
legend = r.TLegend.new 0.1, 0.1, 0.9, 0.9
first_file = true
## get normalizers and maxima
file_list.each do |file_hash|
  file_hash[:obj] = r.TFile.new file_hash[:name], 'READ'
  # get the electron yield, for normalization
  file_hash[:ele_yield] = file_hash[:obj].Get('dihadronCntDist').GetEntries
  # get maxima
  plot_list.each do |plot_hash|
    plot_hash[:max] = [ plot_hash[:max], file_hash[:obj].Get(plot_hash[:name]).GetMaximum/file_hash[:ele_yield] ].max
  end
end
## draw each plot
file_list.each do |file_hash|
  first_plot = true
  plot_list.each do |plot_hash|
    plot_hash[:canv].cd
    plot_hash[:canv].SetGrid 1,1
    plot = file_hash[:obj].Get(plot_hash[:name])#.Clone "#{plot_hash[:name].gsub '/', '_'}_#{file_hash[:id]}"
    plot.Scale 1.0 / file_hash[:ele_yield]
    plot.SetMarkerStyle file_hash[:style]
    plot.SetMarkerColor file_hash[:color]
    plot.SetMarkerSize 0.75
    plot.GetXaxis.SetRangeUser 0.25, 1.0 if plot_hash[:name] == 'MhDist'
    plot.SetMaximum plot_hash[:max]*1.1
    if file_hash.has_key?(:special) and file_hash[:special]
      plot.SetLineWidth 3
      plot.SetLineColor file_hash[:color]
      # plot.SetFillColor r.kGray
      puts "draw plot '#{plot.GetName}'"
      plot.Draw first_file ? 'hist' : 'hist same'
      if first_plot
        puts "add legend entry '#{file_hash[:title]}'"
        legend.AddEntry plot, file_hash[:title], 'lf'
      end
    else
      puts "draw plot '#{plot.GetName}'"
      plot.Draw first_file ? 'p hist' : 'p hist same'
      if first_plot
        puts "add legend entry '#{file_hash[:title]}'"
        legend.AddEntry plot, file_hash[:title], 'p'
      end
    end
    first_plot = false
    plot_hash[:canv].SetLogx if plot_hash[:logx]
    plot_hash[:canv].SetLogy if plot_hash[:logy]
  end
  first_file = false
end

# output plots
puts 'save legend'
legend_canv = r.TCanvas.new 'legend', 'legend', 600, 600
legend.Draw
legend_canv.SaveAs 'ssscomp___legend.png'
puts 'save plots'
plot_list.each do |plot_hash|
  puts "save #{plot_hash[:name]}"
  plot_hash[:canv].SaveAs "ssscomp_#{plot_hash[:name].gsub '/', '_'}.png"
end

# close
file_list.each do |file_hash|
  file_hash[:obj].Close
end
