#!/usr/bin/env ruby
# compare various diagnostic distributions for stringspinner

require 'awesome_print'
require 'pycall/import'
require 'pry'
r = PyCall.import_module 'ROOT'
r.gStyle.SetOptStat 0
r.gROOT.SetBatch true

if ARGV.size<1
  $stderr.puts "#{$0} [symmetric/asymmetric]"
  exit 1
end
sym_arg = ARGV[0]

# open trees
trees = {
  # :harut   => r.TTree.new,
  :timothy => r.TFile.new('timothy_dihadron_theta_calculation.root').Get('PhysicsEvents'),
  :chris   => r.TFile.new('outroot.test.inbending/sidisdvcs_016771.hipo.root').Get('tree'),
}
# [:harut, :timothy].each do |human|
#   trees[human].ReadFile("theta_xcheck.dat")
# end

# cuts
cuts_options = {
  :symmetric => {
    :harut   => 'TMath::Abs(pipp-pimp) < 1',
    :timothy => 'TMath::Abs(p1_p-p2_p) < 1',
    :chris   => 'TMath::Abs(hadP[0]-hadP[1]) < 1 && eleP > 2.64 && Q2 > 1 && W > 2 && y < 0.75',
  },
  :asymmetric => {
    :harut   => 'TMath::Abs(pipp-pimp) > 1',
    :timothy => 'TMath::Abs(p1_p-p2_p) > 1',
    :chris   => 'TMath::Abs(hadP[0]-hadP[1]) > 1 && eleP > 2.64 && Q2 > 1 && W > 2 && y < 0.75',
  },
}
cuts = cuts_options[sym_arg.to_sym]
if cuts.nil?
  $stderr.puts 'error: bad argument'
  exit 1
end

# variables and their ranges
var_hash = {
  :theta    => { :harut => 'theta_proton_calc', :timothy => 'theta',    :chris => 'theta',                                   :range => [0.0, Math::PI]   },
  :x        => { :harut => '',                  :timothy => 'x',        :chris => 'x',                                       :range => [0.0, 1.0]        },
  :Q2       => { :harut => '',                  :timothy => 'Q2',       :chris => 'Q2',                                      :range => [0.0, 10.0]       },
  :Mh       => { :harut => 'pippim_mass',       :timothy => 'Mh',       :chris => 'Mh',                                      :range => [0.0, 5.0]        },
  :eleE     => { :harut => 'elee',              :timothy => 'e_p',      :chris => 'eleP',                                    :range => [0.0, 11.0]       },
  :eleTheta => { :harut => 'lelet',             :timothy => 'e_theta',  :chris => '2*TMath::ATan(TMath::Exp(-1*eleEta))',    :range => [0.0, Math::PI/4] },
  :pipP     => { :harut => 'pipp',              :timothy => 'p1_p',     :chris => 'hadP[0]',                                 :range => [0.0, 11.0]       },
  :pipTheta => { :harut => 'pipt',              :timothy => 'p1_theta', :chris => '2*TMath::ATan(TMath::Exp(-1*hadEta[0]))', :range => [0.0, Math::PI/4] },
  :pimP     => { :harut => 'pimp',              :timothy => 'p2_p',     :chris => 'hadP[1]',                                 :range => [0.0, 11.0]       },
  :pimTheta => { :harut => 'pimt',              :timothy => 'p2_theta', :chris => '2*TMath::ATan(TMath::Exp(-1*hadEta[1]))', :range => [0.0, Math::PI/4] },
}

# define histograms
Nbins = 50
plot_hash = var_hash.keys.map do |var_key|
  plots = trees.keys.map{ |human| [ human, r.TH1D.new("#{var_key.to_s}__#{human.to_s}", "#{var_key.to_s} (#{sym_arg})", Nbins, *var_hash[var_key][:range]) ] }.to_h
  plots[:chris].SetLineColor r.kRed
  # plots[:harut].SetLineColor r.kGreen+1
  plots[:timothy].SetLineColor r.kBlue
  [ var_key, plots ]
end.to_h

# define canvases
canv_hash = var_hash.keys.map do |var_key|
  [ var_key, r.TCanvas.new(var_key.to_s, var_key.to_s, 800, 600) ]
end.to_h

# project
canv_hash.each do |var_key, canv|
  canv.cd
  trees.keys.each_with_index do |human, idx|
    plot = plot_hash[var_key][human]
    trees[human].Project plot.GetName, var_hash[var_key][human], cuts[human]
    plot.Scale 1.0/plot.Integral # normalize
  end
  trees.keys.each do |human| # set maximum y-scale
    plot_hash[var_key][human].SetMaximum plot_hash[var_key].values.map(&:GetMaximum).max*1.1
  end
  plot_hash[var_key].values.each_with_index do |plot, idx| # draw
    plot.Draw idx==0 ? 'hist' : 'hist same'
  end
end

# print PDF
OutName = "xcheck_#{sym_arg}.pdf"
canv_hash.values.each_with_index do |canv, idx|
  if canv_hash.size == 1
    canv.SaveAs OutName
  else
    case idx
    when 0
      canv.SaveAs OutName+'('
    when canv_hash.size-1
      canv.SaveAs OutName+')'
    else
      canv.SaveAs OutName
    end
  end
end
