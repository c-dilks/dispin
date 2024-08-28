#!/usr/bin/env ruby

require 'optparse'
require 'ostruct'
require 'fileutils'

def print_log(name, val)
  puts name.rjust(30) + " = #{val}"
end

DataExt = 'root'

# user options
@args = OpenStruct.new
@args.inputs     = nil
@args.output_dir = nil
@args.prefix     = nil
@args.num_merge  = nil
OptionParser.new do |o|
  o.banner = "USAGE: #{$0} [OPTIONS]..."
  o.separator ''
  o.separator 'REQUIRED OPTIONS:'
  o.on('-i', '--input INPUTS', 'input directory or file glob;', 'surround file glob in quotes') { |a| @args.inputs = a }
  o.on('-o', '--output OUTPUT_DIR', 'output directory') { |a| @args.output_dir = a }
  o.on('-p', '--prefix OUTPUT_PREFIX', 'output filename prefix;', "output filenames will be [OUTPUT_DIR]/[OUTPUT_PREFIX]_#####.#{DataExt}") { |a| @args.prefix = a }
  o.on('-n', '--num NUM_FILES', 'number of files per output merged file') { |a| @args.num_merge = a.to_i }
  o.separator ''
  o.separator 'OPTIONAL OPTIONS:'
  o.on_tail('-h', '--help', 'show this message') do
    puts o
    exit
  end
end.parse! ARGV.empty? ? ['--help'] : ARGV

# check required options
if [@args.inputs, @args.output_dir, @args.prefix, @args.num_merge].include? nil
  raise 'missing required option(s;) re-run with "--help" for guidance.'
end
raise 'option "--num" must be greater than zero' unless @args.num_merge > 0

# glob inputs
input_glob = File.expand_path @args.inputs
input_glob = File.join input_glob, "*.#{DataExt}" if File.directory? input_glob
print_log 'input glob', input_glob
print_log 'output dir', @args.output_dir
print_log 'output prefix', @args.prefix
print_log 'num files per output', @args.num_merge

# chunks
input_files = Dir.glob input_glob
raise "no input files found with glob '#{input_glob}'" if input_files.empty?
input_chunks = input_files.each_slice(@args.num_merge).to_a
print_log 'num input files', input_files.size
print_log 'num output files', input_chunks.size
raise 'option "--num" >= num input files, therefore there is nothing to do' if input_chunks.size == 1

# build commands
puts "="*50
merge_cmds = input_chunks.each_with_index.map do |input_chunk, chunk_num|
  [
    'hadd',
    File.join(@args.output_dir, "#{@args.prefix}_#{chunk_num.to_s.rjust(5, '0')}.#{DataExt}"),
    *input_chunk,
  ]
end

# execute
FileUtils.mkdir_p @args.output_dir
sbatch_args = {
  'job-name'      => "merge_osg_files___#{@args.prefix}",
  'account'       => 'clas12',
  'partition'     => 'production',
  'mem-per-cpu'   => 500,
  'time'          => '1:00:00',
  'ntasks'        => 1,
  'cpus-per-task' => 1,
}.map{ |opt, val| "--#{opt}=#{val.to_s}" }

slurm_cmds = merge_cmds.each_with_index.map do |merge_cmd, job_num|
  log_name = "/farm_out/%u/%x_#{job_num.to_s.rjust(5, '0')}"
  [
    'sbatch',
    *sbatch_args,
    "--output=#{log_name}.out",
    "--error=#{log_name}.err",
    "--wrap='#{merge_cmd.join ' '}'",
  ].join ' '
end
slurm_cmds.each do |cmd| system cmd end
