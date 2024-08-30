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
    :name  => 'plots.rga.inbending.fa18.root',
    :title => 'CLAS12 Data',
    :id    => 'data',
    :color => r.kBlack,
    :style => r.kFullCross,
    :baseline => true, # baseline plot MUST be first, and there can only be one
  },
  {
    :name  => 'plots.sss.aug24_LU_2.inbending.root',
    :title => 'StringSpinner',
    :id    => 'sss',
    :color => r.kGreen+1,
    :style => r.kFullCross,
  },
  {
    :name  => 'plots.mc.richard.inbending.root',
    :title => 'CLASDIS Richard',
    :id    => 'mc_richard',
    :color => r.kBlue,
    :style => r.kFullCircle,
  },
  {
    :name  => 'plots.mc.inbending.bg45.root',
    :title => 'CLASDIS Old',
    :id    => 'mc_old',
    :color => r.kOrange-3,
    :style => r.kFullSquare,
  },
  ##### generator level testing #####
  # {
  #   :name  => 'plots.mcgen.root',
  #   :title => 'CLASDIS',
  #   :id    => 'mcgen',
  #   :color => r.kBlack,
  #   :style => r.kFullCross,
  #   :baseline => true, # baseline plot MUST be first, and there can only be one
  # },
  # { ### test enabling kT
  #   :name  => 'plots.outroot.sss.GLGT_1.4.thetaLT_0.testAandBparams.root',
  #   :title => 'S.S. kT off',
  #   :id    => 'ktoff',
  #   :color => r.kOrange,
  #   :style => r.kFullCircle,
  # },
  # { ### test enabling kT
  #   :name  => 'plots.sss.test_kt_0.64.root',
  #   :title => 'S.S. sigmaKThard = 0.64',
  #   :id    => 'kton',
  #   :color => r.kGreen+1,
  #   :style => r.kFullCircle,
  # },
  # { ### test decreasing stopMass
  #   :name  => 'plots.sss.test_stopMass_0.3.root',
  #   :title => 'S.S. stopMass = 0.3',
  #   :id    => 'stopmass03',
  #   :color => r.kBlue,
  #   :style => r.kFullCircle,
  # },
  # { ### test decreasing stopMass
  #   :name  => 'plots.sss.test_stopMass_0.0.root',
  #   :title => 'S.S. stopMass = 0.0',
  #   :id    => 'stopmass00',
  #   :color => r.kMagenta,
  #   :style => r.kFullCircle,
  # },
  # { ### test PDF sets
  #   :name  => 'plots.sss.PDF2.root',
  #   :title => 'S.S. CTEQ 5L, LO #alpha_{s}(M_{Z}) = 0.127',
  #   :id    => 'pdf2',
  #   :color => r.kRed,
  #   :style => r.kFullCircle,
  # },
  # { ### test PDF sets
  #   :name  => 'plots.sss.PDF13.root',
  #   :title => 'S.S. NNPDF2.3 QCD+QED LO #alpha_{s}(M_{Z}) = 0.130',
  #   :id    => 'pdf13',
  #   :color => r.kCyan-7,
  #   :style => r.kFullCircle,
  # },
  # { ### test PDF sets
  #   :name  => 'plots.sss.PDF14.root',
  #   :title => 'S.S. NNPDF2.3 QCD+QED LO #alpha_{s}(M_{Z}) = 0.119',
  #   :id    => 'pdf14',
  #   :color => r.kViolet+1,
  #   :style => r.kFullCircle,
  # },
  # { ### test PDF sets
  #   :name  => 'plots.sss.PDF15.root',
  #   :title => 'S.S. NNPDF2.3 QCD+QED NLO #alpha_{s}(M_{Z}) = 0.119',
  #   :id    => 'pdf15',
  #   :color => r.kGreen+2,
  #   :style => r.kFullCircle,
  # },
  # { ### test PDF sets
  #   :name  => 'plots.sss.PDF16.root',
  #   :title => 'S.S. NNPDF2.3 QCD+QED NNLO #alpha_{s}(M_{Z}) = 0.119',
  #   :id    => 'pdf16',
  #   :color => r.kBlue,
  #   :style => r.kFullCircle,
  # },
  # { ### test particleOn mode (USES pdfSet = 13)
  #   :name  => 'plots.sss.testOnMode.root',
  #   :title => 'test onMode',
  #   :id    => 'onMode',
  #   :color => r.kMagenta,
  #   :style => r.kFullCircle,
  # },
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
  'Q2Dist'     => {:logx=>false, :logy=>false},
  'WDist'      => {:logx=>false, :logy=>false},
  'XDist'      => {:logx=>false, :logy=>false},
  'YDist'      => {:logx=>false, :logy=>false},
  'PhPerpDist' => {:logx=>false, :logy=>false},
  'MhDist' => {},
  'ZpairDist' => {},
  'PhiHDist' => {},
  'PhiRDist' => {},
  'thetaDist' => {},
  'cosThetaDist' => {},
  'MmissDist' => {},
  'YHDist' => {},
  'deltaPhiDist' => {},
  'piPlushadEDist'      => { :subplot_of => 'hadECanv'     },
  'piMinushadEDist'     => { :subplot_of => 'hadECanv'     },
  'piPlushadPDist'      => { :subplot_of => 'hadPCanv'     },
  'piMinushadPDist'     => { :subplot_of => 'hadPCanv'     },
  'piPlushadPtDist'     => { :subplot_of => 'hadPtCanv'    },
  'piMinushadPtDist'    => { :subplot_of => 'hadPtCanv'    },
  'piPlushadPperpDist'  => { :subplot_of => 'hadPperpCanv' },
  'piMinushadPperpDist' => { :subplot_of => 'hadPperpCanv' },
  'piPlushadThetaDist'  => { :subplot_of => 'hadThetaCanv' },
  'piMinushadThetaDist' => { :subplot_of => 'hadThetaCanv' },
  'piPlushadPhiDist'    => { :subplot_of => 'hadPhiCanv'   },
  'piMinushadPhiDist'   => { :subplot_of => 'hadPhiCanv'   },
  'piPlushadZDist'      => { :subplot_of => 'hadZCanv'     },
  'piMinushadZDist'     => { :subplot_of => 'hadZCanv'     },
  'piPlushadXFDist'     => { :subplot_of => 'hadXFCanv'    },
  'piMinushadXFDist'    => { :subplot_of => 'hadXFCanv'    },
  'depolarizationFactors/kfAvsMh' => { :projection => 'y', :title => 'depol. A(#varepsilon,y) distribution' },
  'depolarizationFactors/kfBvsMh' => { :projection => 'y', :title => 'depol. B(#varepsilon,y) distribution'},
  'depolarizationFactors/kfCvsMh' => { :projection => 'y', :title => 'depol. C(#varepsilon,y) distribution'},
  'depolarizationFactors/kfVvsMh' => { :projection => 'y', :title => 'depol. V(#varepsilon,y) distribution'},
  'depolarizationFactors/kfWvsMh' => { :projection => 'y', :title => 'depol. W(#varepsilon,y) distribution'},
  '/symmetry/symHadP' => {},
  '/symmetry/symHadPt' => {},
  '/symmetry/symHadPperp' => {},
  '/symmetry/symHadZ' => {},
  '/symmetry/symHadTheta' => {},
}.map do |plot_name,settings|
  canv_name = "canv_#{plot_name.gsub /\//, '_'}"
  res = {
    :name => plot_name,
    :canv => r.TCanvas.new(canv_name, canv_name, 2*800, 2*600),
    :max => 0.0,
    :logx => false,
    :logy => false,
  }
  settings.each{ |k,v| res[k] = v }
  res[:canv].Divide 2,2
  [1,3].each do |pad|
    res[:canv].GetPad(pad).SetGrid 1,1
    res[:canv].GetPad(pad).SetLeftMargin 0.12
  end
  res[:canv].GetPad(1).SetBottomMargin 0.0
  res[:canv].GetPad(3).SetTopMargin 0.0
  res
