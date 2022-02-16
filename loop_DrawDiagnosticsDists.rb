#!/usr/bin/env ruby
# run DrawDiagnosticsDists.C for a list of files

# list of datasets
datasets = [
  "mc.inbending.bg45",
  "mc.outbending.bg40",
  "mc.outbending.bg50",
  "rga.inbending.fa18",
  "rga.inbending.sp19",
  "rga.outbending.fa18",
  "rgb.inbending.sp19",
  "rgb.inbending.sp20",
  "rgb.outbending.fa19",
]

# build DrawDiagnosticsDists calls
cmds = []
datasets.each do |dataset|
  args = [
    "plots.testvertex.#{dataset}.root",
    "diagplots/#{dataset}",
  ]
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  cmds << "root -b -q 'DrawDiagnosticsDists.C(#{args.join(',')})'"
end
puts cmds

# execution
cmds.each{ |cmd| system cmd }
