# 6356 RGB Spring 2019 Inbending
VAR           CHRIS   TIMOTHY  TIMOTHY_FILE
nAllCuts      25686   26213    6356/6356_all_cuts.txt
nNoCuts       894417  704168   6356/6356_assumed_cuts.txt
nCutDIS       686802  683905   6356/6356_sidis_cuts.txt
nCutDihadron  740830  737308   6356/6356_exclusive_cuts.txt
nCutHelicity  886176
nCutFiducial  738015  734608   6356/6356_fiducial_cuts.txt
nCutPID       54673   54195    6356/6356_pid_cuts.txt
nCutVertex    716208  713345   6356/6356_vertex_cuts.txt
nCutFR        377431  375957   6356/6356_CFR_cuts.txt

# 11171 RGB Fall 2019 Outbending
VAR           CHRIS   TIMOTHY  TIMOTHY_FILE
nAllCuts      30255   34278    11171/11171_all_cuts.txt
nNoCuts       839714  829788   11171/11171_assumed_cuts.txt
nCutDIS       672160  662728   11171/11171_sidis_cuts.txt
nCutDihadron  649086  641373   11171/11171_exclusive_cuts.txt
nCutHelicity  819980
nCutFiducial  666613  658771   11171/11171_fiducial_cuts.txt
nCutPID       78609   77230    11171/11171_pid_cuts.txt
nCutVertex    663390  654753   11171/11171_vertex_cuts.txt
nCutFR        404113  398379   11171/11171_CFR_cuts.txt

# 11364 RGB Spring 2020 Inbending
VAR           CHRIS   TIMOTHY  TIMOTHY_FILE
nAllCuts      25055   25602    11364/11364_all_cuts.txt
nNoCuts       567737  566812   11364/11364_assumed_cuts.txt
nCutDIS       433713  432548   11364/11364_sidis_cuts.txt
nCutDihadron  467031  465454   11364/11364_exclusive_cuts.txt
nCutHelicity  562061
nCutFiducial  460973  459036   11364/11364_fiducial_cuts.txt
nCutPID       54306   53906    11364/11364_pid_cuts.txt
nCutVertex    466300  465079   11364/11364_vertex_cuts.txt
nCutFR        248719  248222   11364/11364_CFR_cuts.txt


# PID cross check ###################
## RUN 6356
### chris
nCutPID      54673
nCutTest (cut1 && cut6 && cut9) = 670107
nCutTest (cut7 && cut10)        = 173368
nCutTest (cut8 && cut11)        = 587739
nCutTest (cut4 && cut5)         = 493250
### timothy
    667986 6356/6356_pid_1_6_9_cuts.txt
    172974 6356/6356_pid_7_10_cuts.txt
    584079 6356/6356_pid_8_11_cuts.txt
    490488 6356/6356_pid_4_5_cuts.txt

## RUN 11171
### chris
nCutPID      78609
nCutTest (cut1 && cut6 && cut9) = 617203
nCutTest (cut7 && cut10)        = 139731
nCutTest (cut8 && cut11)        = 528999
nCutTest (cut4 && cut5)         = 804297
### timothy
    610107 11171/11171_pid_1_6_9_cuts.txt
    138721 11171/11171_pid_7_10_cuts.txt
    519667 11171/11171_pid_8_11_cuts.txt
    794383 11171/11171_pid_4_5_cuts.txt

## RUN 11364
### chris
nCutPID      54306
nCutTest (cut1 && cut6 && cut9) = 431328
nCutTest (cut7 && cut10)        = 109727
nCutTest (cut8 && cut11)        = 393663
nCutTest (cut4 && cut5)         = 484338
### timothy
    430624 11364/11364_pid_1_6_9_cuts.txt
    109770 11364/11364_pid_7_10_cuts.txt
    391386 11364/11364_pid_8_11_cuts.txt
    482341 11364/11364_pid_4_5_cuts.txt

