// Stefan Diehl's Fiducial Volume Cuts
// version: 04/16/2020
// changes for `dispin` analysis code:
// -> encapsulated in a class for implementation in `dispin`
// -> changes to Stefan's code are kept to a minimum
//

#ifndef FiducialCuts_
#define FiducialCuts_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <cmath>

// ROOT
#include "TObject.h"


class FiducialCuts : public TObject {
  public:
    
    FiducialCuts(int particleType_);
    ~FiducialCuts(){};

    // enumerators
    enum levelEnum { cutTight, cutMedium, cutLoose, nLevel }; // cut levels
    enum particleTypeEnum { kElectron, kHadron, nParticleType };

    // method which applies the cuts
    // - needs particleType
    // - sets booleans listed below
    void ApplyCuts(int runnum_, int pid_);
    // - booleans (so far just one)
    Bool_t fiduCut[nLevel];

    // total EC energy (for sampling fraction)
    Float_t GetECenergy() {
      return part_Cal_PCAL_energy[0] +
             part_Cal_ECIN_energy[0] +
             part_Cal_ECOUT_energy[0];
    };

    
    // Stefan's methods (the integer argument is the particle number, 
    // just use 0; see below); these are called by ApplyCuts
    int determineSector(int i);
    /// 1.1 A homgenous cut for all sectors which does not consider variations
    // of the sampling fraction. This cut assumes, that for the electron ID
    // only a proper cluster formation is required, which is given, if the
    // center of the cluster has enough distance to the edges.  
    // - loose: cuts 1 bar from the outer side (4.5 cm)
    // - medium: cuts 2 bars from the outer side (9.0 cm)
    // - tight: cuts 3 bars from the outer side (13.5 cm)
    // - applies to: electrons, photons
    bool EC_hit_position_fiducial_cut_homogeneous(int j);
    /// 1.2 PCAL fiducial cut based on fitted sampling fraction; cut
    // criterium is drop of sampling fraction
    // - loose: The mean value of E/p is allowed to drop to 2 RMS of the
    //   distribution 
    // - medium: The mean value of E/p is allowed to drop to 1.5 RMS of the
    //   distribution 
    // - tight: The mean value of E/p is allowed to drop to 1 RMS of the
    //   distribution 
    // - applies to: photons
    bool EC_hit_position_fiducial_cut(int j);
    /// 2.1 DC fiducial cuts for the 3 regions based on the drop of the count
    // rate at the edeges of the DC
    // - loose: count rate can drop to 35 %
    // - medium: count rate can drop to 50 %
    // - tight: count rate can drop to 65 %
    // - applies to: electrons only
    bool DC_hit_position_counts_fiducial_cut(int j, int region);
    /// 2.2 DC fiducial cuts for the 3 regions based on the chi2/NDF value of
    // the tracks
    // - loose,medium,tight: no difference
    // - applies to: hadrons (also electrons, but with some uncertainty)
    bool DC_fiducial_cut_chi2(int j, int region, int part_pid);


    static const int nReg = 3; // number of DC regions

    // variables are arrays of length 1
    // - Stefan's original code is meant to run at the event level; 
    // - this class that encapsulates his code is meant to run at the particle level
    // - these arrays store info for each particle, and since only one is needed, the size
    //   is limited to 1
    // - preserving the array structure, rather than just defining floats, minimizes 
    //   changes to Stefan's code
    
    Float_t part_Cal_PCAL_found[1];
    Float_t part_Cal_PCAL_sector[1];
    Float_t part_Cal_PCAL_energy[1];
    Float_t part_Cal_PCAL_time[1];
    Float_t part_Cal_PCAL_path[1];
    Float_t part_Cal_PCAL_x[1];
    Float_t part_Cal_PCAL_y[1];
    Float_t part_Cal_PCAL_z[1];
    Float_t part_Cal_PCAL_lu[1];
    Float_t part_Cal_PCAL_lv[1];
    Float_t part_Cal_PCAL_lw[1];

    Float_t part_Cal_ECIN_found[1];
    Float_t part_Cal_ECIN_sector[1];
    Float_t part_Cal_ECIN_energy[1];
    Float_t part_Cal_ECIN_time[1];
    Float_t part_Cal_ECIN_path[1];
    Float_t part_Cal_ECIN_x[1];
    Float_t part_Cal_ECIN_y[1];
    Float_t part_Cal_ECIN_z[1];
    Float_t part_Cal_ECIN_lu[1];
    Float_t part_Cal_ECIN_lv[1];
    Float_t part_Cal_ECIN_lw[1];

    Float_t part_Cal_ECOUT_found[1];
    Float_t part_Cal_ECOUT_sector[1];
    Float_t part_Cal_ECOUT_energy[1];
    Float_t part_Cal_ECOUT_time[1];
    Float_t part_Cal_ECOUT_path[1];
    Float_t part_Cal_ECOUT_x[1];
    Float_t part_Cal_ECOUT_y[1];
    Float_t part_Cal_ECOUT_z[1];
    Float_t part_Cal_ECOUT_lu[1];
    Float_t part_Cal_ECOUT_lv[1];
    Float_t part_Cal_ECOUT_lw[1];

    Float_t part_DC_Track_found[1];
    Float_t part_DC_Track_chi2[1];
    Float_t part_DC_Track_NDF[1];
    Float_t part_DC_Track_status[1];

    Float_t part_DC_Traj_found[1];
    Float_t part_DC_c1x[1];
    Float_t part_DC_c1y[1];
    Float_t part_DC_c1z[1];
    Float_t part_DC_c2x[1];
    Float_t part_DC_c2y[1];
    Float_t part_DC_c2z[1];
    Float_t part_DC_c3x[1];
    Float_t part_DC_c3y[1];
    Float_t part_DC_c3z[1];

  private:
    
    int particleType;
    bool inbending, outbending, tight, medium, loose;
    int part_DC_sector[1];
    const double Pival = 3.1415927;

    bool fcutEleEC;
    bool fcutEleDC1[nReg];
    bool fcutEleDC2[nReg];
    bool fcutHadDC[nReg];

  
  ClassDef(FiducialCuts,1);
};

#endif
