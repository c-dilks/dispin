# Event Selection Criteria
Organized into various sets, defining the following yields for each cut set:

### nNoCuts
- baseline cuts
  - PDG
    - at least one electron and (pi+,pi-) dihadron, using Event Builder PID hypothesis
  - from `skimDihadrons.groovy`, applied in this following order:
    - `qa.OkForAsymmetry(runnum,evnum))`
    - choose max-E electron
    - `! (eleDIS.status <= -3000 || eleDIS.status > -2000)`: electron in FD trigger
    - `! (hadA.status>=4000 && hadA.status<5000) || (hadB.status>=4000 && hadB.status<5000) )`: reject hadrons which are only in CD, since they will fail FD fiducial cuts
  - skim-file cuts (`nSidis` train; tighter than legacy `skim4` trains):
    - electron in FD
    - Q2 > 1 GeV2
    - W > 2 GeV
    - electron p > 2 GeV
    - -25 < electronVertexZ < 20

### nCutDIS
- Q2 > 1.0
- W > 2.0
- y < 0.8

### nCutDihadron
- Zpair < 0.95
- Missing mass cut: Mmiss > 1.5 (for (pi+,pi-), may differ for other channels)

### nCutHelicity
- electron helicity is `+1` or `-1` (defined, as opposed to `0`, undefined)

### nCutFiducial
- fiducial cuts (see `FiducialCuts::ApplyCuts`)

### nCutPID
- electron PID
  - 5 < eleTheta < 35 degrees
  - eleP > 2
  - elePCALen > 0.07
  - sampling fraction cuts:
    - diagonal cut on PCAL and ECIN sampling fraction correlation
    - (mu,std) cut of sampling fraction vs. p
- hadron PID (applied to both hadrons of the dihadron)
  - 5 < hadTheta < 35
  - hadP > 1.25
  - refined `chi2pid` cut

### nCutVertex
- electron z-vertex
  - inbending:  `-8.0  < eleVertexZ && eleVertexZ < 3.0`
  - outbending: `-10.0 < eleVertexZ && eleVertexZ < 2.5`
- `| had_Vz - ele_Vz |` cut
  - must be less than 20 for each hadron of the dihadron

### nCutFR
- xF>0, for each hadron of the dihadron

### nAllCuts
- `AND` of all the above cuts


# Cross Check Yields
These are the tables of yields we have cross checked

## 5036 (RGA inbending)
|  VAR           |  CHRIS   |  TIMOTHY  |  GREGORY  |  DIFF(CHRIS,TIMOTHY)  |  DIFF(CHRIS,GREGORY)  |
|  ---           |  ---     |  ---      |  ---      |  ---                  |  ---                  |
|  nNoCuts       |  121354  |  121044   |  121331   |  0.2555%              |  0.0190%              |
|  nAllCuts      |  8283    |  8295     |  8283     |  0.1449%              |  0.0000%              |
|  nCutDIS       |  117659  |  117351   |  117639   |  0.2618%              |  0.0170%              |
|  nCutDihadron  |  97598   |  ?        |  97596    |  ?                    |  0.0020%              |
|  nCutHelicity  |  120433  |  ?        |  120410   |  ?                    |  0.0191%              |
|  nCutFiducial  |  104361  |  103840   |  104340   |  0.4992%              |  0.0201%              |
|  nCutPID       |  17403   |  17323    |  17403    |  0.4597%              |  0.0000%              |
|  nCutVertex    |  100834  |  100529   |  100818   |  0.3025%              |  0.0159%              |
|  nCutFR        |  58514   |  58341    |  58501    |  0.2957%              |  0.0222%              |

## 5403 (RGA inbending)
|  VAR       |  CHRIS  |  TIMOTHY  |  GREGORY  |  DIFF(CHRIS,TIMOTHY)  |  DIFF(CHRIS,GREGORY)  |
|  ---       |  ---    |  ---      |  ---      |  ---                  |  ---                  |
|  nAllCuts  |  7031   |  7045     |  7031     |  0.1991%              |  0.0000%              |

