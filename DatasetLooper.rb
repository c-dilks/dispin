# common methods for looping through the dataset lists
# - see `testDatasetLooper.rb` for usage guidance

require 'pp'
require 'colorize'

class DatasetLooper

  #####################################
  # CONSTANTS

  # binning schemes and related options: `ivType` => { :bins=>[bn0,bn1,bn2], :option=>value, ... }
  BinHash = {
    1  => { :bins=>[6],   :name=>'x',   :xTitle=>'$x$',         :xTranslation=>0.006      },
    2  => { :bins=>[6],   :name=>'m',   :xTitle=>'$M_h$ [GeV]', :xTranslation=>0.020      },
    32 => { :bins=>[3,2], :name=>'zm',  :xTitle=>'$z$',         :blTitle=>'$M_h$ __BL__', :xTranslation=>0.010 },
    42 => { :bins=>[3,2], :name=>'ptm', :xTitle=>'$p_T$ [GeV]', :blTitle=>'$M_h$ __BL__', :xTranslation=>0.015 },
  }

  # dihadron types
  # - if used 'truncation' for yield balancing when buidling bibending sets, :useTruncation should be true
  Dihadrons = {
    :pm   => { :title=>'pi+pi-', :latex=>'$\pi^+\pi^-$', :pairType=>'0x34', :useTruncation=>false },
    :p0   => { :title=>'pi+pi0', :latex=>'$\pi^+\pi^0$', :pairType=>'0x3b', :useTruncation=>true  },
    :m0   => { :title=>'pi0pi-', :latex=>'$\pi^0\pi^-$', :pairType=>'0xb4', :useTruncation=>true  },
    :none => { :title=>'',       :latex=>'',             :pairType=>'0',    :useTruncation=>false },
  }
 
  #####################################
  # construction

  # constructor
  def initialize(dihadronTok=:none)

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

    # add dihadron token to each dataset (if specified)
    # - useful for creating an instance for a specific dihadron pairType
    begin
      @pairType      = Dihadrons[dihadronTok][:pairType]
      @useTruncation = Dihadrons[dihadronTok][:useTruncation]
    rescue
      $stderr.puts "\nERROR: unknown dihadron in DatasetLooper\n\n"
      dihadronTok = :none
      retry
    end
    unless dihadronTok==:none
      @datasetList.map! do |dataset|
        dataset.split('.').insert(1,dihadronTok.to_s).join('.')
      end
    end

    # make @subsetList
    @subsetList = @datasetList.map do |dataset|
      dataset+".subset"
    end

    # make @allsetList
    @allsetList = @datasetList.map do |dataset|
      dataset.gsub(/bending\..*$/,"bending.all")
    end.uniq

    # add bibending sets to @allsetList only
    @allsetList = @allsetList.map do |set|
      if set.split('.').include?('outbending')
        [ set, set.sub('outbending','bibending') ]
      else
        set
      end
    end.flatten

    # if we used truncation, we have `mca` and `mcb` instead of `mc`
    if @useTruncation
      @allsetList = @allsetList.map do |set|
        if set.split('.').include?('mc') and set.split('.').include?('bibending')
          [ set.sub('mc','mca'), set.sub('mc','mcb') ]
        else
          set
        end
      end.flatten
    end

  end

  # list of lists that have been defined above
  ListOfLists = [
    :datasetList,
    :subsetList,
    :allsetList,
  ]

  # generate accessors
  ListOfLists.each{ |sym| attr_accessor sym }
  attr_accessor :pairType
  attr_accessor :useTruncation


  #####################################
  # list filters for data or MC

  def noop(list) list end
  def onlyMC(list) list.find_all{ |set| set.match? /^mc/ } end
  def onlyData(list) list.reject{ |set| set.match? /^mc/ } end
  def onlyRGA(list) list.find_all{ |set| set.match? /^rga\./ } end
  def onlyRGB(list) list.find_all{ |set| set.match? /^rgb\./ } end
  def onlyInbending(list) list.find_all{ |set| set.match? /\.inbending\./ } end
  def onlyOutbending(list) list.find_all{ |set| set.match? /\.outbending\./ } end
  def onlyBibending(list) list.find_all{ |set| set.match? /\.bibending\./ } end

  ListOfFilters = [
    :noop,
    :onlyMC,
    :onlyData,
    :onlyRGA,
    :onlyRGB,
    :onlyInbending,
    :onlyOutbending,
    :onlyBibending,
  ]


  #####################################
  # looping method generators

  # generator of a method for looping through elements of `list`
  # - example: `gen_loopSets :datasetList` generates method `datasetListLoop(&block)`, which will call
  #   `block` for each element in `datasetList` (and if you do not pass a block when calling the method,
  #   it will simply return the list)
  # - specify a filter, to filter the loop
  #   - example: `gen_loopSets(:datasetList,:onlyMC)` generates method `datasetListLoopOnlyMC(&block)`,
  #     which will call `block` for each element in `onlyMC(datasetList)`
  def self.gen_loopSets(list,filter=:noop)
    methodName = "#{list.to_s}Loop"
    methodName += filter.to_s.sub(/^./,&:upcase) if filter!=:noop # append filter name, capitalizing first letter
    define_method(methodName) do |&block|
      filteredList = send( filter.to_s, instance_variable_get("@#{list}") ) # filter `list`
      filteredList.each do |dataset|
        block.call dataset unless block.nil?
      end
    end
  end

  # generator of a method for looping through pairs of elements of `list`
  # - analagous to `gen_loopSets`
  # - example: method `datasetListPairs(&block)` calls `block` for each pair in `datasetList`
  # - example: method `datasetListPairsOnlyMC(&block)` calls `block` for each pair `onlyMC(datsetList)`
  def self.gen_loopPairs(list,filter=:noop)
    methodName = "#{list.to_s}Pairs"
    methodName += filter.to_s.sub(/^./,&:upcase) if filter!=:noop # append filter name, capitalizing first letter
    define_method(methodName) do |&block|
      filteredList = send( filter.to_s, instance_variable_get("@#{list}") ) # filter `list`
      filteredList.combination(2).to_a.each do |pair|
        block.call pair unless block.nil?
      end
    end
  end

  # generate the looping methods for each list in ListOfLists and for each filter in ListOfFilters
  ListOfLists.each do |list|
    ListOfFilters.each do |filter|
      gen_loopSets(list, filter)
      gen_loopPairs(list, filter)
    end
  end


  ######################################
  ## utilities

  # print all the lists
  def printLists
    ListOfLists.each do |list|
      puts "\n#{list.to_s} = "
      pp instance_variable_get("@#{list}")
      # pp onlyMC   instance_variable_get("@#{list}")
      # pp onlyData instance_variable_get("@#{list}")
    end
  end

  # print available dihadrons
  def self.printDihadrons
    puts "\n  DIHADRON can be:"
    DatasetLooper::Dihadrons.each do |k,v|
      puts "  - #{k.to_s.colorize(:light_red)}: #{v[:title]}" unless k==:none
    end
    puts ""
  end

  # convert dataset name to title
  def self.datasetTitle(dataset)
    toks = dataset.split('.').map do |tok|
      tok = tok.upcase if tok.match? /rga|rgb|mc/
      tok = tok.gsub(/bg/,"(BG-merged ") + " nA)" if tok.match? /^bg/
      tok.gsub!(/MCA/,"MC")
      tok.gsub!(/MCB/,"MC")
      tok.gsub!(/^fa/,"Fall 20")
      tok.gsub!(/^sp/,"Spring 20")
      tok.gsub!(/^wi/,"Winter 20")
      tok.gsub!(/bibending/,"combined inbending+outbending")
      Dihadrons.each do |k,v| tok.gsub!(/#{k.to_s}/,v[:latex]) end
      tok
    end
    toks.delete("subset")
    toks.delete("diph")
    toks.delete("all")
    return toks.append("data set").join(' ')
  end

  # find dataset in `searchList` that has matching torus polarity; only returns 
  # the first match, so this is best used when there will be only one possible match
  def matchByTorus(dataset,searchList)
    torus = dataset.split('.').find{ |tok| tok.include?"bending" }
    results = searchList.find_all{ |set| set.include? torus }
    ### if truncation was used, match `mca` or `mcb`, rather than `mc`
    if @useTruncation
      if torus=='bibending' and results.find{ |result| result.include?"mc" }
        if dataset.include?"rga"
          return results.find{ |result| result.include?"mca" }
        elsif dataset.include?"rgb"
          return results.find{ |result| result.include?"mcb" }
        else
          $stderr.puts "ERROR in DatasetLooper.matchByTorus (see class)"
          return results.first
        end
      end
    end
    return results.first
  end

  # get catTree file basename (does not include ".root" or ".idx.root" extensions
  def self.catTreeBaseName(dataset)
    prefix = dataset.split('.').find{|tok|tok.match?(/^mc/)} ? 'catTreeMC' : 'catTreeData'
    return "#{prefix}.#{dataset}"
  end

end
