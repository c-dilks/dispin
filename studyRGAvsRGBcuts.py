#!/usr/bin/env python3
# compare all histograms from RGA and RGB, for several cut criteria sets
#
# EXPECTED FILE NAME FORMAT:
# plots.[data_set_name].[cut_set_name].root

import re, subprocess, shlex

# list of the TWO data set names to expect; these two will be compared
dataSetList = [
    "rga.fa18",
    "rgb.sp19",
]

# list of cut set names to expect:
cutSetList = [
    "cut_all_enabled",
    "cut_all_disabled",
    "cut_dihadron_disabled",
    "cut_DIS_disabled",
    "cut_fragmentation_region_disabled",
    "cut_PID_disabled",
    "cut_vertex_disabled",
    "cut_missing_mass_disabled",
    "cut_fiducial_volume_disabled",
]

# loop through data and cuts
for cutSet in cutSetList:
    print(f'cutSet = {cutSet}')

    # format file names
    def fileNameFormat(ds): return f'plots.{ds}.{cutSet}.root'
    fileNameList = list(map(fileNameFormat, dataSetList))
    print(f'fileNameList = {fileNameList}')

    # format data set titles
    def dataTitleFormat(ds):
        ds += "."+cutSet
        ds = re.sub('rga','RGA',ds)
        ds = re.sub('rgb','RGB',ds)
        ds = re.sub('fa','Fall',ds)
        ds = re.sub('sp','Spring',ds)
        ds = re.sub('cut_','',ds)
        ds = re.sub('enabled','cuts_enabled',ds)
        ds = re.sub('disabled','cuts_disabled',ds)
        ds = re.sub('(\.|_)',' ',ds)
        return ds
    dataTitleList = list(map(dataTitleFormat, dataSetList))
    print(f'dataTitleList = {dataTitleList}')

    # output directory name
    dirName=f'diagcomp.{cutSet}'

    # build root command
    cmd = 'root -b -q CompareDiagnosticsDists.C'
    cmd += '\'('
    cmd += f'"{fileNameList[0]}","{fileNameList[1]}"'
    cmd += f',"{dirName}"'
    cmd += f',"{dataTitleList[0]}","{dataTitleList[1]}"'
    cmd += ')\''
    print(f'CMD = {cmd}')

    subprocess.call( shlex.split(cmd) )
