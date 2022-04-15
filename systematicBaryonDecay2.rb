#!/usr/bin/env ruby
# step 2: calculate systematic uncertainty from baryonic decays

require 'pry'
require 'matplotlib/pyplot'
require 'pycall/import'
include PyCall::Import
pyimport 'ROOT', as: :root

inFileN = 'baryonTrees/tree.DIS_pass1_1003_1008.hipo.root'
inFile = root.TFile.new(inFileN,'READ')
tr = inFile.Get('tree_0')
tr.Print
inFile.Close