end

# get a plot from a TFile
get_plot = Proc.new do |file_hash, plot_hash|
  unless plot_hash.has_key? :subplot_of # the plot is at the top-level
    hadplot = file_hash[:obj].Get(plot_hash[:name])
    if plot_hash.has_key? :projection
      case plot_hash[:projection]
      when 'x'
        resultplot = hadplot.ProjectionX "#{hadplot.GetName}_#{file_hash[:id]}"
      when 'y'
        resultplot = hadplot.ProjectionY "#{hadplot.GetName}_#{file_hash[:id]}"
      end
    else
      resultplot = hadplot
    end
    resultplot.SetTitle plot_hash[:title] if plot_hash.has_key? :title
    resultplot
  else # otherwise, the plot is buried in a TPad; drill down and get it
    hadcanv = file_hash[:obj].Get(plot_hash[:subplot_of])
    hadpad = PyCall.iterable(hadcanv.GetListOfPrimitives).first
    hadplot = PyCall.iterable(hadpad.GetListOfPrimitives).find{ |p| p.GetName == plot_hash[:name] }
    case plot_hash[:name]
    when /piPlus/
      hadplot.SetTitle "#pi^{+} #{hadplot.GetTitle.gsub /distribution.*/, 'distribution'}"
    when /piMinus/
      hadplot.SetTitle "#pi^{-} #{hadplot.GetTitle.gsub /distribution.*/, 'distribution'}"
    end
    hadplot
  end
end

# get normalizers and maxima
file_list.each do |file_hash|
  file_hash[:obj] = r.TFile.new file_hash[:name], 'READ'
  # get the electron yield, for normalization
  file_hash[:ele_yield] = file_hash[:obj].Get('dihadronCntDist').GetEntries
  # get maxima
  plot_list.each do |plot_hash|
    plot_hash[:max] = [ plot_hash[:max], get_plot.call(file_hash,plot_hash).GetMaximum/file_hash[:ele_yield] ].max
  end
