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
    enum particleTypeEnum { kElectron, kHadron, nParticleType };

    // method which applies the cuts
    // - needs particleType
    // - sets booleans listed below
    void ApplyCuts(int runnum_, int pid_);
    // - booleans (so far just one)
    Bool_t fiduCut;

    // total EC energy (for sampling fraction)
    Float_t GetECenergy() {
      return part_Cal_PCAL_energy[0] +
             part_Cal_ECIN_energy[0] +
             part_Cal_ECOUT_energy[0];
    };

    
    // Stefan's methods (the integer argument is the particle number, 
    // just use 0; see below); these are called by ApplyCuts
    int determineSector(int i);
    /// use the following cut for inbending hadrons:
    /// j is the index of teh particle. Th variable part_pid is needed to assign the correct cut
    /// The inebdning / outbending flags (see top of thsi document) have to be set to assign the correct cut
    bool DC_fiducial_cut_theta_phi(int j, int region, int part_pid);
    /// use the following cut for inebnding electrons and all outbending particles
    /// j is the index of teh particle. Th variable part_pid is needed to assign the correct cut
    /// The inebdning / outbending flags (see top of thsi document) have to be set to assign the correct cut
    bool DC_fiducial_cut_XY(int j, int region, int part_pid);


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
    bool inbending, outbending;
    int part_DC_sector[1];
    const double Pival = 3.1415927;

    bool fcutEle[nReg];
    bool fcutHad[nReg];

  
  ClassDef(FiducialCuts,1);
};

#endif
