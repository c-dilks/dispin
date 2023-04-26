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
| VAR          | CHRIS  | TIMOTHY | GREGORY |
| ---          | ---    | ---     | ---     |
| nNoCuts      | 121354 | 121044  |         |
| nAllCuts     | 8283   | 8295    |         |
| nCutDIS      | 117659 | 117351  |         |
| nCutDihadron | 97598  | ?       |         |
| nCutHelicity | 120433 | ?       |         |
| nCutFiducial | 104361 | 103840  |         |
| nCutPID      | 17403  | 17323   |         |
| nCutVertex   | 100834 | 100529  |         |
| nCutFR       | 58514  | 58341   |         |

## 5403 (RGA inbending)
| VAR      | CHRIS | TIMOTHY | GREGORY |
| ---      | ---   | ---     | ---     |
| nAllCuts | 7031  | 7045    |         |

## 5424 (RGA outbending)
| VAR          | CHRIS  | TIMOTHY | GREGORY |
| ---          | ---    | ---     | ---     |
| nAllCuts     | 39899  | 40107   |         |
| nNoCuts      | 699793 | 699742  |         |
| nCutDIS      | 686811 | 686439  |         |
| nCutDihadron | 541674 | 541704  |         |
| nCutHelicity | 693886 | ?       |         |
| nCutFiducial | 555088 | 555283  |         |
| nCutPID      | 94251  | 93921   |         |
| nCutVertex   | 580271 | 579867  |         |
| nCutFR       | 373031 | 372412  |         |

## 6356 RGB Spring 2019 Inbending
| VAR          | CHRIS  | TIMOTHY | TIMOTHY-FILE                   |
| ---          | ---    | ---     | ---                            |
| nAllCuts     | 25686  | 26213   | `6356/6356_all_cuts.txt`       |
| nNoCuts      | 894417 | 891562  | `6356/6356_assumed_cuts.txt`   |
| nCutDIS      | 686802 | 683905  | `6356/6356_sidis_cuts.txt`     |
| nCutDihadron | 740830 | 737308  | `6356/6356_exclusive_cuts.txt` |
| nCutHelicity | 886176 | ?       | ?                              |
| nCutFiducial | 738015 | 734608  | `6356/6356_fiducial_cuts.txt`  |
| nCutPID      | 54673  | 54195   | `6356/6356_pid_cuts.txt`       |
| nCutVertex   | 716208 | 713345  | `6356/6356_vertex_cuts.txt`    |
| nCutFR       | 377431 | 375957  | `6356/6356_CFR_cuts.txt`       |

## 11171 RGB Fall 2019 Outbending
| VAR          | CHRIS  | TIMOTHY | TIMOTHY-FILE                     |
| ---          | ---    | ---     | ---                              |
| nAllCuts     | 30255  | 34278   | `11171/11171_all_cuts.txt`       |
| nNoCuts      | 839714 | 829788  | `11171/11171_assumed_cuts.txt`   |
| nCutDIS      | 672160 | 662728  | `11171/11171_sidis_cuts.txt`     |
| nCutDihadron | 649086 | 641373  | `11171/11171_exclusive_cuts.txt` |
| nCutHelicity | 819980 | ?       | ?                                |
| nCutFiducial | 666613 | 658771  | `11171/11171_fiducial_cuts.txt`  |
| nCutPID      | 78609  | 77230   | `11171/11171_pid_cuts.txt`       |
| nCutVertex   | 663390 | 654753  | `11171/11171_vertex_cuts.txt`    |
| nCutFR       | 404113 | 398379  | `11171/11171_CFR_cuts.txt`       |

## 11364 RGB Spring 2020 Inbending
| VAR          | CHRIS  | TIMOTHY | TIMOTHY-FILE                     |
| ---          | ---    | ---     | ---                              |
| nAllCuts     | 25055  | 25602   | `11364/11364_all_cuts.txt`       |
| nNoCuts      | 567737 | 566812  | `11364/11364_assumed_cuts.txt`   |
| nCutDIS      | 433713 | 432548  | `11364/11364_sidis_cuts.txt`     |
| nCutDihadron | 467031 | 465454  | `11364/11364_exclusive_cuts.txt` |
| nCutHelicity | 562061 | ?       | ?                                |
| nCutFiducial | 460973 | 459036  | `11364/11364_fiducial_cuts.txt`  |
| nCutPID      | 54306  | 53906   | `11364/11364_pid_cuts.txt`       |
| nCutVertex   | 466300 | 465079  | `11364/11364_vertex_cuts.txt`    |
| nCutFR       | 248719 | 248222  | `11364/11364_CFR_cuts.txt`       |