end
ap plot_list

# draw each plot
legend = r.TLegend.new 0.1, 0.1, 0.9, 0.9
first_file = true
rat_hash = Hash.new
file_list.each do |file_hash|
  first_plot = true
  plot_list.each do |plot_hash|
    plot = get_plot.call file_hash, plot_hash
    plot.GetYaxis.SetTitle 'normalized counts'
    plot.Scale 1.0 / file_hash[:ele_yield]
    plot.SetMarkerStyle file_hash[:style]
    plot.SetMarkerColor file_hash[:color]
    plot.SetMarkerSize 1.25
    plot.GetXaxis.SetRangeUser 0.25, 1.0 if plot_hash[:name] == 'MhDist'
    plot.SetMaximum plot_hash[:max]*1.1
    plot_pad = plot_hash[:canv].GetPad 1
    plot_pad.cd
    if file_hash.has_key?(:baseline) and file_hash[:baseline]
      plot.SetLineWidth 2
      plot.SetLineColor file_hash[:color]
      rat_hash[plot_hash[:name]] = {
        :baseline => plot.Clone('base_'+plot.GetName),
        :canv => plot_hash[:canv],
        :rats => []
      }
      # plot.SetFillColor r.kGray
      puts "draw plot '#{plot.GetName}' to canvas '#{plot_hash[:canv].GetName}'"
      plot.Draw first_file ? 'hist' : 'hist same'
      if first_plot
        puts "add legend entry '#{file_hash[:title]}'"
        legend.AddEntry plot, file_hash[:title], 'lf'
      end
    else
      puts "draw plot '#{plot.GetName}' to canvas '#{plot_hash[:canv].GetName}'"
      plot.Draw first_file ? 'p hist' : 'p hist same'
      if first_plot
        puts "add legend entry '#{file_hash[:title]}'"
        legend.AddEntry plot, file_hash[:title], 'p'
      end
    end
    if rat_hash.has_key? plot_hash[:name]
      rat_hash[plot_hash[:name]][:rats] << plot.Clone('rat_'+plot.GetName)
    end
    first_plot = false
    plot_pad.SetLogx if plot_hash[:logx]
    plot_pad.SetLogy if plot_hash[:logy]
  end
  first_file = false
end

# redraw baseline plots on the top layer
rat_hash.each do |name,hash|
  hash[:canv].cd 1
  hash[:baseline].Draw 'hist same'
end

# draw ratios
rat_hash.each do |name,hash|
  hash[:canv].cd 3
  hash[:rat_leg] = r.TLegend.new 0.1, 0.1, 0.9, 0.9
  hash[:rats].each_with_index do |rat_plot,idx|
    rat_plot.SetTitle ''
    rat_plot.GetYaxis.SetTitle 'ratio'
    rat_plot.Divide hash[:baseline]
    rat_plot.SetMinimum 0.1
    rat_plot.SetMaximum 3.0
    rat_plot.Draw idx==0 ? 'p hist' : 'p hist same'
    # fit to 'ratio = 1' to get a chi2
    rat_plot_xmin = rat_plot.GetXaxis.GetXmin
    rat_plot_xmax = rat_plot.GetXaxis.GetXmax
    rat_fit = r.TF1.new 'fit_'+rat_plot.GetName, 'pol0', rat_plot_xmin, rat_plot_xmax
    rat_fit.FixParameter 0, 1
    rat_plot.Fit rat_fit, 'NBQ', ''
    chi2ndf = rat_fit.GetChisquare / rat_fit.GetNDF
    hash[:rat_leg].AddEntry rat_plot, "#chi^{2}/NDF = #{chi2ndf.round 3}", 'p' unless idx==0
  end
  hash[:canv].cd 4
  hash[:rat_leg].Draw
  hash[:canv].cd 2
  legend.Draw
end

# output plots
puts 'save legend'
legend_canv = r.TCanvas.new 'legend', 'legend', 600, 600
legend.Draw
legend_canv.SaveAs 'ssscomp___legend.png'
puts 'save plots'
pdfname = 'ssscomp.pdf'
plot_list.each_with_index do |plot_hash, idx|
  puts "save #{plot_hash[:name]}"
  ext = 'png'
  out_name = "ssscomp_#{plot_hash[:name].gsub '/', '_'}.#{ext}"
  ['piPlus','piMinus'].each do |str|
    if out_name.match? /#{str}/
      out_name.gsub! /#{str}/, ''
      out_name.gsub! /#{ext}$/, "#{str}.#{ext}"
    end
  end
  plot_hash[:canv].SaveAs out_name
  case idx
  when 0
    pdfsym = '('
  when plot_list.size-1
    pdfsym = ')'
  else
    pdfsym = ''
  end
  plot_hash[:canv].SaveAs pdfname + pdfsym
end

# close
file_list.each do |file_hash|
  file_hash[:obj].Close
end
