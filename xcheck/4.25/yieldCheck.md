# 6356 RGB Spring 2019 Inbending
VAR          CHRIS  TIMOTHY
nAllCuts     25686  40127 <-----
nNoCuts      894417 891562
nCutDIS      686802 683905
nCutDihadron 740830 737308
nCutHelicity 886176
nCutFiducial 738015 734608
nCutPID      54673  88117 <-----
nCutVertex   716208 713345
nCutFR       377431 375957


# 11171 RGB Fall 2019 Outbending
VAR          CHRIS  TIMOTHY
nAllCuts     32500  30720
nNoCuts      839714 829788
nCutDIS      672160 662728
nCutDihadron 649086 641373
nCutHelicity 819980
nCutFiducial 703739 658771 <-----
nCutPID      78609  77991
nCutVertex   663390 654753
nCutFR       404113 398379


# 11364 RGB Spring 2020 Inbending
VAR          CHRIS  TIMOTHY
nAllCuts     25055  25746
nNoCuts      567737 566812
nCutDIS      433713 432548
nCutDihadron 467031 465454
nCutHelicity 562061
nCutFiducial 460973 459036
nCutPID      54306  57478 <-----
nCutVertex   466300 465079
nCutFR       248719 248222


# PID cross check
## RUN 6356
nAllCuts     25686
nNoCuts      894417
nCutDIS      686802
nCutDihadron 740830
nCutHelicity 886176
nCutFiducial 738015
nCutPID      54673
nCutVertex   716208
nCutFR       377431
nCutTest (cut1 && cut6 && cut9) = 670107
nCutTest (cut7 && cut10)        = 173368
nCutTest (cut8 && cut11)        = 587739
nCutTest (cut4 && cut5)         = 493250

## RUN 11171
nAllCuts     32500
nNoCuts      839714
nCutDIS      672160
nCutDihadron 649086
nCutHelicity 819980
nCutFiducial 703739
nCutPID      78609
nCutVertex   663390
nCutFR       404113
nCutTest (cut1 && cut6 && cut9) = 617203
nCutTest (cut7 && cut10)        = 139731
nCutTest (cut8 && cut11)        = 528999
nCutTest (cut4 && cut5)         = 804297

## RUN 11364
nAllCuts     25055
nNoCuts      567737
nCutDIS      433713
nCutDihadron 467031
nCutHelicity 562061
nCutFiducial 460973
nCutPID      54306
nCutVertex   466300
nCutFR       248719
nCutTest (cut1 && cut6 && cut9) = 431328
nCutTest (cut7 && cut10)        = 109727
nCutTest (cut8 && cut11)        = 393663
nCutTest (cut4 && cut5)         = 484338
