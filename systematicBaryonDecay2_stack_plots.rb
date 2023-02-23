#!/usr/bin/env ruby
# stack the transparent images from 12 and 22 GeV on top of each other,
# then convert to PDFs
out = 'baryonTrees.stacked'
system "mkdir -p #{out}"
plots = {
  12 => Dir.glob('baryonTrees.12gev.rgcMC/*.png'),
  22 => Dir.glob('baryonTrees.22gev/*.png'),
}
plots[12].zip(plots[22]).each do |a,b|
  c = out + '/' + File.basename(a).sub(/.*_/,'frac.')
  system "convert #{a} #{b} -composite #{c}"
  system "convert #{c} #{c.sub /png$/, 'pdf'}"
end
