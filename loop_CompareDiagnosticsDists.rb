#!/usr/bin/env ruby
# run CompareDiagnosticsDists.C for various pairs of files

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

# pair each dataset
pairs = datasets.combination(2).to_a
pairs.each{ |pair| puts "#{pair}" }
puts "="*50

# build CompareDiagnosticsDists calls
cmds = []
pairs.each do |pair|
  args = [
    *pair.map{ |dataset| "plots.testvertex.#{dataset}.root" },
    "diagcomps/#{pair.join("__vs__")}",
    *pair,
  ]
  args.map!{ |arg| if arg.class==String then "\"#{arg}\"" else arg end } # add quotes around strings
  cmds << "root -b -q 'CompareDiagnosticsDists.C(#{args.join(',')})'"
end
puts cmds

# execution
cmds.each{ |cmd| system cmd }
