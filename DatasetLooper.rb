#!/usr/bin/env ruby
# common methods for looping through all our datasets

require 'pp'

class DatasetLooper

  # list of datasets
  DatasetList = [
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

  #####################################
  # constructor

  def initialize
    # make @subsetList
    @subsetList = DatasetList.map do |dataset|
      dataset+".subset"
    end
    # make @allsetList
    @allsetList = DatasetList.map do |dataset|
      dataset.gsub(/bending\..*$/,"bending.all")
    end.uniq
  end

  #####################################
  # print
 
  def printLists
    puts "\nDatasetList = "
    pp DatasetList
    puts "\nSubsetList = "
    pp @subsetList
    puts "\nAllsetList = "
    pp @allsetList
  end

  #####################################
  # general loopers; better to use 'sugar' below

  # loop over datases in `arr`, executing a block for each
  def loopSets(arr,&block)
    arr.each do |dataset|
      yield dataset
    end
  end

  # loop over pairs of datasets in `arr`, executing a block for each
  def loopPairs(arr)
    arr.combination(2).to_a.each do |pair|
      yield pair
    end
  end

  #####################################
  # specific loopers (sugar)

  # Datasets
  def loopDatasets(&block)
    loopSets(DatasetList,&block)
  end
  def loopDatasetPairs(&block)
    loopPairs(DatasetList,&block)
  end

  # Subsets
  def loopSubsets(&block)
    loopSets(@subsetList,&block)
  end
  def loopSubsetPairs(&block)
    loopPairs(@subsetList,&block)
  end
    
  # Allsets
  def loopAllsets(&block)
    loopSets(@allsetList,&block)
  end
  def loopAllsetPairs(&block)
    loopPairs(@allsetList,&block)
  end

  #####################################
  # tests
  def test
    puts '='*30
    loopSubsets{ |set| puts "#{set}" }
    puts '='*30
    loopAllsetPairs{ |set| puts "#{set}" }
  end

  #####################################

  attr_accessor :subsetList
  attr_accessor :allsetList

end
