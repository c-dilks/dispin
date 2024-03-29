#!/usr/bin/env ruby
# generate latex code for plots from finalPlot.rb
# - for transversity 2022

require './DatasetLooper.rb'
require 'awesome_print'

looper = DatasetLooper.new :pm

outN = 'prelimTransversity2022_img.tex'
out = File.open(outN,'w')
out.puts "%%%%% generated by latex_finalPlots.rb %%%%%"
[2,3].each do |twist|
  looper.binHash.each do |ivType,hash|
    blList = [0]
    blList << 1 if ivType>9
    blList.each do |bl|
      png = "rga.pm.bibending.all.#{hash[:name]}.minuit_asym_minuit_BL#{bl}_sc#{twist}.png"
      caption = "Twist #{twist} $\\alu$ measurements, in bins of #{hash[:xTitle].gsub(/ \[.*$/,'')}"
      if blList.length==1
        caption += "."
      else
        caption += ", for $M_h<0.63~\\gev$." if bl==0
        caption += ", for $M_h>0.63~\\gev$." if bl==1
      end
      width = (twist==2) ? 0.7 : 1.0
      caption += " Light red circles are for the proton target, and dark blue squares are for the deuteron target."
      out.puts "\\begin{figure}[p]"
      out.puts "\\centering"
      out.puts "\\includegraphics[width=#{width}\\textwidth]{img/prelimTransversity2022/#{png}}"
      out.puts "\\caption{#{caption}}"
      out.puts "\\label{fig:asym:#{hash[:name]}:bl#{bl}:tw#{twist}}"
      out.puts "\\end{figure}"
    end
  end
end
out.puts "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% end generated latex"
puts "produced #{outN}"
out.close