## 5424 (RGA outbending)
|  VAR           |  CHRIS   |  TIMOTHY  |  GREGORY  |  DIFF(CHRIS,TIMOTHY)  |  DIFF(CHRIS,GREGORY)  |
|  ---           |  ---     |  ---      |  ---      |  ---                  |  ---                  |
|  nAllCuts      |  39899   |  40107    |  39898    |  0.5213%              |  0.0025%              |
|  nNoCuts       |  699793  |  699742   |  700064   |  0.0073%              |  0.0387%              |
|  nCutDIS       |  686811  |  686439   |  686832   |  0.0542%              |  0.0031%              |
|  nCutDihadron  |  541674  |  541704   |  541807   |  0.0055%              |  0.0246%              |
|  nCutHelicity  |  693886  |  ?        |  694157   |  ?                    |  0.0391%              |
|  nCutFiducial  |  555088  |  555283   |  555297   |  0.0351%              |  0.0377%              |
|  nCutPID       |  94251   |  93921    |  94256    |  0.3501%              |  0.0053%              |
|  nCutVertex    |  580271  |  579867   |  580459   |  0.0696%              |  0.0324%              |
|  nCutFR        |  373031  |  372412   |  373148   |  0.1659%              |  0.0314%              |

## 6741 (RGA inbending)
|  VAR           |  CHRIS   |  GREGORY  |  DIFF(CHRIS,GREGORY)  |
|  ---           |  ---     |  ---      |  ---                  |
|  nAllCuts      |  39404   |  41781    |  6.0324%              |
|  nNoCuts       |  690382  |  689858   |  0.0759%              |
|  nCutDIS       |  683549  |  669208   |  2.0980%              |
|  nCutDihadron  |  538448  |  582240   |  8.1330%              |
|  nCutHelicity  |  684599  |  684096   |  0.0735%              |
|  nCutFiducial  |  561445  |  561021   |  0.0755%              |
|  nCutPID       |  80758   |  80693    |  0.0805%              |
|  nCutVertex    |  561880  |  561450   |  0.0765%              |
|  nCutFR        |  338516  |  318039   |  6.0490%              |

## 6356 RGB Spring 2019 Inbending

**NOTE**: `nCutPID` differs between CHRIS and GREGORY, since (1) our sampling fraction vs. p cuts differ, and (2), we
need to update them anyway; our final `nAllCuts` difference is reasonable, so this update is likely negligible.

|  VAR           |  CHRIS   |  TIMOTHY  |  GREGORY  |  TIMOTHY-FILE                    |  DIFF(CHRIS,TIMOTHY)  |  DIFF(CHRIS,GREGORY)  |
|  ---           |  ---     |  ---      |  ---      |  ---                             |  ---                  |  ---                  |
|  nAllCuts      |  25686   |  26213    |  25686    |  `6356/6356_all_cuts.txt`        |  2.0517%              |  0.0000%              |
|  nNoCuts       |  894417  |  891562   |  894417   |  `6356/6356_assumed_cuts.txt`    |  0.3192%              |  0.0000%              |
|  nCutDIS       |  686802  |  683905   |  686801   |  `6356/6356_sidis_cuts.txt`      |  0.4218%              |  0.0001%              |
|  nCutDihadron  |  740830  |  737308   |  740826   |  `6356/6356_exclusive_cuts.txt`  |  0.4754%              |  0.0005%              |
|  nCutHelicity  |  886176  |  ?        |  886176   |  ?                               |  ?                    |  0.0000%              |
|  nCutFiducial  |  738015  |  734608   |  738015   |  `6356/6356_fiducial_cuts.txt`   |  0.4616%              |  0.0000%              |
|  nCutPID       |  54673   |  54195    |  72120    |  `6356/6356_pid_cuts.txt`        |  0.8743%              |  31.9115%             |
|  nCutVertex    |  716208  |  713345   |  716208   |  `6356/6356_vertex_cuts.txt`     |  0.3997%              |  0.0000%              |
|  nCutFR        |  377431  |  375957   |  377428   |  `6356/6356_CFR_cuts.txt`        |  0.3905%              |  0.0008%              |

