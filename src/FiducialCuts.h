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

// ROOT
#include "TObject.h"


class FiducialCuts : public TObject {
  public:
    
    FiducialCuts(int particleType_);
    ~FiducialCuts(){};

    // method which applies the cuts
    // - needs particleType
    // - sets booleans listed below
    void ApplyCuts(int runnum_, int pid_);
    // - booleans (so far just one)
    Bool_t fiduCut[nLevel];

    
    // Stefan's methods (the integer argument is the particle number, 
    // just use 0; see below); these are called by ApplyCuts
    int determineSector(int i);
    bool EC_hit_position_fiducial_cut_homogeneous(int j); // electrons, photons
    bool EC_hit_position_fiducial_cut(int j); // photons
    bool DC_hit_position_counts_fiducial_cut(int j, int region); // electrons only
    bool DC_fiducial_cut_chi2(int j, int region, int part_pid); // electrons, hadrons


    enum levelEnum { cutTight, cutMedium, cutLoose, nLevel }; // cut levels
    enum particleTypeEnum { kElectron, kHadron, nParticleType };
    const int nReg = 3; // number of DC regions

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
