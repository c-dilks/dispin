#!/usr/bin/env ruby
# make montages (png grids) from files in `diagcomp`
# - see CompareDiagnosticsDists.C
# - requires imagemagick montage

# arguments
if ARGV.length < 1
  $stderr.puts "USAGE: #{$0} [diagcomp directory]"
  exit 2
end
dir = ARGV[0]

# check if dir exists
if ! Dir.exist?(dir)
  $stderr.puts "ERROR: directory #{dir} does not exist"
  exit 1
end


# prepend each `plotlist` elem with each `partlist` elem
def prependParts(plotlist)
  partlist = ["piPlus","piMinus"]
  partlist.map{ |had| plotlist.map{ |plt| had+plt } }.flatten
end
  
# plot lists for each montage
montages = Hash.new
montages["dis"] = [
  "Q2Dist",
  "XDist",
  "WDist",
  "YDist",
]
montages["dihadron"] = [
  "MhDist",
  "ZpairDist",
  "PhPerpDist",
  "MmissDist",
  "PhiHDist",
  "PhiRDist",
  "thetaDist",
]
montages["hadron"] = prependParts [ 
  "hadPDist",
  "hadPhiHDist",
  "hadThetaDist",
  "hadXFDist",
]
montages["elePID1"] = [
  "elePDist",
  "eleThetaDist",
  "elePCALenDist",
  "eleECINenDist",
  "eleECOUTenDist",
]
montages["elePID2"] = [
  "eleSampFracDist",
  "eleSFvsP",
  "eleDiagonalSFdist",
  "eleECALvsPCALedep",
]
montages["hadPID"] = prependParts [
  "hadChi2pidDist",
  "hadChi2pidVsP",
]
montages["vertex"] = [
  "piPlushadEleVzDiffDist",
  "piMinushadEleVzDiffDist",
  "eleVzDist",
]


# montage settings
settings = [
  # "-tile 2x0",
  "-geometry 800x400x0x0",
  # "-geometry 1600x800x0x0",
]

# execution
count = 0
montages.each do |mname,plist|
  count += plist.length
  pnglist = plist.map{ |pname| dir+"/"+pname+".png" }
  outname = dir+"/montage_"+mname+".png"
  cmd = "montage #{settings.join " "} #{pnglist.join " "} #{outname}"
  puts "montaging #{mname}..."
  system cmd
end
numfigs = `ls #{dir}/*.png | grep -v "montage_" | wc -l`.strip
puts "-> montaged #{count} out of #{numfigs} figures"

# view
system "sxiv #{dir}/montage*.png" if system "which sxiv"

