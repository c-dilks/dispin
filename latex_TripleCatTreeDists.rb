#!/usr/bin/env ruby
# generate latex image markup for TripleCatTreeDists

vars = [
  "Mh",
  "X",
  "Z",
  "Q2",
  "PhPerp",
  "PhiH",
  "PhiR",
  # "PhiD",
  "Theta",
  "Depol2",
  "Depol3",
  "Mmiss",
  "XF",
  # "DYsgn",
]

titles = {
  "Mh"     => "$M_{h}$",
  "X"      => "$x$",
  "Z"      => "$z$",
  "Q2"     => "$Q^{2}$",
  "PhPerp" => "$\\pt$",
  "PhiH"   => "$\\phih$",
  "PhiR"   => "$\\phir$",
  "PhiD"   => "$\\Delta\\phi$",
  "Theta"  => "$\\theta$",
  "Depol2" => "$K_2$",
  "Depol3" => "$K_3$",
  "Mmiss"  => "$M_X$",
  "XF"     => "$x_F$",
  "DYsgn"  => "$\\Delta Y$",
}

tori = [
  "inbending",
  "outbending",
  "bibending",
]

outN = "triple.tex"
out = File.open(outN,"w")
out.puts "%%%%% latex generated by #{$0} %%%%%"
cnt = 0
vars.each do |var|
  out.puts "\\begin{figure}[h]"
  out.puts "\\centering"
  tori.each do |torus|
    out.puts "\\includegraphics[width=0.32\\textwidth]{img/triple/#{torus}/#{var}.png}"
  end
  caption = "Comparisons of #{titles[var]} for inbending (left), outbending (middle), and the combined inbending and outbending data sets (right)."
  caption += " The light red upward triangles are for RGA data, the dark blue downward triangles are for RGB data, and the green circles are for MC reconstructed data."
  caption += " The distributions are normalized by their respective dihadron yields."
  out.puts "\\caption{#{caption}}"
  out.puts "\\label{fig:triple:#{var}}"
  out.puts "\\end{figure}"
  out.puts "\\clearpage" if cnt.odd?
  cnt += 1
end
out.puts "%%%%% end generated latex %%%%%"
out.close
puts "wrote #{outN}"
