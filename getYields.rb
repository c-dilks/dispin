#!/usr/bin/env ruby
# get yields in each catTree

require 'awesome_print'
require 'pycall/import'
r = PyCall.import_module 'ROOT'

yieldHash = Hash.new
Dir.glob("catTrees/catTree*.idx.root").each do |treeFileName|
  treeFile = r.TFile.new treeFileName
  tree = treeFile.Get "tree"
  datasetName = treeFileName.split('.').select{ |tok| tok.match?(/bending|mc|rg/) }.join ' '
  ap [treeFileName,datasetName]
  yieldHash[datasetName] = tree.GetEntries
  treeFile.Close
end
puts "\n\nYIELDS:"
ap yieldHash
