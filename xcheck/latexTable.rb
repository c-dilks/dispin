#!/usr/bin/env ruby
# convert `yieldCheck.md` to latex table lines
#
# FIXME: yieldCheck.md has been renamed to README.md, and reformatted...
#

File.readlines('yieldCheck.md').each do |line|
  puts line if line.match?(/^#/)
  if line.match?(/^nAllCuts/)
    c = line.split[1].to_f # chris's yield
    t = line.split[2].to_f # timothy's yield
    ave = [c,t].sum / 2.0 # mean
    diff = 100.0 * (c-t).abs / ave # percent difference
    puts "   $#{c.to_i}$  &  $#{t.to_i}$  &  $%.2f\\%%$  \\\\ \\hline\n\n" % [diff]
  end
end