## 11171 RGB Fall 2019 Outbending

**NOTE**: `nCutPID` differs between CHRIS and GREGORY, since (1) our sampling fraction vs. p cuts differ, and (2), we
need to update them anyway; our final `nAllCuts` difference is reasonable, so this update is likely negligible.

|  VAR           |  CHRIS   |  TIMOTHY  |  GREGORY  |  TIMOTHY-FILE                      |  DIFF(CHRIS,TIMOTHY)  |  DIFF(CHRIS,GREGORY)  |
|  ---           |  ---     |  ---      |  ---      |  ---                               |  ---                  |  ---                  |
|  nAllCuts      |  30255   |  34278    |  30254    |  `11171/11171_all_cuts.txt`        |  13.2970%             |  0.0033%              |
|  nNoCuts       |  839714  |  829788   |  839714   |  `11171/11171_assumed_cuts.txt`    |  1.1821%              |  0.0000%              |
|  nCutDIS       |  672160  |  662728   |  672154   |  `11171/11171_sidis_cuts.txt`      |  1.4032%              |  0.0009%              |
|  nCutDihadron  |  649086  |  641373   |  649074   |  `11171/11171_exclusive_cuts.txt`  |  1.1883%              |  0.0018%              |
|  nCutHelicity  |  819980  |  ?        |  819980   |  ?                                 |  ?                    |  0.0000%              |
|  nCutFiducial  |  666613  |  658771   |  666613   |  `11171/11171_fiducial_cuts.txt`   |  1.1764%              |  0.0000%              |
|  nCutPID       |  78609   |  77230    |  93936    |  `11171/11171_pid_cuts.txt`        |  1.7543%              |  19.4978%             |
|  nCutVertex    |  663390  |  654753   |  663390   |  `11171/11171_vertex_cuts.txt`     |  1.3019%              |  0.0000%              |
|  nCutFR        |  404113  |  398379   |  404109   |  `11171/11171_CFR_cuts.txt`        |  1.4189%              |  0.0010%              |


## 11364 RGB Spring 2020 Inbending

**NOTE**: `nCutPID` differs between CHRIS and GREGORY, since (1) our sampling fraction vs. p cuts differ, and (2), we
need to update them anyway; our final `nAllCuts` difference is reasonable, so this update is likely negligible.

|  VAR           |  CHRIS   |  TIMOTHY  |  GREGORY  |  TIMOTHY-FILE                      |  DIFF(CHRIS,TIMOTHY)  |  DIFF(CHRIS,GREGORY)  |
|  ---           |  ---     |  ---      |  ---      |  ---                               |  ---                  |  ---                  |
|  nAllCuts      |  25055   |  25602    |  25055    |  `11364/11364_all_cuts.txt`        |  2.1832%              |  0.0000%              |
|  nNoCuts       |  567737  |  566812   |  567737   |  `11364/11364_assumed_cuts.txt`    |  0.1629%              |  0.0000%              |
|  nCutDIS       |  433713  |  432548   |  433711   |  `11364/11364_sidis_cuts.txt`      |  0.2686%              |  0.0005%              |
|  nCutDihadron  |  467031  |  465454   |  467029   |  `11364/11364_exclusive_cuts.txt`  |  0.3377%              |  0.0004%              |
|  nCutHelicity  |  562061  |  ?        |  562061   |  ?                                 |  ?                    |  0.0000%              |
|  nCutFiducial  |  460973  |  459036   |  460973   |  `11364/11364_fiducial_cuts.txt`   |  0.4202%              |  0.0000%              |
|  nCutPID       |  54306   |  53906    |  72906    |  `11364/11364_pid_cuts.txt`        |  0.7366%              |  34.2504%             |
|  nCutVertex    |  466300  |  465079   |  466300   |  `11364/11364_vertex_cuts.txt`     |  0.2618%              |  0.0000%              |
|  nCutFR        |  248719  |  248222   |  248712   |  `11364/11364_CFR_cuts.txt`        |  0.1998%              |  0.0028%              |
