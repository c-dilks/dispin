#!/usr/bin/env ruby
# common methods for looping through the dataset lists

require 'pp'

class DatasetLooper

  #####################################
  # construction
 
  # list of lists that we will define below
  ListOfLists = [
    "datasetList",
    "subsetList",
    "allsetList",
  ]

  # constructor
  def initialize

    # list of datasets
    @datasetList = [
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

    # make @subsetList
    @subsetList = @datasetList.map do |dataset|
      dataset+".subset"
    end

    # make @allsetList
    @allsetList = @datasetList.map do |dataset|
      dataset.gsub(/bending\..*$/,"bending.all")
    end.uniq

  end

  # generate list accessors
  ListOfLists.map(&:to_sym).each{ |sym| attr_accessor sym }


  #####################################
  # looping method generators

  # generator of a method for looping through elements of `list`
  # - example: `gen_loopSets :datasetList` generates method `datasetListLoop(&block)`, 
  #   which will call `block` for each element in `datasetList`
  def self.gen_loopSets(list)
    define_method("#{list}Loop") do |&block|
      instance_variable_get("@#{list}").each do |dataset|
        block.call dataset
      end
    end
  end

  # generator of a method for looping through pairs of elements of `list`
  # - analagous to `gen_loopSets`
  # - example: method `datasetListPairs(&block)` calls `block` for each pair
  def self.gen_loopPairs(list)
    define_method("#{list}Pairs") do |&block|
      instance_variable_get("@#{list}").combination(2).to_a.each do |pair|
        block.call pair
      end
    end
  end

  # generate the looping methods for each list in ListOfLists
  ListOfLists.map(&:to_sym).each do |sym|
    gen_loopSets sym
    gen_loopPairs sym
  end


  ######################################
  ## utilities

  # print all the lists
  def printLists
    ListOfLists.each do |list|
      puts "\n#{list} = "
      pp instance_variable_get("@#{list}")
    end
  end

  # convert dataset name to title
  def datasetTitle(dataset)
    toks = dataset.split('.').map do |tok|
      tok = tok.upcase if tok.match? /rga|rgb|mc/
      tok = tok.gsub(/bg/,"(BG-merged ") + " nA)" if tok.match? /^bg/
      tok.gsub!(/^fa/,"Fall 20")
      tok.gsub!(/^sp/,"Spring 20")
      tok.gsub!(/^wi/,"Winter 20")
      tok
    end
    toks.delete("subset")
    toks.delete("diph")
    toks.delete("all")
    return toks.append("data set").join(' ')
  end

end
