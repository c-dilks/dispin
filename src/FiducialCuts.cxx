// Stefan Diehl's Fiducial Volume Cuts
// version: 04/16/2020

#include "FiducialCuts.h"

ClassImp(FiducialCuts)

// constructor; give it a particleType (see particleTypeEnum), which
// will be needed in ApplyCuts()
FiducialCuts::FiducialCuts(int particleType_) {

  // set default torus setting to inbending
  inbending=true; outbending=false;

  // set default cut level to tight
  tight=true; medium=false; loose=false;

  // set particleType
  particleType = particleType_;

  // initialize public cut booleans
  for(int l=0; l<nLevel; l++) {
    fiduCut[l] = false;
  };

};


// apply the cuts; public booleans listed in header file will be set
void FiducialCuts::ApplyCuts(int runnum_, int pid_) {

  // use runnumber to determine torus setting
  if(runnum_>=5032 && runnum_<=5419) {
    inbending=true; outbending=false;
  } else if(runnum_>=5422 && runnum_<=5666) {
    inbending=false; outbending=true;
  } else if(runnum_==11) { // MC
    inbending=true; outbending=false;
  } else {
    fprintf(stderr,"ERROR: FiducialCuts does not know whether this run\n");
    fprintf(stderr,"       is inbending or outbending; setting to inbending\n");
    inbending=true; outbending=false;
  };

  // determine sector (requires part_DC_Traj_found to be true)
  part_DC_sector[0] = this->determineSector(0);

  // loop over cut levels
  for(int cutLevel=0; cutLevel<nLevel; cutLevel++) {

    // set cut level booleans
    tight=false; medium=false; loose=false;
    switch(cutLevel) {
      case cutTight: tight=true; break;
      case cutMedium: medium=true; break;
      case cutLoose: loose=true; break;
    };

    // apply cuts, depending on the value of particleType
    // - electron
    if(particleType == kElectron) {
      // -- cut 1.1
      fcutEleEC = (part_Cal_PCAL_found[0]>0) ? 
        this->EC_hit_position_fiducial_cut_homogeneous(0) : false;
      for(int r=0; r<nReg; r++) {
        if(part_DC_Traj_found[0]>0) {
          // -- cut 2.1 // DISABLED
          //fcutEleDC1[r] = this->DC_hit_position_counts_fiducial_cut(0,r+1);
          fcutEleDC1[r] = true;
          // -- cut 2.2
          fcutEleDC2[r] = this->DC_fiducial_cut_chi2(0,r+1,11);
        } else {
          fcutEleDC1[r] = false;
          fcutEleDC2[r] = false;
        };
      };
      fiduCut[cutLevel] = fcutEleEC && 
                          fcutEleDC1[0] && fcutEleDC1[1] && fcutEleDC1[2] && 
                          fcutEleDC2[0] && fcutEleDC2[1] && fcutEleDC2[2];
    }
    // - hadrons
    else if(particleType == kHadron) {
      for(int r=0; r<nReg; r++) {
        // -- cut 2.2
        fcutHadDC[r] = (part_DC_Traj_found[0]>0) ? 
          this->DC_fiducial_cut_chi2(0,r+1,pid_) : false;
      };
      fiduCut[cutLevel] = fcutHadDC[0] && fcutHadDC[1] && fcutHadDC[2];
    }
    else {
      fprintf(stderr,"ERROR: FiducialCuts bad particleType\n");
      return;
    };
  };

};



////////////////////////////////
////////////////////////////////
//
// BEGIN STEFAN'S METHODS
//
////////////////////////////////
////////////////////////////////


int FiducialCuts::determineSector(int i){
  double phi = 180 / Pival * atan2(part_DC_c2y[i] / sqrt(pow(part_DC_c2x[i], 2) + pow(part_DC_c2y[i], 2) + pow(part_DC_c2z[i], 2)), part_DC_c2x[i] / sqrt(pow(part_DC_c2x[i], 2) 
               + pow(part_DC_c2y[i], 2) + pow(part_DC_c2z[i], 2)));

  if(phi < 30 && phi >= -30){        return 1;}
  else if(phi < 90 && phi >= 30){    return 2;}
  else if(phi < 150 && phi >= 90){   return 3;}
  else if(phi >= 150 || phi < -150){ return 4;}
  else if(phi < -90 && phi >= -150){ return 5;}
  else if(phi < -30 && phi >= -90){  return 6;}

  return 0;
}


/// //////////////////////////////////////////////////////////////////////////////////////////////
/// 1. Fiducial cuts for the PCAL (required for electrons and photons)
///     

/// 1.1 A homgenous cut for all sectors which does not consider variations of the sampling fraction
///     This cut assumes, that for the electron ID only a proper cluster formation is required, which
///     is given, if the center of the cluster has enough distance to the edges.
///     loose: cuts 1 bar from the outer side (4.5 cm)
///     medium: cuts 2 bars from the outer side (9.0 cm)
///     tight: cuts 3 bars from the outer side (13.5 cm)


bool FiducialCuts::EC_hit_position_fiducial_cut_homogeneous(int j){

// Cut using the natural directions of the scintillator bars/ fibers:

  double u = part_Cal_PCAL_lu[j];
  double v = part_Cal_PCAL_lv[j];
  double w = part_Cal_PCAL_lw[j];
   
  /// v + w is going from the side to the back end of the PCAL, u is going from side to side
  /// 1 scintillator bar is 4.5 cm wide. In the outer regions (back) double bars are used.

  ///////////////////////////////////////////////////////////////////
  /// inbending:
  // 
  double min_u_tight_inb[] = {19.0,  19.0,  19.0,  19.0,  19.0,  19.0 };
  double min_u_med_inb[]   = {14.0,  14.0,  14.0,  14.0,  14.0,  14.0 };
  double min_u_loose_inb[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_u_tight_inb[] = {398, 398, 398, 398, 398, 398}; 
  double max_u_med_inb[]   = {408, 408, 408, 408, 408, 408}; 
  double max_u_loose_inb[] = {420, 420, 420, 420, 420, 420}; 
  // 
  double min_v_tight_inb[] = {19.0,  19.0,  19.0,  19.0,  19.0,  19.0 };
  double min_v_med_inb[]   = {14.0,  14.0,  14.0,  14.0,  14.0,  14.0 };
  double min_v_loose_inb[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  //
  double max_v_tight_inb[] = {400, 400, 400, 400, 400, 400};
  double max_v_med_inb[]   = {400, 400, 400, 400, 400, 400};
  double max_v_loose_inb[] = {400, 400, 400, 400, 400, 400};
  //
  double min_w_tight_inb[] = {19.0,  19.0,  19.0,  19.0,  19.0,  19.0 };
  double min_w_med_inb[]   = {14.0,  14.0,  14.0,  14.0,  14.0,  14.0 };
  double min_w_loose_inb[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_w_tight_inb[] = {400, 400, 400, 400, 400, 400};
  double max_w_med_inb[]   = {400, 400, 400, 400, 400, 400};
  double max_w_loose_inb[] = {400, 400, 400, 400, 400, 400};

  ///////////////////////////////////////////////////////////////////////
  /// outbending (not adjusted up to now, same as inbending!):
  // 
  double min_u_tight_out[] = {19.0,  19.0,  19.0,  19.0,  19.0,  19.0 };
  double min_u_med_out[]   = {14.0,  14.0,  14.0,  14.0,  14.0,  14.0 };
  double min_u_loose_out[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_u_tight_out[] = {398, 398, 398, 398, 398, 398}; 
  double max_u_med_out[]   = {408, 408, 408, 408, 408, 408}; 
  double max_u_loose_out[] = {420, 420, 420, 420, 420, 420}; 
  // 
  double min_v_tight_out[] = {19.0,  19.0,  19.0,  19.0,  19.0,  19.0 };
  double min_v_med_out[]   = {14.0,  14.0,  14.0,  14.0,  14.0,  14.0 };
  double min_v_loose_out[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_v_tight_out[] = {400, 400, 400, 400, 400, 400};
  double max_v_med_out[]   = {400, 400, 400, 400, 400, 400};
  double max_v_loose_out[] = {400, 400, 400, 400, 400, 400};
  //
  double min_w_tight_out[] = {19.0,  19.0,  19.0,  19.0,  19.0,  19.0 };
  double min_w_med_out[]   = {14.0,  14.0,  14.0,  14.0,  14.0,  14.0 };
  double min_w_loose_out[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  //
  double max_w_tight_out[] = {400, 400, 400, 400, 400, 400};
  double max_w_med_out[]   = {400, 400, 400, 400, 400, 400};
  double max_w_loose_out[] = {400, 400, 400, 400, 400, 400};

  //////////////////////////////////////////////////////////////

  double min_u = 0; double max_u = 0; double min_v = 0; double max_v = 0; double min_w = 0; double max_w = 0;  

  for(Int_t k = 0; k < 6; k++){  
    if(part_Cal_PCAL_sector[j]-1 == k && inbending == true){
      if(tight == true){
        min_u = min_u_tight_inb[k]; max_u = max_u_tight_inb[k];
        min_v = min_v_tight_inb[k]; max_v = max_v_tight_inb[k];
        min_w = min_w_tight_inb[k]; max_w = max_w_tight_inb[k];
      }
      if(medium == true){
        min_u = min_u_med_inb[k]; max_u = max_u_med_inb[k];
        min_v = min_v_med_inb[k]; max_v = max_v_med_inb[k];
        min_w = min_w_med_inb[k]; max_w = max_w_med_inb[k];
      }
      if(loose == true){
        min_u = min_u_loose_inb[k]; max_u = max_u_loose_inb[k];
        min_v = min_v_loose_inb[k]; max_v = max_v_loose_inb[k];
        min_w = min_w_loose_inb[k]; max_w = max_w_loose_inb[k];
      }
    }
    if(part_Cal_PCAL_sector[j]-1 == k && outbending == true){
      if(tight == true){
        min_u = min_u_tight_out[k]; max_u = max_u_tight_out[k];
        min_v = min_v_tight_out[k]; max_v = max_v_tight_out[k];
        min_w = min_w_tight_out[k]; max_w = max_w_tight_out[k];
      }
      if(medium == true){
        min_u = min_u_med_out[k]; max_u = max_u_med_out[k];
        min_v = min_v_med_out[k]; max_v = max_v_med_out[k];
        min_w = min_w_med_out[k]; max_w = max_w_med_out[k];
      }
      if(loose == true){
        min_u = min_u_loose_out[k]; max_u = max_u_loose_out[k];
        min_v = min_v_loose_out[k]; max_v = max_v_loose_out[k];
        min_w = min_w_loose_out[k]; max_w = max_w_loose_out[k];
      }
    }
  }

  if(v > min_v && v < max_v && w > min_w && w < max_w) return true;
  else return false;
}



/// 1.2 PCAL fiducial cut based on fitted sampling fraction (I woudl recommend this cut for photons)
///     For this cut, the cut criterium is teh drop of the sampling fraction 
///     loose: The mean value of E/p is allowed to drop to 2 RMS of the distribution 
///     medium: The mean value of E/p is allowed to drop to 1.5 RMS of the distribution 
///     tight: The mean value of E/p is allowed to drop to 1 RMS of the distribution 

bool FiducialCuts::EC_hit_position_fiducial_cut(int j){


  double theta_PCAL = 180/Pival * acos(part_Cal_PCAL_z[j]/sqrt(pow(part_Cal_PCAL_x[j], 2) + pow(part_Cal_PCAL_y[j], 2) + pow(part_Cal_PCAL_z[j],2)));
  double phi_PCAL_raw = 180/Pival * atan2(part_Cal_PCAL_y[j]/sqrt(pow(part_Cal_PCAL_x[j], 2) + pow(part_Cal_PCAL_y[j], 2) + pow(part_Cal_PCAL_z[j],2)), 
                                            part_Cal_PCAL_x[j]/sqrt(pow(part_Cal_PCAL_x[j], 2) + pow(part_Cal_PCAL_y[j], 2) + pow(part_Cal_PCAL_z[j],2)));

  double phi_PCAL = 0;
  if(part_Cal_PCAL_sector[j] == 1) phi_PCAL = phi_PCAL_raw;
  if(part_Cal_PCAL_sector[j] == 2) phi_PCAL = phi_PCAL_raw - 60;
  if(part_Cal_PCAL_sector[j] == 3) phi_PCAL = phi_PCAL_raw - 120;
  if(part_Cal_PCAL_sector[j] == 4 && phi_PCAL_raw > 0) phi_PCAL = phi_PCAL_raw - 180;
  if(part_Cal_PCAL_sector[j] == 4 && phi_PCAL_raw < 0) phi_PCAL = phi_PCAL_raw + 180;
  if(part_Cal_PCAL_sector[j] == 5) phi_PCAL = phi_PCAL_raw + 120;
  if(part_Cal_PCAL_sector[j] == 6) phi_PCAL = phi_PCAL_raw + 60;

  // 2 sigma (inb adjusted):

 double par_0_min_2sigma[] = {13.771, 25.639, 28.4616, 34.2333, 41.777, 18.041};
 double par_1_min_2sigma[] = {-14.3952, -27.4437, -29.5074, -40.9268, -33.056, -19.1539};
 double par_2_min_2sigma[] = {-0.0579567, 2.12184, 2.7033, 4.72287, 1.80765, 0.648548};
 double par_3_min_2sigma[] = {0.0190146, -0.0315046, -0.0523381, -0.0919215, -0.00542624, 0.00836905};
 double par_4_min_2sigma[] = {-0.000222315, 0.000243574, 0.000453275, 0.000771317, -0.000150837, -0.000208545};
 double par_0_max_2sigma[] = {-19.2009, -16.4848, -47.8295, -24.0029, -25.096, -19.2967};
 double par_1_max_2sigma[] = {19.3148, 11.0556, 47.4188, 23.6525, 19.3032, 19.9627};
 double par_2_max_2sigma[] = {-0.66582, 1.35067, -5.54184, -1.20742, 0.0744728, -0.714339};
 double par_3_max_2sigma[] = {-0.00592537, -0.0623736, 0.123022, 0.00276304, -0.0334298, -0.0077081};
 double par_4_max_2sigma[] = {0.000187643, 0.000759023, -0.00120291, 0.000128345, 0.000486216, 0.000224336};

  double par_0_min_2sigma_out[] = {64.895, 69.4726, 37.4087, 57.2897, 36.5138, 66.2482};
  double par_1_min_2sigma_out[] = {-49.7813, -52.7634, -28.7868, -44.398, -27.2841, -50.6646};
  double par_2_min_2sigma_out[] = {3.79889, 4.06934, 1.52759, 3.26992, 1.25399, 3.8817};
  double par_3_min_2sigma_out[] = {-0.0389919, -0.0418169, -0.0120817, -0.0329207, -0.00772183, -0.0398076};
  double par_4_min_2sigma_out[] = {0,0,0,0,0,0};
  double par_0_max_2sigma_out[] = {-58.8252, -42.022, -35.6843, -62.0889, -25.7336, -62.4078};
  double par_1_max_2sigma_out[] = {46.3788, 32.7105, 29.2649, 48.8274, 21.4091, 49.5489};
  double par_2_max_2sigma_out[] = {-3.47241, -2.00905, -1.83462, -3.7453, -0.830711, -3.86809};
  double par_3_max_2sigma_out[] = {0.0350037, 0.018892, 0.0188074, 0.0381712, 0.00422423, 0.040186};
  double par_4_max_2sigma_out[] = {0,0,0,0,0,0};

  // 1.5 sigma (inb adjusted, 4 min replaced by 2 min!):

  double par_0_min_15sigma[] = {25.2996, 19.3705, 59.5003, 19.3705, 26.9823, 21.8217};
  double par_1_min_15sigma[] = {-26.1158, -19.5271, -55.9639, -19.5271, -22.4489, -23.0262};
  double par_2_min_15sigma[] = {2.09145, 0.72118, 6.4372, 0.72118, 0.890449, 1.48469};
  double par_3_min_15sigma[] = {-0.041483, 0.00293465, -0.13059, 0.00293465, -0.000774943, -0.0190315};
  double par_4_min_15sigma[] = {0.000456261, -0.000109323, 0.00115246, -0.000109323, -7.12074e-05, 0.000125463};
  double par_0_max_15sigma[] = {-26.7425, -15.9009, -47.556, -21.5038, -33.9197, -24.0325};
  double par_1_max_15sigma[] = {26.004, 10.5989, 41.6295, 21.1734, 31.1811, 23.3122};
  double par_2_max_15sigma[] = {-1.76638, 1.10168, -3.66934, -0.969572, -2.51229, -1.23308};
  double par_3_max_15sigma[] = {0.0227414, -0.0455311, 0.0493171, 0.00373945, 0.0443308, 0.00762314};
  double par_4_max_15sigma[] = {-0.000111102, 0.000503536, -0.000151053, 5.22425e-05, -0.000403627, 4.24553e-05};

  double par_0_min_15sigma_out[] = {57.0314, 70.411, 74.9683, 53.9146, 44.7614, 64.012};
  double par_1_min_15sigma_out[] = {-43.0803, -53.3163, -59.0842, -41.6436, -35.2193, -48.8726};
  double par_2_min_15sigma_out[] = {2.99452, 4.11397, 5.27234, 2.96164, 2.44681, 3.67536};
  double par_3_min_15sigma_out[] = {-0.0287862, -0.042257, -0.0638554, -0.0293148, -0.0264986, -0.0372214};
  double par_4_min_15sigma_out[] = {0,0,0,0,0,0};
  double par_0_max_15sigma_out[] = {-52.0537, -48.3703, -94.6197, -54.6123, -79.9164, -55.3222};
  double par_1_max_15sigma_out[] = {40.7573, 38.333, 73.5425, 42.9251, 64.9277, 42.8186};
  double par_2_max_15sigma_out[] = {-2.8105, -2.83403, -6.88649, -3.08431, -6.18973, -2.99337};
  double par_3_max_15sigma_out[] = {0.0266371, 0.0318955, 0.0851474, 0.0301575, 0.0787016, 0.0286132};
  double par_4_max_15sigma_out[] = {0,0,0,0,0,0};

  // 1 sigma (inb adjusted):

  double par_0_min_1sigma[] = {34.1128, 26.6064, 65.3241, 44.0344, 54.5539, 25.7356};
  double par_1_min_1sigma[] = {-30.7179, -26.3373, -58.7761, -35.918, -47.3194, -25.3968};
  double par_2_min_1sigma[] = {2.31272, 1.85141, 6.48495, 2.34733, 4.58872, 1.76128};
  double par_3_min_1sigma[] = {-0.0351384, -0.023687, -0.121042, -0.0170119, -0.0778135, -0.0243688};
  double par_4_min_1sigma[] = {0.000262438, 0.000120765, 0.000936822, -7.66933e-05, 0.000539922, 0.000156061};
  double par_0_max_1sigma[] = {-31.6364, -28.7094, -35.6017, -30.1334, -61.5491, -30.9496};
  double par_1_max_1sigma[] = {27.253, 20.8471, 26.4139, 27.7852, 55.5266, 28.8408};
  double par_2_max_1sigma[] = {-1.53381, -0.254236, -0.77312, -1.85849, -6.03473, -2.07467};
  double par_3_max_1sigma[] = {0.00817262, -0.0259995, -0.0300504, 0.0211806, 0.113112, 0.0285026};
  double par_4_max_1sigma[] = {0.000129402, 0.000495665, 0.000740296, -6.30543e-05, -0.000871381, -0.000162669};

  double par_0_min_1sigma_out[] = {58.8694, 75.494, 119.951, 52.9731, 111.593, 53.6272};
  double par_1_min_1sigma_out[] = {-43.6605, -55.9065, -89.9595, -41.2703, -85.093, -40.5409};
  double par_2_min_1sigma_out[] = {3.03906, 4.28761, 8.38977, 3.00994, 8.06472, 2.75155};
  double par_3_min_1sigma_out[] = {-0.0301444, -0.0440019, -0.0999269, -0.0313006, -0.0984858, -0.0264477};
  double par_4_min_1sigma_out[] = {0,0,0,0,0,0};
  double par_0_max_1sigma_out[] = {-40.256, -58.3938, -60.3614, -57.7244, -102.98, -51.0424};
  double par_1_max_1sigma_out[] = {31.4367, 43.3923, 45.8203, 42.9619, 79.613, 38.5613};
  double par_2_max_1sigma_out[] = {-1.78797, -3.14225, -3.49161, -2.9105, -7.51346, -2.46405};
  double par_3_max_1sigma_out[] = {0.0146775, 0.0334355, 0.0366689, 0.0277125, 0.0916122, 0.0223185};
  double par_4_max_1sigma_out[] = {0,0,0,0,0,0};


  double par0_min = 0;
  double par1_min = 0;
  double par2_min = 0;
  double par3_min = 0;
  double par4_min = 0;
  double par0_max = 0;
  double par1_max = 0;
  double par2_max = 0;
  double par3_max = 0;
  double par4_max = 0;


  if(tight == true){
    for(int d=0; d<6; d++){
      if(part_Cal_PCAL_sector[j] == d+1){ 
        par0_min = par_0_min_1sigma[d]; par1_min = par_1_min_1sigma[d]; par2_min = par_2_min_1sigma[d]; par3_min = par_3_min_1sigma[d]; par4_min = par_4_min_1sigma[d]; 
        par0_max = par_0_max_1sigma[d]; par1_max = par_1_max_1sigma[d]; par2_max = par_2_max_1sigma[d]; par3_max = par_3_max_1sigma[d]; par4_max = par_4_max_1sigma[d]; 
        if(outbending == true){
          par0_min = par_0_min_1sigma_out[d]; par1_min = par_1_min_1sigma_out[d]; par2_min = par_2_min_1sigma_out[d]; par3_min = par_3_min_1sigma_out[d]; par4_min = par_4_min_1sigma_out[d]; 
          par0_max = par_0_max_1sigma_out[d]; par1_max = par_1_max_1sigma_out[d]; par2_max = par_2_max_1sigma_out[d]; par3_max = par_3_max_1sigma_out[d]; par4_max = par_4_max_1sigma_out[d]; 
        }
      }
    }
  }

  if(medium == true){
    for(int d=0; d<6; d++){
      if(part_Cal_PCAL_sector[j] == d+1){ 
        par0_min = par_0_min_15sigma[d]; par1_min = par_1_min_15sigma[d]; par2_min = par_2_min_15sigma[d]; par3_min = par_3_min_15sigma[d]; par4_min = par_4_min_15sigma[d]; 
        par0_max = par_0_max_15sigma[d]; par1_max = par_1_max_15sigma[d]; par2_max = par_2_max_15sigma[d]; par3_max = par_3_max_15sigma[d]; par4_max = par_4_max_15sigma[d];  
        if(outbending == true){
          par0_min = par_0_min_15sigma_out[d]; par1_min = par_1_min_15sigma_out[d]; par2_min = par_2_min_15sigma_out[d]; par3_min = par_3_min_15sigma_out[d]; par4_min = par_4_min_15sigma_out[d]; 
          par0_max = par_0_max_15sigma_out[d]; par1_max = par_1_max_15sigma_out[d]; par2_max = par_2_max_15sigma_out[d]; par3_max = par_3_max_15sigma_out[d]; par4_max = par_4_max_15sigma_out[d]; 
        }
      }
    }
  }

  if(loose == true){
    for(int d=0; d<6; d++){
      if(part_Cal_PCAL_sector[j] == d+1){ 
        par0_min = par_0_min_2sigma[d]; par1_min = par_1_min_2sigma[d]; par2_min = par_2_min_2sigma[d]; par3_min = par_3_min_2sigma[d]; par4_min = par_4_min_2sigma[d];  
        par0_max = par_0_max_2sigma[d]; par1_max = par_1_max_2sigma[d]; par2_max = par_2_max_2sigma[d]; par3_max = par_3_max_2sigma[d]; par4_max = par_4_max_2sigma[d]; 
        if(outbending == true){
          par0_min = par_0_min_2sigma_out[d]; par1_min = par_1_min_2sigma_out[d]; par2_min = par_2_min_2sigma_out[d]; par3_min = par_3_min_2sigma_out[d]; par4_min = par_4_min_2sigma_out[d]; 
          par0_max = par_0_max_2sigma_out[d]; par1_max = par_1_max_2sigma_out[d]; par2_max = par_2_max_2sigma_out[d]; par3_max = par_3_max_2sigma_out[d]; par4_max = par_4_max_2sigma_out[d]; 
        }
      }
    }
  }

  double min_phi = par0_min + par1_min * log(theta_PCAL) + par2_min * theta_PCAL + par3_min *theta_PCAL*theta_PCAL + par4_min *theta_PCAL*theta_PCAL*theta_PCAL;
  double max_phi = par0_max + par1_max * log(theta_PCAL) + par2_max * theta_PCAL + par3_max *theta_PCAL*theta_PCAL + par4_max *theta_PCAL*theta_PCAL*theta_PCAL;

  if(phi_PCAL >= min_phi && phi_PCAL <= max_phi) return true; 
  else return false;
}



/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 2.1 DC fiducial cuts for the 3 regions based on the drop of the count rate at the edeges of the DC  (only for electrons!)
///     loose: count rate can drop to 35 %
///     medium: count rate can drop to 50 %
///     tight: count rate can drop to 65 %


bool FiducialCuts::DC_hit_position_counts_fiducial_cut(int j, int region){

  // calculate theta and phi local:

  double theta_DC = 0;
  double phi_DC_raw = 0;

  if(region == 1){
    theta_DC = 180/Pival * acos(part_DC_c1z[j]/sqrt(pow(part_DC_c1x[j], 2) + pow(part_DC_c1y[j], 2) + pow(part_DC_c1z[j],2)));
    phi_DC_raw = 180/Pival * atan2(part_DC_c1y[j]/sqrt(pow(part_DC_c1x[j], 2) + pow(part_DC_c1y[j], 2) + pow(part_DC_c1z[j],2)), 
                                   part_DC_c1x[j]/sqrt(pow(part_DC_c1x[j], 2) + pow(part_DC_c1y[j], 2) + pow(part_DC_c1z[j],2)));
  }
  if(region == 2){
    theta_DC = 180/Pival * acos(part_DC_c2z[j]/sqrt(pow(part_DC_c2x[j], 2) + pow(part_DC_c2y[j], 2) + pow(part_DC_c2z[j],2)));
    phi_DC_raw = 180/Pival * atan2(part_DC_c2y[j]/sqrt(pow(part_DC_c2x[j], 2) + pow(part_DC_c2y[j], 2) + pow(part_DC_c2z[j],2)), 
                                   part_DC_c2x[j]/sqrt(pow(part_DC_c2x[j], 2) + pow(part_DC_c2y[j], 2) + pow(part_DC_c2z[j],2)));
  }
  if(region == 3){
    theta_DC = 180/Pival * acos(part_DC_c3z[j]/sqrt(pow(part_DC_c3x[j], 2) + pow(part_DC_c3y[j], 2) + pow(part_DC_c3z[j],2)));
    phi_DC_raw = 180/Pival * atan2(part_DC_c3y[j]/sqrt(pow(part_DC_c3x[j], 2) + pow(part_DC_c3y[j], 2) + pow(part_DC_c3z[j],2)), 
                                   part_DC_c3x[j]/sqrt(pow(part_DC_c3x[j], 2) + pow(part_DC_c3y[j], 2) + pow(part_DC_c3z[j],2)));
  }
    

  double phi_DC = 0;
  if(part_DC_sector[j] == 1) phi_DC = phi_DC_raw;
  if(part_DC_sector[j] == 2) phi_DC = phi_DC_raw - 60;
  if(part_DC_sector[j] == 3) phi_DC = phi_DC_raw - 120;
  if(part_DC_sector[j] == 4 && phi_DC_raw > 0) phi_DC = phi_DC_raw - 180;
  if(part_DC_sector[j] == 4 && phi_DC_raw < 0) phi_DC = phi_DC_raw + 180;
  if(part_DC_sector[j] == 5) phi_DC = phi_DC_raw + 120;
  if(part_DC_sector[j] == 6) phi_DC = phi_DC_raw + 60;

 // inbending cut parameters (adjusted):

 // medium (50 %):

 double reg1_min_sec1_inb[] = {25.6464, -26.0938, 1.36205, 0.0083525, -0.000433074};
 double reg1_max_sec1_inb[] = {-21.002, 19.3141, 0.183817, -0.061424, 0.00117881};
 double reg2_min_sec1_inb[] = {20.8156, -19.1708, -0.244432, 0.0652386, -0.00120129};
 double reg2_max_sec1_inb[] = {-20.2319, 19.2371, 0.0491417, -0.0527963, 0.000995253};
 double reg3_min_sec1_inb[] = {22.9342, -24.6621, 1.25766, 0.0133507, -0.00049441};
 double reg3_max_sec1_inb[] = {-8.72949, 4.50811, 3.13299, -0.15452, 0.00234593};
 double reg1_min_sec2_inb[] = {16.0752, -15.8399, -0.65495, 0.0740595, -0.00133068};
 double reg1_max_sec2_inb[] = {-40.921, 40.2558, -3.67186, 0.0498104, -0.000151358};
 double reg2_min_sec2_inb[] = {20.5524, -23.7002, 1.20081, 0.0126066, -0.000501927};
 double reg2_max_sec2_inb[] = {-22.7933, 18.397, 0.707315, -0.0848617, 0.00150965};
 double reg3_min_sec2_inb[] = {7.84857, -9.62769, -1.39224, 0.0876156, -0.00137369};
 double reg3_max_sec2_inb[] = {-24.5099, 24.8373, -1.22162, -0.0135951, 0.00051455};
 double reg1_min_sec3_inb[] = {18.2067, -12.322, -1.79675, 0.111661, -0.0017831};
 double reg1_max_sec3_inb[] = {-20.4639, 11.3285, 2.56498, -0.149755, 0.00243423};
 double reg2_min_sec3_inb[] = {17.5093, -13.0897, -1.44801, 0.0995944, -0.00163973};
 double reg2_max_sec3_inb[] = {-29.4241, 25.1723, -0.643032, -0.0433457, 0.0010157};
 double reg3_min_sec3_inb[] = {19.6412, -18.3363, -0.139482, 0.0623644, -0.00122319};
 double reg3_max_sec3_inb[] = {-23.4069, 20.1087, 0.0111006, -0.0595069, 0.00123094};
 double reg1_min_sec4_inb[] = {27.7329, -24.8668, 0.855257, 0.0229504, -0.000588103};
 double reg1_max_sec4_inb[] = {-25.1595, 24.3626, -0.826633, -0.0280833, 0.000694613};
 double reg2_min_sec4_inb[] = {35.3061, -35.9815, 3.34709, -0.0587318, 0.000517383};
 double reg2_max_sec4_inb[] = {-23.5005, 23.5232, -0.872864, -0.0214562, 0.000570724};
 double reg3_min_sec4_inb[] = {13.1517, -8.73139, -2.18634, 0.115133, -0.00170354};
 double reg3_max_sec4_inb[] = {-14.1635, 13.6887, 0.949846, -0.0787818, 0.00128683};
 double reg1_min_sec5_inb[] = {23.5653, -17.285, -1.21053, 0.108096, -0.00189499};
 double reg1_max_sec5_inb[] = {-23.3017, 16.1351, 1.45328, -0.112999, 0.00193088};
 double reg2_min_sec5_inb[] = {30.5631, -30.3887, 2.11072, -0.00879513, -0.000281135};
 double reg2_max_sec5_inb[] = {-18.7762, 11.5513, 2.23634, -0.132, 0.00208225};
 double reg3_min_sec5_inb[] = {14.2375, -10.0129, -2.03418, 0.1236, -0.00199283};
 double reg3_max_sec5_inb[] = {-23.8159, 21.7783, -0.373554, -0.0463973, 0.0010055};
 double reg1_min_sec6_inb[] = {34.9766, -38.2253, 3.80455, -0.0624294, 0.000385789};
 double reg1_max_sec6_inb[] = {-23.6287, 21.0793, 0.0916929, -0.0641014, 0.00125089};
 double reg2_min_sec6_inb[] = {21.6462, -23.9106, 1.1985, 0.0108284, -0.000435214};
 double reg2_max_sec6_inb[] = {-22.1881, 21.3921, -0.325758, -0.0406509, 0.000821165};
 double reg3_min_sec6_inb[] = {12.3393, -12.5734, -1.07645, 0.0836258, -0.00137295};
 double reg3_max_sec6_inb[] = {-20.6041, 23.7403, -1.43195, 0.00507544, 0.000160285};

 // loose (35 %):
 double reg1_min_sec1_inb_loose[] = {24.9198, -26.6682, 1.22328, 0.0326184, 0.0326184};
 double reg1_max_sec1_inb_loose[] = {-38.0903, 44.8413, -5.13684, 0.0913599, -0.000566229};
 double reg2_min_sec1_inb_loose[] = {34.3872, -41.6892, 4.78076, -0.0904016, 0.000691157};
 double reg2_max_sec1_inb_loose[] = {-36.582, 45.4679, -5.70527, 0.121105, -0.00107292};
 double reg3_min_sec1_inb_loose[] = {38.5882, -51.7707, 7.65247, -0.198183, 0.00222671};
 double reg3_max_sec1_inb_loose[] = {-35.1098, 46.9821, -6.62713, 0.16512, -0.00177347};
 double reg1_min_sec2_inb_loose[] = {26.3964, -32.5314, 2.87032, -0.0261837, -0.0261837};
 double reg1_max_sec2_inb_loose[] = {-45.4528, 53.4467, -6.83895, 0.144609, -0.00127358};
 double reg2_min_sec2_inb_loose[] = {29.5885, -38.6103, 4.45494, -0.084114, 0.000626459};
 double reg2_max_sec2_inb_loose[] = {-42.6042, 50.783, -6.44381, 0.135305, -0.00117332};
 double reg3_min_sec2_inb_loose[] = {21.3286, -33.2316, 4.16718, -0.0947928, 0.000950229};
 double reg3_max_sec2_inb_loose[] = {-26.2955, 32.832, -3.21406, 0.0443669, -0.000110018};
 double reg1_min_sec3_inb_loose[] = {38.3683, -39.555, 3.48786, -0.0311045, -0.0311045};
 double reg1_max_sec3_inb_loose[] = {-44.7002, 48.5238, -5.33933, 0.0857334, -0.000389759};
 double reg2_min_sec3_inb_loose[] = {26.2411, -26.4956, 1.17454, 0.0314168, -0.000923764};
 double reg2_max_sec3_inb_loose[] = {-49.1042, 53.7936, -6.50797, 0.125435, -0.000940508};
 double reg3_min_sec3_inb_loose[] = {30.2087, -36.2303, 3.94272, -0.070659, 0.000484111};
 double reg3_max_sec3_inb_loose[] = {-25.1075, 27.5492, -1.81589, -0.00372993, 0.000534551};
 double reg1_min_sec4_inb_loose[] = {45.5355, -50.2561, 5.91736, -0.116203, -0.116203};
 double reg1_max_sec4_inb_loose[] = {-45.3956, 52.8208, -6.57583, 0.13245, -0.00106037};
 double reg2_min_sec4_inb_loose[] = {42.9007, -48.8609, 6.02101, -0.13073, 0.00125121};
 double reg2_max_sec4_inb_loose[] = {-32.1523, 38.6933, -4.15879, 0.0722104, -0.000474606};
 double reg3_min_sec4_inb_loose[] = {34.0991, -40.7811, 4.8997, -0.107033, 0.00105086};
 double reg3_max_sec4_inb_loose[] = {-39.189, 52.9641, -8.01756, 0.214872, -0.00250226};
 double reg1_min_sec5_inb_loose[] = {52.0273, -61.3244, 8.45187, -0.193011, -0.193011};
 double reg1_max_sec5_inb_loose[] = {-50.6284, 54.3673, -6.38031, 0.116905, -0.000778886};
 double reg2_min_sec5_inb_loose[] = {32.1495, -35.3084, 3.11177, -0.0314703, -8.39833e-05};
 double reg2_max_sec5_inb_loose[] = {-47.0576, 50.676, -5.80697, 0.10389, -0.000677858};
 double reg3_min_sec5_inb_loose[] = {27.5659, -34.8935, 3.81989, -0.0655272, 0.000389246};
 double reg3_max_sec5_inb_loose[] = {-25.7138, 27.0011, -1.53335, -0.0152295, 0.000694336};
 double reg1_min_sec6_inb_loose[] = {35.6586, -43.3351, 5.05374, -0.0961372, -0.0961372};
 double reg1_max_sec6_inb_loose[] = {-43.6986, 52.1413, -6.61602, 0.137247, -0.00115149};
 double reg2_min_sec6_inb_loose[] = {33.3586, -42.3939, 5.17582, -0.106612, 0.000910642};
 double reg2_max_sec6_inb_loose[] = {-38.3305, 46.3099, -5.57615, 0.109409, -0.000869664};
 double reg3_min_sec6_inb_loose[] = {20.9702, -29.641, 3.08715, -0.0542694, 0.00036852};
 double reg3_max_sec6_inb_loose[] = {-36.3081, 47.6882, -6.51571, 0.152934, -0.00153014};

 // tight (65 %):
 double reg1_min_sec1_inb_tight[] = {10.8127, 0.867417, -5.23315, 0.236924, 0.236924};
 double reg1_max_sec1_inb_tight[] = {-1.05905, -11.11, 6.99604, -0.280718, 0.00394115};
 double reg2_min_sec1_inb_tight[] = {15.6429, -8.51554, -2.8828, 0.158688, -0.00249913};
 double reg2_max_sec1_inb_tight[] = {4.94591, -19.5039, 8.81259, -0.33865, 0.00465444};
 double reg3_min_sec1_inb_tight[] = {1.34092, 9.28247, -6.60044, 0.281318, -0.00409396};
 double reg3_max_sec1_inb_tight[] = {7.95213, -22.3926, 9.38108, -0.362289, 0.00504018};
 double reg1_min_sec2_inb_tight[] = {6.83952, 1.15885, -4.75388, 0.212145, 0.212145};
 double reg1_max_sec2_inb_tight[] = {-13.5338, 2.90293, 4.04606, -0.18406, 0.00270977};
 double reg2_min_sec2_inb_tight[] = {2.23926, 6.12481, -5.72273, 0.242722, -0.00348261};
 double reg2_max_sec2_inb_tight[] = {-19.2228, 12.2586, 1.80921, -0.109461, 0.00172974};
 double reg3_min_sec2_inb_tight[] = {-7.44705, 15.9478, -7.51342, 0.2998, -0.00422782};
 double reg3_max_sec2_inb_tight[] = {-8.05779, -1.88087, 4.79989, -0.208209, 0.00301469};
 double reg1_min_sec3_inb_tight[] = {5.36591, 8.06724, -6.39578, 0.262518, 0.262518};
 double reg1_max_sec3_inb_tight[] = {-8.20889, -5.7771, 5.98507, -0.245252, 0.00348177};
 double reg2_min_sec3_inb_tight[] = {-2.61308, 17.4864, -8.28425, 0.321424, -0.00449555};
 double reg2_max_sec3_inb_tight[] = {-15.7028, 5.06188, 3.58882, -0.169598, 0.00252206};
 double reg3_min_sec3_inb_tight[] = {-1.47028, 12.199, -6.67515, 0.261044, -0.00361062};
 double reg3_max_sec3_inb_tight[] = {-2.49195, -10.9866, 6.77373, -0.267217, 0.0037212};
 double reg1_min_sec4_inb_tight[] = {7.28085, 4.43634, -5.4954, 0.22469, 0.22469};
 double reg1_max_sec4_inb_tight[] = {-2.88726, -7.75256, 6.11348, -0.246024, 0.00342};
 double reg2_min_sec4_inb_tight[] = {11.1628, -0.875717, -4.40181, 0.191682, -0.00269305};
 double reg2_max_sec4_inb_tight[] = {4.98009, -20.3121, 9.08305, -0.347362, 0.00476636};
 double reg3_min_sec4_inb_tight[] = {-1.53387, 16.9129, -8.4497, 0.334303, -0.00465195};
 double reg3_max_sec4_inb_tight[] = {5.76932, -17.8998, 8.19437, -0.317309, 0.00436422};
 double reg1_min_sec5_inb_tight[] = {16.2619, -7.2257, -3.02427, 0.151797, 0.151797};
 double reg1_max_sec5_inb_tight[] = {-8.7963, -3.03534, 5.17438, -0.214586, 0.0030239};
 double reg2_min_sec5_inb_tight[] = {14.656, -7.43444, -2.74998, 0.141668, -0.00216617};
 double reg2_max_sec5_inb_tight[] = {2.24964, -18.1672, 8.48444, -0.321566, 0.00438927};
 double reg3_min_sec5_inb_tight[] = {3.38717, 7.1616, -5.73249, 0.231652, -0.00320412};
 double reg3_max_sec5_inb_tight[] = {4.63583, -20.0558, 8.78226, -0.33313, 0.0045258};
 double reg1_min_sec6_inb_tight[] = {8.74274, 0.225052, -4.62671, 0.205157, 0.205157};
 double reg1_max_sec6_inb_tight[] = {-8.82084, 1.73358, 3.85433, -0.168173, 0.0024055};
 double reg2_min_sec6_inb_tight[] = {3.86614, 7.51372, -6.36287, 0.268641, -0.00385432};
 double reg2_max_sec6_inb_tight[] = {-7.0834, -0.809866, 4.46815, -0.18982, 0.00266762};
 double reg3_min_sec6_inb_tight[] = {3.12244, 3.84008, -5.05955, 0.223398, -0.00326745};
 double reg3_max_sec6_inb_tight[] = {5.70817, -16.9736, 7.8074, -0.295122, 0.00398425};

 // outbending cut parameters (not adjusted, same as inbending):

 //_medium (50 %):
 double reg1_min_sec1_outb[] = {25.6464, -26.0938, 1.36205, 0.0083525, 0.0083525};
 double reg1_max_sec1_outb[] = {-21.002, 19.3141, 0.183817, -0.061424, 0.00117881};
 double reg2_min_sec1_outb[] = {20.8156, -19.1708, -0.244432, 0.0652386, -0.00120129};
 double reg2_max_sec1_outb[] = {-20.2319, 19.2371, 0.0491417, -0.0527963, 0.000995253};
 double reg3_min_sec1_outb[] = {22.9342, -24.6621, 1.25766, 0.0133507, -0.00049441};
 double reg3_max_sec1_outb[] = {-8.72949, 4.50811, 3.13299, -0.15452, 0.00234593};
 double reg1_min_sec2_outb[] = {16.0752, -15.8399, -0.65495, 0.0740595, 0.0740595};
 double reg1_max_sec2_outb[] = {-40.921, 40.2558, -3.67186, 0.0498104, -0.000151358};
 double reg2_min_sec2_outb[] = {20.5524, -23.7002, 1.20081, 0.0126066, -0.000501927};
 double reg2_max_sec2_outb[] = {-22.7933, 18.397, 0.707315, -0.0848617, 0.00150965};
 double reg3_min_sec2_outb[] = {7.84857, -9.62769, -1.39224, 0.0876156, -0.00137369};
 double reg3_max_sec2_outb[] = {-24.5099, 24.8373, -1.22162, -0.0135951, 0.00051455};
 double reg1_min_sec3_outb[] = {18.2067, -12.322, -1.79675, 0.111661, 0.111661};
 double reg1_max_sec3_outb[] = {-20.4639, 11.3285, 2.56498, -0.149755, 0.00243423};
 double reg2_min_sec3_outb[] = {17.5093, -13.0897, -1.44801, 0.0995944, -0.00163973};
 double reg2_max_sec3_outb[] = {-29.4241, 25.1723, -0.643032, -0.0433457, 0.0010157};
 double reg3_min_sec3_outb[] = {19.6412, -18.3363, -0.139482, 0.0623644, -0.00122319};
 double reg3_max_sec3_outb[] = {-23.4069, 20.1087, 0.0111006, -0.0595069, 0.00123094};
 double reg1_min_sec4_outb[] = {27.7329, -24.8668, 0.855257, 0.0229504, 0.0229504};
 double reg1_max_sec4_outb[] = {-25.1595, 24.3626, -0.826633, -0.0280833, 0.000694613};
 double reg2_min_sec4_outb[] = {35.3061, -35.9815, 3.34709, -0.0587318, 0.000517383};
 double reg2_max_sec4_outb[] = {-23.5005, 23.5232, -0.872864, -0.0214562, 0.000570724};
 double reg3_min_sec4_outb[] = {13.1517, -8.73139, -2.18634, 0.115133, -0.00170354};
 double reg3_max_sec4_outb[] = {-14.1635, 13.6887, 0.949846, -0.0787818, 0.00128683};
 double reg1_min_sec5_outb[] = {23.5653, -17.285, -1.21053, 0.108096, 0.108096};
 double reg1_max_sec5_outb[] = {-23.3017, 16.1351, 1.45328, -0.112999, 0.00193088};
 double reg2_min_sec5_outb[] = {30.5631, -30.3887, 2.11072, -0.00879513, -0.000281135};
 double reg2_max_sec5_outb[] = {-18.7762, 11.5513, 2.23634, -0.132, 0.00208225};
 double reg3_min_sec5_outb[] = {14.2375, -10.0129, -2.03418, 0.1236, -0.00199283};
 double reg3_max_sec5_outb[] = {-23.8159, 21.7783, -0.373554, -0.0463973, 0.0010055};
 double reg1_min_sec6_outb[] = {34.9766, -38.2253, 3.80455, -0.0624294, -0.0624294};
 double reg1_max_sec6_outb[] = {-23.6287, 21.0793, 0.0916929, -0.0641014, 0.00125089};
 double reg2_min_sec6_outb[] = {21.6462, -23.9106, 1.1985, 0.0108284, -0.000435214};
 double reg2_max_sec6_outb[] = {-22.1881, 21.3921, -0.325758, -0.0406509, 0.000821165};
 double reg3_min_sec6_outb[] = {12.3393, -12.5734, -1.07645, 0.0836258, -0.00137295};
 double reg3_max_sec6_outb[] = {-20.6041, 23.7403, -1.43195, 0.00507544, 0.000160285};

 // loose (35 %):
 double reg1_min_sec1_outb_loose[] = {24.9198, -26.6682, 1.22328, 0.0326184, 0.0326184};
 double reg1_max_sec1_outb_loose[] = {-38.0903, 44.8413, -5.13684, 0.0913599, -0.000566229};
 double reg2_min_sec1_outb_loose[] = {34.3872, -41.6892, 4.78076, -0.0904016, 0.000691157};
 double reg2_max_sec1_outb_loose[] = {-36.582, 45.4679, -5.70527, 0.121105, -0.00107292};
 double reg3_min_sec1_outb_loose[] = {38.5882, -51.7707, 7.65247, -0.198183, 0.00222671};
 double reg3_max_sec1_outb_loose[] = {-35.1098, 46.9821, -6.62713, 0.16512, -0.00177347};
 double reg1_min_sec2_outb_loose[] = {26.3964, -32.5314, 2.87032, -0.0261837, -0.0261837};
 double reg1_max_sec2_outb_loose[] = {-45.4528, 53.4467, -6.83895, 0.144609, -0.00127358};
 double reg2_min_sec2_outb_loose[] = {29.5885, -38.6103, 4.45494, -0.084114, 0.000626459};
 double reg2_max_sec2_outb_loose[] = {-42.6042, 50.783, -6.44381, 0.135305, -0.00117332};
 double reg3_min_sec2_outb_loose[] = {21.3286, -33.2316, 4.16718, -0.0947928, 0.000950229};
 double reg3_max_sec2_outb_loose[] = {-26.2955, 32.832, -3.21406, 0.0443669, -0.000110018};
 double reg1_min_sec3_outb_loose[] = {38.3683, -39.555, 3.48786, -0.0311045, -0.0311045};
 double reg1_max_sec3_outb_loose[] = {-44.7002, 48.5238, -5.33933, 0.0857334, -0.000389759};
 double reg2_min_sec3_outb_loose[] = {26.2411, -26.4956, 1.17454, 0.0314168, -0.000923764};
 double reg2_max_sec3_outb_loose[] = {-49.1042, 53.7936, -6.50797, 0.125435, -0.000940508};
 double reg3_min_sec3_outb_loose[] = {30.2087, -36.2303, 3.94272, -0.070659, 0.000484111};
 double reg3_max_sec3_outb_loose[] = {-25.1075, 27.5492, -1.81589, -0.00372993, 0.000534551};
 double reg1_min_sec4_outb_loose[] = {45.5355, -50.2561, 5.91736, -0.116203, -0.116203};
 double reg1_max_sec4_outb_loose[] = {-45.3956, 52.8208, -6.57583, 0.13245, -0.00106037};
 double reg2_min_sec4_outb_loose[] = {42.9007, -48.8609, 6.02101, -0.13073, 0.00125121};
 double reg2_max_sec4_outb_loose[] = {-32.1523, 38.6933, -4.15879, 0.0722104, -0.000474606};
 double reg3_min_sec4_outb_loose[] = {34.0991, -40.7811, 4.8997, -0.107033, 0.00105086};
 double reg3_max_sec4_outb_loose[] = {-39.189, 52.9641, -8.01756, 0.214872, -0.00250226};
 double reg1_min_sec5_outb_loose[] = {52.0273, -61.3244, 8.45187, -0.193011, -0.193011};
 double reg1_max_sec5_outb_loose[] = {-50.6284, 54.3673, -6.38031, 0.116905, -0.000778886};
 double reg2_min_sec5_outb_loose[] = {32.1495, -35.3084, 3.11177, -0.0314703, -8.39833e-05};
 double reg2_max_sec5_outb_loose[] = {-47.0576, 50.676, -5.80697, 0.10389, -0.000677858};
 double reg3_min_sec5_outb_loose[] = {27.5659, -34.8935, 3.81989, -0.0655272, 0.000389246};
 double reg3_max_sec5_outb_loose[] = {-25.7138, 27.0011, -1.53335, -0.0152295, 0.000694336};
 double reg1_min_sec6_outb_loose[] = {35.6586, -43.3351, 5.05374, -0.0961372, -0.0961372};
 double reg1_max_sec6_outb_loose[] = {-43.6986, 52.1413, -6.61602, 0.137247, -0.00115149};
 double reg2_min_sec6_outb_loose[] = {33.3586, -42.3939, 5.17582, -0.106612, 0.000910642};
 double reg2_max_sec6_outb_loose[] = {-38.3305, 46.3099, -5.57615, 0.109409, -0.000869664};
 double reg3_min_sec6_outb_loose[] = {20.9702, -29.641, 3.08715, -0.0542694, 0.00036852};
 double reg3_max_sec6_outb_loose[] = {-36.3081, 47.6882, -6.51571, 0.152934, -0.00153014};

 // tight (65 %):
 double reg1_min_sec1_outb_tight[] = {10.8127, 0.867417, -5.23315, 0.236924, 0.236924};
 double reg1_max_sec1_outb_tight[] = {-1.05905, -11.11, 6.99604, -0.280718, 0.00394115};
 double reg2_min_sec1_outb_tight[] = {15.6429, -8.51554, -2.8828, 0.158688, -0.00249913};
 double reg2_max_sec1_outb_tight[] = {4.94591, -19.5039, 8.81259, -0.33865, 0.00465444};
 double reg3_min_sec1_outb_tight[] = {1.34092, 9.28247, -6.60044, 0.281318, -0.00409396};
 double reg3_max_sec1_outb_tight[] = {7.95213, -22.3926, 9.38108, -0.362289, 0.00504018};
 double reg1_min_sec2_outb_tight[] = {6.83952, 1.15885, -4.75388, 0.212145, 0.212145};
 double reg1_max_sec2_outb_tight[] = {-13.5338, 2.90293, 4.04606, -0.18406, 0.00270977};
 double reg2_min_sec2_outb_tight[] = {2.23926, 6.12481, -5.72273, 0.242722, -0.00348261};
 double reg2_max_sec2_outb_tight[] = {-19.2228, 12.2586, 1.80921, -0.109461, 0.00172974};
 double reg3_min_sec2_outb_tight[] = {-7.44705, 15.9478, -7.51342, 0.2998, -0.00422782};
 double reg3_max_sec2_outb_tight[] = {-8.05779, -1.88087, 4.79989, -0.208209, 0.00301469};
 double reg1_min_sec3_outb_tight[] = {5.36591, 8.06724, -6.39578, 0.262518, 0.262518};
 double reg1_max_sec3_outb_tight[] = {-8.20889, -5.7771, 5.98507, -0.245252, 0.00348177};
 double reg2_min_sec3_outb_tight[] = {-2.61308, 17.4864, -8.28425, 0.321424, -0.00449555};
 double reg2_max_sec3_outb_tight[] = {-15.7028, 5.06188, 3.58882, -0.169598, 0.00252206};
 double reg3_min_sec3_outb_tight[] = {-1.47028, 12.199, -6.67515, 0.261044, -0.00361062};
 double reg3_max_sec3_outb_tight[] = {-2.49195, -10.9866, 6.77373, -0.267217, 0.0037212};
 double reg1_min_sec4_outb_tight[] = {7.28085, 4.43634, -5.4954, 0.22469, 0.22469};
 double reg1_max_sec4_outb_tight[] = {-2.88726, -7.75256, 6.11348, -0.246024, 0.00342};
 double reg2_min_sec4_outb_tight[] = {11.1628, -0.875717, -4.40181, 0.191682, -0.00269305};
 double reg2_max_sec4_outb_tight[] = {4.98009, -20.3121, 9.08305, -0.347362, 0.00476636};
 double reg3_min_sec4_outb_tight[] = {-1.53387, 16.9129, -8.4497, 0.334303, -0.00465195};
 double reg3_max_sec4_outb_tight[] = {5.76932, -17.8998, 8.19437, -0.317309, 0.00436422};
 double reg1_min_sec5_outb_tight[] = {16.2619, -7.2257, -3.02427, 0.151797, 0.151797};
 double reg1_max_sec5_outb_tight[] = {-8.7963, -3.03534, 5.17438, -0.214586, 0.0030239};
 double reg2_min_sec5_outb_tight[] = {14.656, -7.43444, -2.74998, 0.141668, -0.00216617};
 double reg2_max_sec5_outb_tight[] = {2.24964, -18.1672, 8.48444, -0.321566, 0.00438927};
 double reg3_min_sec5_outb_tight[] = {3.38717, 7.1616, -5.73249, 0.231652, -0.00320412};
 double reg3_max_sec5_outb_tight[] = {4.63583, -20.0558, 8.78226, -0.33313, 0.0045258};
 double reg1_min_sec6_outb_tight[] = {8.74274, 0.225052, -4.62671, 0.205157, 0.205157};
 double reg1_max_sec6_outb_tight[] = {-8.82084, 1.73358, 3.85433, -0.168173, 0.0024055};
 double reg2_min_sec6_outb_tight[] = {3.86614, 7.51372, -6.36287, 0.268641, -0.00385432};
 double reg2_max_sec6_outb_tight[] = {-7.0834, -0.809866, 4.46815, -0.18982, 0.00266762};
 double reg3_min_sec6_outb_tight[] = {3.12244, 3.84008, -5.05955, 0.223398, -0.00326745};
 double reg3_max_sec6_outb_tight[] = {5.70817, -16.9736, 7.8074, -0.295122, 0.00398425};


 double p0_min = 0; double p1_min = 0; double p2_min = 0; double p3_min = 0; double p4_min = 0;
 double p0_max = 0; double p1_max = 0; double p2_max = 0; double p3_max = 0; double p4_max = 0;


  if(region == 1){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg1_min_sec1_inb[0]; p1_min = reg1_min_sec1_inb[1]; p2_min = reg1_min_sec1_inb[2]; p3_min = reg1_min_sec1_inb[3]; p4_min = reg1_min_sec1_inb[4];
      p0_max = reg1_max_sec1_inb[0]; p1_max = reg1_max_sec1_inb[1]; p2_max = reg1_max_sec1_inb[2]; p3_max = reg1_max_sec1_inb[3]; p4_max = reg1_max_sec1_inb[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg1_min_sec2_inb[0]; p1_min = reg1_min_sec2_inb[1]; p2_min = reg1_min_sec2_inb[2]; p3_min = reg1_min_sec2_inb[3]; p4_min = reg1_min_sec2_inb[4];
      p0_max = reg1_max_sec2_inb[0]; p1_max = reg1_max_sec2_inb[1]; p2_max = reg1_max_sec2_inb[2]; p3_max = reg1_max_sec2_inb[3]; p4_max = reg1_max_sec2_inb[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg1_min_sec3_inb[0]; p1_min = reg1_min_sec3_inb[1]; p2_min = reg1_min_sec3_inb[2]; p3_min = reg1_min_sec3_inb[3]; p4_min = reg1_min_sec3_inb[4];
      p0_max = reg1_max_sec3_inb[0]; p1_max = reg1_max_sec3_inb[1]; p2_max = reg1_max_sec3_inb[2]; p3_max = reg1_max_sec3_inb[3]; p4_max = reg1_max_sec3_inb[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg1_min_sec4_inb[0]; p1_min = reg1_min_sec4_inb[1]; p2_min = reg1_min_sec4_inb[2]; p3_min = reg1_min_sec4_inb[3]; p4_min = reg1_min_sec4_inb[4];
      p0_max = reg1_max_sec4_inb[0]; p1_max = reg1_max_sec4_inb[1]; p2_max = reg1_max_sec4_inb[2]; p3_max = reg1_max_sec4_inb[3]; p4_max = reg1_max_sec4_inb[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg1_min_sec5_inb[0]; p1_min = reg1_min_sec5_inb[1]; p2_min = reg1_min_sec5_inb[2]; p3_min = reg1_min_sec5_inb[3]; p4_min = reg1_min_sec5_inb[4];
      p0_max = reg1_max_sec5_inb[0]; p1_max = reg1_max_sec5_inb[1]; p2_max = reg1_max_sec5_inb[2]; p3_max = reg1_max_sec5_inb[3]; p4_max = reg1_max_sec5_inb[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg1_min_sec6_inb[0]; p1_min = reg1_min_sec6_inb[1]; p2_min = reg1_min_sec6_inb[2]; p3_min = reg1_min_sec6_inb[3]; p4_min = reg1_min_sec6_inb[4];
      p0_max = reg1_max_sec6_inb[0]; p1_max = reg1_max_sec6_inb[1]; p2_max = reg1_max_sec6_inb[2]; p3_max = reg1_max_sec6_inb[3]; p4_max = reg1_max_sec6_inb[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg1_min_sec1_outb[0]; p1_min = reg1_min_sec1_outb[1]; p2_min = reg1_min_sec1_outb[2]; p3_min = reg1_min_sec1_outb[3]; p4_min = reg1_min_sec1_outb[4];
      p0_max = reg1_max_sec1_outb[0]; p1_max = reg1_max_sec1_outb[1]; p2_max = reg1_max_sec1_outb[2]; p3_max = reg1_max_sec1_outb[3]; p4_max = reg1_max_sec1_outb[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg1_min_sec2_outb[0]; p1_min = reg1_min_sec2_outb[1]; p2_min = reg1_min_sec2_outb[2]; p3_min = reg1_min_sec2_outb[3]; p4_min = reg1_min_sec2_outb[4];
      p0_max = reg1_max_sec2_outb[0]; p1_max = reg1_max_sec2_outb[1]; p2_max = reg1_max_sec2_outb[2]; p3_max = reg1_max_sec2_outb[3]; p4_max = reg1_max_sec2_outb[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg1_min_sec3_outb[0]; p1_min = reg1_min_sec3_outb[1]; p2_min = reg1_min_sec3_outb[2]; p3_min = reg1_min_sec3_outb[3]; p4_min = reg1_min_sec3_outb[4];
      p0_max = reg1_max_sec3_outb[0]; p1_max = reg1_max_sec3_outb[1]; p2_max = reg1_max_sec3_outb[2]; p3_max = reg1_max_sec3_outb[3]; p4_max = reg1_max_sec3_outb[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg1_min_sec4_outb[0]; p1_min = reg1_min_sec4_outb[1]; p2_min = reg1_min_sec4_outb[2]; p3_min = reg1_min_sec4_outb[3]; p4_min = reg1_min_sec4_outb[4];
      p0_max = reg1_max_sec4_outb[0]; p1_max = reg1_max_sec4_outb[1]; p2_max = reg1_max_sec4_outb[2]; p3_max = reg1_max_sec4_outb[3]; p4_max = reg1_max_sec4_outb[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg1_min_sec5_outb[0]; p1_min = reg1_min_sec5_outb[1]; p2_min = reg1_min_sec5_outb[2]; p3_min = reg1_min_sec5_outb[3]; p4_min = reg1_min_sec5_outb[4];
      p0_max = reg1_max_sec5_outb[0]; p1_max = reg1_max_sec5_outb[1]; p2_max = reg1_max_sec5_outb[2]; p3_max = reg1_max_sec5_outb[3]; p4_max = reg1_max_sec5_outb[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg1_min_sec6_outb[0]; p1_min = reg1_min_sec6_outb[1]; p2_min = reg1_min_sec6_outb[2]; p3_min = reg1_min_sec6_outb[3]; p4_min = reg1_min_sec6_outb[4];
      p0_max = reg1_max_sec6_outb[0]; p1_max = reg1_max_sec6_outb[1]; p2_max = reg1_max_sec6_outb[2]; p3_max = reg1_max_sec6_outb[3]; p4_max = reg1_max_sec6_outb[4];
    }
  }

  if(region == 2){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg2_min_sec1_inb[0]; p1_min = reg2_min_sec1_inb[1]; p2_min = reg2_min_sec1_inb[2]; p3_min = reg2_min_sec1_inb[3]; p4_min = reg2_min_sec1_inb[4];
      p0_max = reg2_max_sec1_inb[0]; p1_max = reg2_max_sec1_inb[1]; p2_max = reg2_max_sec1_inb[2]; p3_max = reg2_max_sec1_inb[3]; p4_max = reg2_max_sec1_inb[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg2_min_sec2_inb[0]; p1_min = reg2_min_sec2_inb[1]; p2_min = reg2_min_sec2_inb[2]; p3_min = reg2_min_sec2_inb[3]; p4_min = reg2_min_sec2_inb[4];
      p0_max = reg2_max_sec2_inb[0]; p1_max = reg2_max_sec2_inb[1]; p2_max = reg2_max_sec2_inb[2]; p3_max = reg2_max_sec2_inb[3]; p4_max = reg2_max_sec2_inb[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg2_min_sec3_inb[0]; p1_min = reg2_min_sec3_inb[1]; p2_min = reg2_min_sec3_inb[2]; p3_min = reg2_min_sec3_inb[3]; p4_min = reg2_min_sec3_inb[4];
      p0_max = reg2_max_sec3_inb[0]; p1_max = reg2_max_sec3_inb[1]; p2_max = reg2_max_sec3_inb[2]; p3_max = reg2_max_sec3_inb[3]; p4_max = reg2_max_sec3_inb[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg2_min_sec4_inb[0]; p1_min = reg2_min_sec4_inb[1]; p2_min = reg2_min_sec4_inb[2]; p3_min = reg2_min_sec4_inb[3]; p4_min = reg2_min_sec4_inb[4];
      p0_max = reg2_max_sec4_inb[0]; p1_max = reg2_max_sec4_inb[1]; p2_max = reg2_max_sec4_inb[2]; p3_max = reg2_max_sec4_inb[3]; p4_max = reg2_max_sec4_inb[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg2_min_sec5_inb[0]; p1_min = reg2_min_sec5_inb[1]; p2_min = reg2_min_sec5_inb[2]; p3_min = reg2_min_sec5_inb[3]; p4_min = reg2_min_sec5_inb[4];
      p0_max = reg2_max_sec5_inb[0]; p1_max = reg2_max_sec5_inb[1]; p2_max = reg2_max_sec5_inb[2]; p3_max = reg2_max_sec5_inb[3]; p4_max = reg2_max_sec5_inb[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg2_min_sec6_inb[0]; p1_min = reg2_min_sec6_inb[1]; p2_min = reg2_min_sec6_inb[2]; p3_min = reg2_min_sec6_inb[3]; p4_min = reg2_min_sec6_inb[4];
      p0_max = reg2_max_sec6_inb[0]; p1_max = reg2_max_sec6_inb[1]; p2_max = reg2_max_sec6_inb[2]; p3_max = reg2_max_sec6_inb[3]; p4_max = reg2_max_sec6_inb[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg2_min_sec1_outb[0]; p1_min = reg2_min_sec1_outb[1]; p2_min = reg2_min_sec1_outb[2]; p3_min = reg2_min_sec1_outb[3]; p4_min = reg2_min_sec1_outb[4];
      p0_max = reg2_max_sec1_outb[0]; p1_max = reg2_max_sec1_outb[1]; p2_max = reg2_max_sec1_outb[2]; p3_max = reg2_max_sec1_outb[3]; p4_max = reg2_max_sec1_outb[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg2_min_sec2_outb[0]; p1_min = reg2_min_sec2_outb[1]; p2_min = reg2_min_sec2_outb[2]; p3_min = reg2_min_sec2_outb[3]; p4_min = reg2_min_sec2_outb[4];
      p0_max = reg2_max_sec2_outb[0]; p1_max = reg2_max_sec2_outb[1]; p2_max = reg2_max_sec2_outb[2]; p3_max = reg2_max_sec2_outb[3]; p4_max = reg2_max_sec2_outb[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg2_min_sec3_outb[0]; p1_min = reg2_min_sec3_outb[1]; p2_min = reg2_min_sec3_outb[2]; p3_min = reg2_min_sec3_outb[3]; p4_min = reg2_min_sec3_outb[4];
      p0_max = reg2_max_sec3_outb[0]; p1_max = reg2_max_sec3_outb[1]; p2_max = reg2_max_sec3_outb[2]; p3_max = reg2_max_sec3_outb[3]; p4_max = reg2_max_sec3_outb[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg2_min_sec4_outb[0]; p1_min = reg2_min_sec4_outb[1]; p2_min = reg2_min_sec4_outb[2]; p3_min = reg2_min_sec4_outb[3]; p4_min = reg2_min_sec4_outb[4];
      p0_max = reg2_max_sec4_outb[0]; p1_max = reg2_max_sec4_outb[1]; p2_max = reg2_max_sec4_outb[2]; p3_max = reg2_max_sec4_outb[3]; p4_max = reg2_max_sec4_outb[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg2_min_sec5_outb[0]; p1_min = reg2_min_sec5_outb[1]; p2_min = reg2_min_sec5_outb[2]; p3_min = reg2_min_sec5_outb[3]; p4_min = reg2_min_sec5_outb[4];
      p0_max = reg2_max_sec5_outb[0]; p1_max = reg2_max_sec5_outb[1]; p2_max = reg2_max_sec5_outb[2]; p3_max = reg2_max_sec5_outb[3]; p4_max = reg2_max_sec5_outb[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg2_min_sec6_outb[0]; p1_min = reg2_min_sec6_outb[1]; p2_min = reg2_min_sec6_outb[2]; p3_min = reg2_min_sec6_outb[3]; p4_min = reg2_min_sec6_outb[4];
      p0_max = reg2_max_sec6_outb[0]; p1_max = reg2_max_sec6_outb[1]; p2_max = reg2_max_sec6_outb[2]; p3_max = reg2_max_sec6_outb[3]; p4_max = reg2_max_sec6_outb[4];
    }
  }

  if(region == 3){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg3_min_sec1_inb[0]; p1_min = reg3_min_sec1_inb[1]; p2_min = reg3_min_sec1_inb[2]; p3_min = reg3_min_sec1_inb[3]; p4_min = reg3_min_sec1_inb[4];
      p0_max = reg3_max_sec1_inb[0]; p1_max = reg3_max_sec1_inb[1]; p2_max = reg3_max_sec1_inb[2]; p3_max = reg3_max_sec1_inb[3]; p4_max = reg3_max_sec1_inb[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg3_min_sec2_inb[0]; p1_min = reg3_min_sec2_inb[1]; p2_min = reg3_min_sec2_inb[2]; p3_min = reg3_min_sec2_inb[3]; p4_min = reg3_min_sec2_inb[4];
      p0_max = reg3_max_sec2_inb[0]; p1_max = reg3_max_sec2_inb[1]; p2_max = reg3_max_sec2_inb[2]; p3_max = reg3_max_sec2_inb[3]; p4_max = reg3_max_sec2_inb[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg3_min_sec3_inb[0]; p1_min = reg3_min_sec3_inb[1]; p2_min = reg3_min_sec3_inb[2]; p3_min = reg3_min_sec3_inb[3]; p4_min = reg3_min_sec3_inb[4];
      p0_max = reg3_max_sec3_inb[0]; p1_max = reg3_max_sec3_inb[1]; p2_max = reg3_max_sec3_inb[2]; p3_max = reg3_max_sec3_inb[3]; p4_max = reg3_max_sec3_inb[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg3_min_sec4_inb[0]; p1_min = reg3_min_sec4_inb[1]; p2_min = reg3_min_sec4_inb[2]; p3_min = reg3_min_sec4_inb[3]; p4_min = reg3_min_sec4_inb[4];
      p0_max = reg3_max_sec4_inb[0]; p1_max = reg3_max_sec4_inb[1]; p2_max = reg3_max_sec4_inb[2]; p3_max = reg3_max_sec4_inb[3]; p4_max = reg3_max_sec4_inb[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg3_min_sec5_inb[0]; p1_min = reg3_min_sec5_inb[1]; p2_min = reg3_min_sec5_inb[2]; p3_min = reg3_min_sec5_inb[3]; p4_min = reg3_min_sec5_inb[4];
      p0_max = reg3_max_sec5_inb[0]; p1_max = reg3_max_sec5_inb[1]; p2_max = reg3_max_sec5_inb[2]; p3_max = reg3_max_sec5_inb[3]; p4_max = reg3_max_sec5_inb[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg3_min_sec6_inb[0]; p1_min = reg3_min_sec6_inb[1]; p2_min = reg3_min_sec6_inb[2]; p3_min = reg3_min_sec6_inb[3]; p4_min = reg3_min_sec6_inb[4];
      p0_max = reg3_max_sec6_inb[0]; p1_max = reg3_max_sec6_inb[1]; p2_max = reg3_max_sec6_inb[2]; p3_max = reg3_max_sec6_inb[3]; p4_max = reg3_max_sec6_inb[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg3_min_sec1_outb[0]; p1_min = reg3_min_sec1_outb[1]; p2_min = reg3_min_sec1_outb[2]; p3_min = reg3_min_sec1_outb[3]; p4_min = reg3_min_sec1_outb[4];
      p0_max = reg3_max_sec1_outb[0]; p1_max = reg3_max_sec1_outb[1]; p2_max = reg3_max_sec1_outb[2]; p3_max = reg3_max_sec1_outb[3]; p4_max = reg3_max_sec1_outb[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg3_min_sec2_outb[0]; p1_min = reg3_min_sec2_outb[1]; p2_min = reg3_min_sec2_outb[2]; p3_min = reg3_min_sec2_outb[3]; p4_min = reg3_min_sec2_outb[4];
      p0_max = reg3_max_sec2_outb[0]; p1_max = reg3_max_sec2_outb[1]; p2_max = reg3_max_sec2_outb[2]; p3_max = reg3_max_sec2_outb[3]; p4_max = reg3_max_sec2_outb[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg3_min_sec3_outb[0]; p1_min = reg3_min_sec3_outb[1]; p2_min = reg3_min_sec3_outb[2]; p3_min = reg3_min_sec3_outb[3]; p4_min = reg3_min_sec3_outb[4];
      p0_max = reg3_max_sec3_outb[0]; p1_max = reg3_max_sec3_outb[1]; p2_max = reg3_max_sec3_outb[2]; p3_max = reg3_max_sec3_outb[3]; p4_max = reg3_max_sec3_outb[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg3_min_sec4_outb[0]; p1_min = reg3_min_sec4_outb[1]; p2_min = reg3_min_sec4_outb[2]; p3_min = reg3_min_sec4_outb[3]; p4_min = reg3_min_sec4_outb[4];
      p0_max = reg3_max_sec4_outb[0]; p1_max = reg3_max_sec4_outb[1]; p2_max = reg3_max_sec4_outb[2]; p3_max = reg3_max_sec4_outb[3]; p4_max = reg3_max_sec4_outb[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg3_min_sec5_outb[0]; p1_min = reg3_min_sec5_outb[1]; p2_min = reg3_min_sec5_outb[2]; p3_min = reg3_min_sec5_outb[3]; p4_min = reg3_min_sec5_outb[4];
      p0_max = reg3_max_sec5_outb[0]; p1_max = reg3_max_sec5_outb[1]; p2_max = reg3_max_sec5_outb[2]; p3_max = reg3_max_sec5_outb[3]; p4_max = reg3_max_sec5_outb[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg3_min_sec6_outb[0]; p1_min = reg3_min_sec6_outb[1]; p2_min = reg3_min_sec6_outb[2]; p3_min = reg3_min_sec6_outb[3]; p4_min = reg3_min_sec6_outb[4];
      p0_max = reg3_max_sec6_outb[0]; p1_max = reg3_max_sec6_outb[1]; p2_max = reg3_max_sec6_outb[2]; p3_max = reg3_max_sec6_outb[3]; p4_max = reg3_max_sec6_outb[4];
    }
  }

if(tight == true){
  if(region == 1){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg1_min_sec1_inb_tight[0]; p1_min = reg1_min_sec1_inb_tight[1]; p2_min = reg1_min_sec1_inb_tight[2]; p3_min = reg1_min_sec1_inb_tight[3]; p4_min = reg1_min_sec1_inb_tight[4];
      p0_max = reg1_max_sec1_inb_tight[0]; p1_max = reg1_max_sec1_inb_tight[1]; p2_max = reg1_max_sec1_inb_tight[2]; p3_max = reg1_max_sec1_inb_tight[3]; p4_max = reg1_max_sec1_inb_tight[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg1_min_sec2_inb_tight[0]; p1_min = reg1_min_sec2_inb_tight[1]; p2_min = reg1_min_sec2_inb_tight[2]; p3_min = reg1_min_sec2_inb_tight[3]; p4_min = reg1_min_sec2_inb_tight[4];
      p0_max = reg1_max_sec2_inb_tight[0]; p1_max = reg1_max_sec2_inb_tight[1]; p2_max = reg1_max_sec2_inb_tight[2]; p3_max = reg1_max_sec2_inb_tight[3]; p4_max = reg1_max_sec2_inb_tight[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg1_min_sec3_inb_tight[0]; p1_min = reg1_min_sec3_inb_tight[1]; p2_min = reg1_min_sec3_inb_tight[2]; p3_min = reg1_min_sec3_inb_tight[3]; p4_min = reg1_min_sec3_inb_tight[4];
      p0_max = reg1_max_sec3_inb_tight[0]; p1_max = reg1_max_sec3_inb_tight[1]; p2_max = reg1_max_sec3_inb_tight[2]; p3_max = reg1_max_sec3_inb_tight[3]; p4_max = reg1_max_sec3_inb_tight[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg1_min_sec4_inb_tight[0]; p1_min = reg1_min_sec4_inb_tight[1]; p2_min = reg1_min_sec4_inb_tight[2]; p3_min = reg1_min_sec4_inb_tight[3]; p4_min = reg1_min_sec4_inb_tight[4];
      p0_max = reg1_max_sec4_inb_tight[0]; p1_max = reg1_max_sec4_inb_tight[1]; p2_max = reg1_max_sec4_inb_tight[2]; p3_max = reg1_max_sec4_inb_tight[3]; p4_max = reg1_max_sec4_inb_tight[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg1_min_sec5_inb_tight[0]; p1_min = reg1_min_sec5_inb_tight[1]; p2_min = reg1_min_sec5_inb_tight[2]; p3_min = reg1_min_sec5_inb_tight[3]; p4_min = reg1_min_sec5_inb_tight[4];
      p0_max = reg1_max_sec5_inb_tight[0]; p1_max = reg1_max_sec5_inb_tight[1]; p2_max = reg1_max_sec5_inb_tight[2]; p3_max = reg1_max_sec5_inb_tight[3]; p4_max = reg1_max_sec5_inb_tight[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg1_min_sec6_inb_tight[0]; p1_min = reg1_min_sec6_inb_tight[1]; p2_min = reg1_min_sec6_inb_tight[2]; p3_min = reg1_min_sec6_inb_tight[3]; p4_min = reg1_min_sec6_inb_tight[4];
      p0_max = reg1_max_sec6_inb_tight[0]; p1_max = reg1_max_sec6_inb_tight[1]; p2_max = reg1_max_sec6_inb_tight[2]; p3_max = reg1_max_sec6_inb_tight[3]; p4_max = reg1_max_sec6_inb_tight[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg1_min_sec1_outb_tight[0]; p1_min = reg1_min_sec1_outb_tight[1]; p2_min = reg1_min_sec1_outb_tight[2]; p3_min = reg1_min_sec1_outb_tight[3]; p4_min = reg1_min_sec1_outb_tight[4];
      p0_max = reg1_max_sec1_outb_tight[0]; p1_max = reg1_max_sec1_outb_tight[1]; p2_max = reg1_max_sec1_outb_tight[2]; p3_max = reg1_max_sec1_outb_tight[3]; p4_max = reg1_max_sec1_outb_tight[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg1_min_sec2_outb_tight[0]; p1_min = reg1_min_sec2_outb_tight[1]; p2_min = reg1_min_sec2_outb_tight[2]; p3_min = reg1_min_sec2_outb_tight[3]; p4_min = reg1_min_sec2_outb_tight[4];
      p0_max = reg1_max_sec2_outb_tight[0]; p1_max = reg1_max_sec2_outb_tight[1]; p2_max = reg1_max_sec2_outb_tight[2]; p3_max = reg1_max_sec2_outb_tight[3]; p4_max = reg1_max_sec2_outb_tight[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg1_min_sec3_outb_tight[0]; p1_min = reg1_min_sec3_outb_tight[1]; p2_min = reg1_min_sec3_outb_tight[2]; p3_min = reg1_min_sec3_outb_tight[3]; p4_min = reg1_min_sec3_outb_tight[4];
      p0_max = reg1_max_sec3_outb_tight[0]; p1_max = reg1_max_sec3_outb_tight[1]; p2_max = reg1_max_sec3_outb_tight[2]; p3_max = reg1_max_sec3_outb_tight[3]; p4_max = reg1_max_sec3_outb_tight[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg1_min_sec4_outb_tight[0]; p1_min = reg1_min_sec4_outb_tight[1]; p2_min = reg1_min_sec4_outb_tight[2]; p3_min = reg1_min_sec4_outb_tight[3]; p4_min = reg1_min_sec4_outb_tight[4];
      p0_max = reg1_max_sec4_outb_tight[0]; p1_max = reg1_max_sec4_outb_tight[1]; p2_max = reg1_max_sec4_outb_tight[2]; p3_max = reg1_max_sec4_outb_tight[3]; p4_max = reg1_max_sec4_outb_tight[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg1_min_sec5_outb_tight[0]; p1_min = reg1_min_sec5_outb_tight[1]; p2_min = reg1_min_sec5_outb_tight[2]; p3_min = reg1_min_sec5_outb_tight[3]; p4_min = reg1_min_sec5_outb_tight[4];
      p0_max = reg1_max_sec5_outb_tight[0]; p1_max = reg1_max_sec5_outb_tight[1]; p2_max = reg1_max_sec5_outb_tight[2]; p3_max = reg1_max_sec5_outb_tight[3]; p4_max = reg1_max_sec5_outb_tight[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg1_min_sec6_outb_tight[0]; p1_min = reg1_min_sec6_outb_tight[1]; p2_min = reg1_min_sec6_outb_tight[2]; p3_min = reg1_min_sec6_outb_tight[3]; p4_min = reg1_min_sec6_outb_tight[4];
      p0_max = reg1_max_sec6_outb_tight[0]; p1_max = reg1_max_sec6_outb_tight[1]; p2_max = reg1_max_sec6_outb_tight[2]; p3_max = reg1_max_sec6_outb_tight[3]; p4_max = reg1_max_sec6_outb_tight[4];
    }
  }

  if(region == 2){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg2_min_sec1_inb_tight[0]; p1_min = reg2_min_sec1_inb_tight[1]; p2_min = reg2_min_sec1_inb_tight[2]; p3_min = reg2_min_sec1_inb_tight[3]; p4_min = reg2_min_sec1_inb_tight[4];
      p0_max = reg2_max_sec1_inb_tight[0]; p1_max = reg2_max_sec1_inb_tight[1]; p2_max = reg2_max_sec1_inb_tight[2]; p3_max = reg2_max_sec1_inb_tight[3]; p4_max = reg2_max_sec1_inb_tight[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg2_min_sec2_inb_tight[0]; p1_min = reg2_min_sec2_inb_tight[1]; p2_min = reg2_min_sec2_inb_tight[2]; p3_min = reg2_min_sec2_inb_tight[3]; p4_min = reg2_min_sec2_inb_tight[4];
      p0_max = reg2_max_sec2_inb_tight[0]; p1_max = reg2_max_sec2_inb_tight[1]; p2_max = reg2_max_sec2_inb_tight[2]; p3_max = reg2_max_sec2_inb_tight[3]; p4_max = reg2_max_sec2_inb_tight[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg2_min_sec3_inb_tight[0]; p1_min = reg2_min_sec3_inb_tight[1]; p2_min = reg2_min_sec3_inb_tight[2]; p3_min = reg2_min_sec3_inb_tight[3]; p4_min = reg2_min_sec3_inb_tight[4];
      p0_max = reg2_max_sec3_inb_tight[0]; p1_max = reg2_max_sec3_inb_tight[1]; p2_max = reg2_max_sec3_inb_tight[2]; p3_max = reg2_max_sec3_inb_tight[3]; p4_max = reg2_max_sec3_inb_tight[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg2_min_sec4_inb_tight[0]; p1_min = reg2_min_sec4_inb_tight[1]; p2_min = reg2_min_sec4_inb_tight[2]; p3_min = reg2_min_sec4_inb_tight[3]; p4_min = reg2_min_sec4_inb_tight[4];
      p0_max = reg2_max_sec4_inb_tight[0]; p1_max = reg2_max_sec4_inb_tight[1]; p2_max = reg2_max_sec4_inb_tight[2]; p3_max = reg2_max_sec4_inb_tight[3]; p4_max = reg2_max_sec4_inb_tight[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg2_min_sec5_inb_tight[0]; p1_min = reg2_min_sec5_inb_tight[1]; p2_min = reg2_min_sec5_inb_tight[2]; p3_min = reg2_min_sec5_inb_tight[3]; p4_min = reg2_min_sec5_inb_tight[4];
      p0_max = reg2_max_sec5_inb_tight[0]; p1_max = reg2_max_sec5_inb_tight[1]; p2_max = reg2_max_sec5_inb_tight[2]; p3_max = reg2_max_sec5_inb_tight[3]; p4_max = reg2_max_sec5_inb_tight[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg2_min_sec6_inb_tight[0]; p1_min = reg2_min_sec6_inb_tight[1]; p2_min = reg2_min_sec6_inb_tight[2]; p3_min = reg2_min_sec6_inb_tight[3]; p4_min = reg2_min_sec6_inb_tight[4];
      p0_max = reg2_max_sec6_inb_tight[0]; p1_max = reg2_max_sec6_inb_tight[1]; p2_max = reg2_max_sec6_inb_tight[2]; p3_max = reg2_max_sec6_inb_tight[3]; p4_max = reg2_max_sec6_inb_tight[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg2_min_sec1_outb_tight[0]; p1_min = reg2_min_sec1_outb_tight[1]; p2_min = reg2_min_sec1_outb_tight[2]; p3_min = reg2_min_sec1_outb_tight[3]; p4_min = reg2_min_sec1_outb_tight[4];
      p0_max = reg2_max_sec1_outb_tight[0]; p1_max = reg2_max_sec1_outb_tight[1]; p2_max = reg2_max_sec1_outb_tight[2]; p3_max = reg2_max_sec1_outb_tight[3]; p4_max = reg2_max_sec1_outb_tight[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg2_min_sec2_outb_tight[0]; p1_min = reg2_min_sec2_outb_tight[1]; p2_min = reg2_min_sec2_outb_tight[2]; p3_min = reg2_min_sec2_outb_tight[3]; p4_min = reg2_min_sec2_outb_tight[4];
      p0_max = reg2_max_sec2_outb_tight[0]; p1_max = reg2_max_sec2_outb_tight[1]; p2_max = reg2_max_sec2_outb_tight[2]; p3_max = reg2_max_sec2_outb_tight[3]; p4_max = reg2_max_sec2_outb_tight[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg2_min_sec3_outb_tight[0]; p1_min = reg2_min_sec3_outb_tight[1]; p2_min = reg2_min_sec3_outb_tight[2]; p3_min = reg2_min_sec3_outb_tight[3]; p4_min = reg2_min_sec3_outb_tight[4];
      p0_max = reg2_max_sec3_outb_tight[0]; p1_max = reg2_max_sec3_outb_tight[1]; p2_max = reg2_max_sec3_outb_tight[2]; p3_max = reg2_max_sec3_outb_tight[3]; p4_max = reg2_max_sec3_outb_tight[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg2_min_sec4_outb_tight[0]; p1_min = reg2_min_sec4_outb_tight[1]; p2_min = reg2_min_sec4_outb_tight[2]; p3_min = reg2_min_sec4_outb_tight[3]; p4_min = reg2_min_sec4_outb_tight[4];
      p0_max = reg2_max_sec4_outb_tight[0]; p1_max = reg2_max_sec4_outb_tight[1]; p2_max = reg2_max_sec4_outb_tight[2]; p3_max = reg2_max_sec4_outb_tight[3]; p4_max = reg2_max_sec4_outb_tight[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg2_min_sec5_outb_tight[0]; p1_min = reg2_min_sec5_outb_tight[1]; p2_min = reg2_min_sec5_outb_tight[2]; p3_min = reg2_min_sec5_outb_tight[3]; p4_min = reg2_min_sec5_outb_tight[4];
      p0_max = reg2_max_sec5_outb_tight[0]; p1_max = reg2_max_sec5_outb_tight[1]; p2_max = reg2_max_sec5_outb_tight[2]; p3_max = reg2_max_sec5_outb_tight[3]; p4_max = reg2_max_sec5_outb_tight[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg2_min_sec6_outb_tight[0]; p1_min = reg2_min_sec6_outb_tight[1]; p2_min = reg2_min_sec6_outb_tight[2]; p3_min = reg2_min_sec6_outb_tight[3]; p4_min = reg2_min_sec6_outb_tight[4];
      p0_max = reg2_max_sec6_outb_tight[0]; p1_max = reg2_max_sec6_outb_tight[1]; p2_max = reg2_max_sec6_outb_tight[2]; p3_max = reg2_max_sec6_outb_tight[3]; p4_max = reg2_max_sec6_outb_tight[4];
    }
  }

  if(region == 3){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg3_min_sec1_inb_tight[0]; p1_min = reg3_min_sec1_inb_tight[1]; p2_min = reg3_min_sec1_inb_tight[2]; p3_min = reg3_min_sec1_inb_tight[3]; p4_min = reg3_min_sec1_inb_tight[4];
      p0_max = reg3_max_sec1_inb_tight[0]; p1_max = reg3_max_sec1_inb_tight[1]; p2_max = reg3_max_sec1_inb_tight[2]; p3_max = reg3_max_sec1_inb_tight[3]; p4_max = reg3_max_sec1_inb_tight[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg3_min_sec2_inb_tight[0]; p1_min = reg3_min_sec2_inb_tight[1]; p2_min = reg3_min_sec2_inb_tight[2]; p3_min = reg3_min_sec2_inb_tight[3]; p4_min = reg3_min_sec2_inb_tight[4];
      p0_max = reg3_max_sec2_inb_tight[0]; p1_max = reg3_max_sec2_inb_tight[1]; p2_max = reg3_max_sec2_inb_tight[2]; p3_max = reg3_max_sec2_inb_tight[3]; p4_max = reg3_max_sec2_inb_tight[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg3_min_sec3_inb_tight[0]; p1_min = reg3_min_sec3_inb_tight[1]; p2_min = reg3_min_sec3_inb_tight[2]; p3_min = reg3_min_sec3_inb_tight[3]; p4_min = reg3_min_sec3_inb_tight[4];
      p0_max = reg3_max_sec3_inb_tight[0]; p1_max = reg3_max_sec3_inb_tight[1]; p2_max = reg3_max_sec3_inb_tight[2]; p3_max = reg3_max_sec3_inb_tight[3]; p4_max = reg3_max_sec3_inb_tight[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg3_min_sec4_inb_tight[0]; p1_min = reg3_min_sec4_inb_tight[1]; p2_min = reg3_min_sec4_inb_tight[2]; p3_min = reg3_min_sec4_inb_tight[3]; p4_min = reg3_min_sec4_inb_tight[4];
      p0_max = reg3_max_sec4_inb_tight[0]; p1_max = reg3_max_sec4_inb_tight[1]; p2_max = reg3_max_sec4_inb_tight[2]; p3_max = reg3_max_sec4_inb_tight[3]; p4_max = reg3_max_sec4_inb_tight[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg3_min_sec5_inb_tight[0]; p1_min = reg3_min_sec5_inb_tight[1]; p2_min = reg3_min_sec5_inb_tight[2]; p3_min = reg3_min_sec5_inb_tight[3]; p4_min = reg3_min_sec5_inb_tight[4];
      p0_max = reg3_max_sec5_inb_tight[0]; p1_max = reg3_max_sec5_inb_tight[1]; p2_max = reg3_max_sec5_inb_tight[2]; p3_max = reg3_max_sec5_inb_tight[3]; p4_max = reg3_max_sec5_inb_tight[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg3_min_sec6_inb_tight[0]; p1_min = reg3_min_sec6_inb_tight[1]; p2_min = reg3_min_sec6_inb_tight[2]; p3_min = reg3_min_sec6_inb_tight[3]; p4_min = reg3_min_sec6_inb_tight[4];
      p0_max = reg3_max_sec6_inb_tight[0]; p1_max = reg3_max_sec6_inb_tight[1]; p2_max = reg3_max_sec6_inb_tight[2]; p3_max = reg3_max_sec6_inb_tight[3]; p4_max = reg3_max_sec6_inb_tight[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg3_min_sec1_outb_tight[0]; p1_min = reg3_min_sec1_outb_tight[1]; p2_min = reg3_min_sec1_outb_tight[2]; p3_min = reg3_min_sec1_outb_tight[3]; p4_min = reg3_min_sec1_outb_tight[4];
      p0_max = reg3_max_sec1_outb_tight[0]; p1_max = reg3_max_sec1_outb_tight[1]; p2_max = reg3_max_sec1_outb_tight[2]; p3_max = reg3_max_sec1_outb_tight[3]; p4_max = reg3_max_sec1_outb_tight[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg3_min_sec2_outb_tight[0]; p1_min = reg3_min_sec2_outb_tight[1]; p2_min = reg3_min_sec2_outb_tight[2]; p3_min = reg3_min_sec2_outb_tight[3]; p4_min = reg3_min_sec2_outb_tight[4];
      p0_max = reg3_max_sec2_outb_tight[0]; p1_max = reg3_max_sec2_outb_tight[1]; p2_max = reg3_max_sec2_outb_tight[2]; p3_max = reg3_max_sec2_outb_tight[3]; p4_max = reg3_max_sec2_outb_tight[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg3_min_sec3_outb_tight[0]; p1_min = reg3_min_sec3_outb_tight[1]; p2_min = reg3_min_sec3_outb_tight[2]; p3_min = reg3_min_sec3_outb_tight[3]; p4_min = reg3_min_sec3_outb_tight[4];
      p0_max = reg3_max_sec3_outb_tight[0]; p1_max = reg3_max_sec3_outb_tight[1]; p2_max = reg3_max_sec3_outb_tight[2]; p3_max = reg3_max_sec3_outb_tight[3]; p4_max = reg3_max_sec3_outb_tight[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg3_min_sec4_outb_tight[0]; p1_min = reg3_min_sec4_outb_tight[1]; p2_min = reg3_min_sec4_outb_tight[2]; p3_min = reg3_min_sec4_outb_tight[3]; p4_min = reg3_min_sec4_outb_tight[4];
      p0_max = reg3_max_sec4_outb_tight[0]; p1_max = reg3_max_sec4_outb_tight[1]; p2_max = reg3_max_sec4_outb_tight[2]; p3_max = reg3_max_sec4_outb_tight[3]; p4_max = reg3_max_sec4_outb_tight[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg3_min_sec5_outb_tight[0]; p1_min = reg3_min_sec5_outb_tight[1]; p2_min = reg3_min_sec5_outb_tight[2]; p3_min = reg3_min_sec5_outb_tight[3]; p4_min = reg3_min_sec5_outb_tight[4];
      p0_max = reg3_max_sec5_outb_tight[0]; p1_max = reg3_max_sec5_outb_tight[1]; p2_max = reg3_max_sec5_outb_tight[2]; p3_max = reg3_max_sec5_outb_tight[3]; p4_max = reg3_max_sec5_outb_tight[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg3_min_sec6_outb_tight[0]; p1_min = reg3_min_sec6_outb_tight[1]; p2_min = reg3_min_sec6_outb_tight[2]; p3_min = reg3_min_sec6_outb_tight[3]; p4_min = reg3_min_sec6_outb_tight[4];
      p0_max = reg3_max_sec6_outb_tight[0]; p1_max = reg3_max_sec6_outb_tight[1]; p2_max = reg3_max_sec6_outb_tight[2]; p3_max = reg3_max_sec6_outb_tight[3]; p4_max = reg3_max_sec6_outb_tight[4];
    }
  }
}

if(loose == true){
  if(region == 1){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg1_min_sec1_inb_loose[0]; p1_min = reg1_min_sec1_inb_loose[1]; p2_min = reg1_min_sec1_inb_loose[2]; p3_min = reg1_min_sec1_inb_loose[3]; p4_min = reg1_min_sec1_inb_loose[4];
      p0_max = reg1_max_sec1_inb_loose[0]; p1_max = reg1_max_sec1_inb_loose[1]; p2_max = reg1_max_sec1_inb_loose[2]; p3_max = reg1_max_sec1_inb_loose[3]; p4_max = reg1_max_sec1_inb_loose[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg1_min_sec2_inb_loose[0]; p1_min = reg1_min_sec2_inb_loose[1]; p2_min = reg1_min_sec2_inb_loose[2]; p3_min = reg1_min_sec2_inb_loose[3]; p4_min = reg1_min_sec2_inb_loose[4];
      p0_max = reg1_max_sec2_inb_loose[0]; p1_max = reg1_max_sec2_inb_loose[1]; p2_max = reg1_max_sec2_inb_loose[2]; p3_max = reg1_max_sec2_inb_loose[3]; p4_max = reg1_max_sec2_inb_loose[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg1_min_sec3_inb_loose[0]; p1_min = reg1_min_sec3_inb_loose[1]; p2_min = reg1_min_sec3_inb_loose[2]; p3_min = reg1_min_sec3_inb_loose[3]; p4_min = reg1_min_sec3_inb_loose[4];
      p0_max = reg1_max_sec3_inb_loose[0]; p1_max = reg1_max_sec3_inb_loose[1]; p2_max = reg1_max_sec3_inb_loose[2]; p3_max = reg1_max_sec3_inb_loose[3]; p4_max = reg1_max_sec3_inb_loose[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg1_min_sec4_inb_loose[0]; p1_min = reg1_min_sec4_inb_loose[1]; p2_min = reg1_min_sec4_inb_loose[2]; p3_min = reg1_min_sec4_inb_loose[3]; p4_min = reg1_min_sec4_inb_loose[4];
      p0_max = reg1_max_sec4_inb_loose[0]; p1_max = reg1_max_sec4_inb_loose[1]; p2_max = reg1_max_sec4_inb_loose[2]; p3_max = reg1_max_sec4_inb_loose[3]; p4_max = reg1_max_sec4_inb_loose[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg1_min_sec5_inb_loose[0]; p1_min = reg1_min_sec5_inb_loose[1]; p2_min = reg1_min_sec5_inb_loose[2]; p3_min = reg1_min_sec5_inb_loose[3]; p4_min = reg1_min_sec5_inb_loose[4];
      p0_max = reg1_max_sec5_inb_loose[0]; p1_max = reg1_max_sec5_inb_loose[1]; p2_max = reg1_max_sec5_inb_loose[2]; p3_max = reg1_max_sec5_inb_loose[3]; p4_max = reg1_max_sec5_inb_loose[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg1_min_sec6_inb_loose[0]; p1_min = reg1_min_sec6_inb_loose[1]; p2_min = reg1_min_sec6_inb_loose[2]; p3_min = reg1_min_sec6_inb_loose[3]; p4_min = reg1_min_sec6_inb_loose[4];
      p0_max = reg1_max_sec6_inb_loose[0]; p1_max = reg1_max_sec6_inb_loose[1]; p2_max = reg1_max_sec6_inb_loose[2]; p3_max = reg1_max_sec6_inb_loose[3]; p4_max = reg1_max_sec6_inb_loose[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg1_min_sec1_outb_loose[0]; p1_min = reg1_min_sec1_outb_loose[1]; p2_min = reg1_min_sec1_outb_loose[2]; p3_min = reg1_min_sec1_outb_loose[3]; p4_min = reg1_min_sec1_outb_loose[4];
      p0_max = reg1_max_sec1_outb_loose[0]; p1_max = reg1_max_sec1_outb_loose[1]; p2_max = reg1_max_sec1_outb_loose[2]; p3_max = reg1_max_sec1_outb_loose[3]; p4_max = reg1_max_sec1_outb_loose[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg1_min_sec2_outb_loose[0]; p1_min = reg1_min_sec2_outb_loose[1]; p2_min = reg1_min_sec2_outb_loose[2]; p3_min = reg1_min_sec2_outb_loose[3]; p4_min = reg1_min_sec2_outb_loose[4];
      p0_max = reg1_max_sec2_outb_loose[0]; p1_max = reg1_max_sec2_outb_loose[1]; p2_max = reg1_max_sec2_outb_loose[2]; p3_max = reg1_max_sec2_outb_loose[3]; p4_max = reg1_max_sec2_outb_loose[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg1_min_sec3_outb_loose[0]; p1_min = reg1_min_sec3_outb_loose[1]; p2_min = reg1_min_sec3_outb_loose[2]; p3_min = reg1_min_sec3_outb_loose[3]; p4_min = reg1_min_sec3_outb_loose[4];
      p0_max = reg1_max_sec3_outb_loose[0]; p1_max = reg1_max_sec3_outb_loose[1]; p2_max = reg1_max_sec3_outb_loose[2]; p3_max = reg1_max_sec3_outb_loose[3]; p4_max = reg1_max_sec3_outb_loose[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg1_min_sec4_outb_loose[0]; p1_min = reg1_min_sec4_outb_loose[1]; p2_min = reg1_min_sec4_outb_loose[2]; p3_min = reg1_min_sec4_outb_loose[3]; p4_min = reg1_min_sec4_outb_loose[4];
      p0_max = reg1_max_sec4_outb_loose[0]; p1_max = reg1_max_sec4_outb_loose[1]; p2_max = reg1_max_sec4_outb_loose[2]; p3_max = reg1_max_sec4_outb_loose[3]; p4_max = reg1_max_sec4_outb_loose[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg1_min_sec5_outb_loose[0]; p1_min = reg1_min_sec5_outb_loose[1]; p2_min = reg1_min_sec5_outb_loose[2]; p3_min = reg1_min_sec5_outb_loose[3]; p4_min = reg1_min_sec5_outb_loose[4];
      p0_max = reg1_max_sec5_outb_loose[0]; p1_max = reg1_max_sec5_outb_loose[1]; p2_max = reg1_max_sec5_outb_loose[2]; p3_max = reg1_max_sec5_outb_loose[3]; p4_max = reg1_max_sec5_outb_loose[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg1_min_sec6_outb_loose[0]; p1_min = reg1_min_sec6_outb_loose[1]; p2_min = reg1_min_sec6_outb_loose[2]; p3_min = reg1_min_sec6_outb_loose[3]; p4_min = reg1_min_sec6_outb_loose[4];
      p0_max = reg1_max_sec6_outb_loose[0]; p1_max = reg1_max_sec6_outb_loose[1]; p2_max = reg1_max_sec6_outb_loose[2]; p3_max = reg1_max_sec6_outb_loose[3]; p4_max = reg1_max_sec6_outb_loose[4];
    }
  }

  if(region == 2){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg2_min_sec1_inb_loose[0]; p1_min = reg2_min_sec1_inb_loose[1]; p2_min = reg2_min_sec1_inb_loose[2]; p3_min = reg2_min_sec1_inb_loose[3]; p4_min = reg2_min_sec1_inb_loose[4];
      p0_max = reg2_max_sec1_inb_loose[0]; p1_max = reg2_max_sec1_inb_loose[1]; p2_max = reg2_max_sec1_inb_loose[2]; p3_max = reg2_max_sec1_inb_loose[3]; p4_max = reg2_max_sec1_inb_loose[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg2_min_sec2_inb_loose[0]; p1_min = reg2_min_sec2_inb_loose[1]; p2_min = reg2_min_sec2_inb_loose[2]; p3_min = reg2_min_sec2_inb_loose[3]; p4_min = reg2_min_sec2_inb_loose[4];
      p0_max = reg2_max_sec2_inb_loose[0]; p1_max = reg2_max_sec2_inb_loose[1]; p2_max = reg2_max_sec2_inb_loose[2]; p3_max = reg2_max_sec2_inb_loose[3]; p4_max = reg2_max_sec2_inb_loose[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg2_min_sec3_inb_loose[0]; p1_min = reg2_min_sec3_inb_loose[1]; p2_min = reg2_min_sec3_inb_loose[2]; p3_min = reg2_min_sec3_inb_loose[3]; p4_min = reg2_min_sec3_inb_loose[4];
      p0_max = reg2_max_sec3_inb_loose[0]; p1_max = reg2_max_sec3_inb_loose[1]; p2_max = reg2_max_sec3_inb_loose[2]; p3_max = reg2_max_sec3_inb_loose[3]; p4_max = reg2_max_sec3_inb_loose[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg2_min_sec4_inb_loose[0]; p1_min = reg2_min_sec4_inb_loose[1]; p2_min = reg2_min_sec4_inb_loose[2]; p3_min = reg2_min_sec4_inb_loose[3]; p4_min = reg2_min_sec4_inb_loose[4];
      p0_max = reg2_max_sec4_inb_loose[0]; p1_max = reg2_max_sec4_inb_loose[1]; p2_max = reg2_max_sec4_inb_loose[2]; p3_max = reg2_max_sec4_inb_loose[3]; p4_max = reg2_max_sec4_inb_loose[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg2_min_sec5_inb_loose[0]; p1_min = reg2_min_sec5_inb_loose[1]; p2_min = reg2_min_sec5_inb_loose[2]; p3_min = reg2_min_sec5_inb_loose[3]; p4_min = reg2_min_sec5_inb_loose[4];
      p0_max = reg2_max_sec5_inb_loose[0]; p1_max = reg2_max_sec5_inb_loose[1]; p2_max = reg2_max_sec5_inb_loose[2]; p3_max = reg2_max_sec5_inb_loose[3]; p4_max = reg2_max_sec5_inb_loose[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg2_min_sec6_inb_loose[0]; p1_min = reg2_min_sec6_inb_loose[1]; p2_min = reg2_min_sec6_inb_loose[2]; p3_min = reg2_min_sec6_inb_loose[3]; p4_min = reg2_min_sec6_inb_loose[4];
      p0_max = reg2_max_sec6_inb_loose[0]; p1_max = reg2_max_sec6_inb_loose[1]; p2_max = reg2_max_sec6_inb_loose[2]; p3_max = reg2_max_sec6_inb_loose[3]; p4_max = reg2_max_sec6_inb_loose[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg2_min_sec1_outb_loose[0]; p1_min = reg2_min_sec1_outb_loose[1]; p2_min = reg2_min_sec1_outb_loose[2]; p3_min = reg2_min_sec1_outb_loose[3]; p4_min = reg2_min_sec1_outb_loose[4];
      p0_max = reg2_max_sec1_outb_loose[0]; p1_max = reg2_max_sec1_outb_loose[1]; p2_max = reg2_max_sec1_outb_loose[2]; p3_max = reg2_max_sec1_outb_loose[3]; p4_max = reg2_max_sec1_outb_loose[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg2_min_sec2_outb_loose[0]; p1_min = reg2_min_sec2_outb_loose[1]; p2_min = reg2_min_sec2_outb_loose[2]; p3_min = reg2_min_sec2_outb_loose[3]; p4_min = reg2_min_sec2_outb_loose[4];
      p0_max = reg2_max_sec2_outb_loose[0]; p1_max = reg2_max_sec2_outb_loose[1]; p2_max = reg2_max_sec2_outb_loose[2]; p3_max = reg2_max_sec2_outb_loose[3]; p4_max = reg2_max_sec2_outb_loose[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg2_min_sec3_outb_loose[0]; p1_min = reg2_min_sec3_outb_loose[1]; p2_min = reg2_min_sec3_outb_loose[2]; p3_min = reg2_min_sec3_outb_loose[3]; p4_min = reg2_min_sec3_outb_loose[4];
      p0_max = reg2_max_sec3_outb_loose[0]; p1_max = reg2_max_sec3_outb_loose[1]; p2_max = reg2_max_sec3_outb_loose[2]; p3_max = reg2_max_sec3_outb_loose[3]; p4_max = reg2_max_sec3_outb_loose[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg2_min_sec4_outb_loose[0]; p1_min = reg2_min_sec4_outb_loose[1]; p2_min = reg2_min_sec4_outb_loose[2]; p3_min = reg2_min_sec4_outb_loose[3]; p4_min = reg2_min_sec4_outb_loose[4];
      p0_max = reg2_max_sec4_outb_loose[0]; p1_max = reg2_max_sec4_outb_loose[1]; p2_max = reg2_max_sec4_outb_loose[2]; p3_max = reg2_max_sec4_outb_loose[3]; p4_max = reg2_max_sec4_outb_loose[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg2_min_sec5_outb_loose[0]; p1_min = reg2_min_sec5_outb_loose[1]; p2_min = reg2_min_sec5_outb_loose[2]; p3_min = reg2_min_sec5_outb_loose[3]; p4_min = reg2_min_sec5_outb_loose[4];
      p0_max = reg2_max_sec5_outb_loose[0]; p1_max = reg2_max_sec5_outb_loose[1]; p2_max = reg2_max_sec5_outb_loose[2]; p3_max = reg2_max_sec5_outb_loose[3]; p4_max = reg2_max_sec5_outb_loose[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg2_min_sec6_outb_loose[0]; p1_min = reg2_min_sec6_outb_loose[1]; p2_min = reg2_min_sec6_outb_loose[2]; p3_min = reg2_min_sec6_outb_loose[3]; p4_min = reg2_min_sec6_outb_loose[4];
      p0_max = reg2_max_sec6_outb_loose[0]; p1_max = reg2_max_sec6_outb_loose[1]; p2_max = reg2_max_sec6_outb_loose[2]; p3_max = reg2_max_sec6_outb_loose[3]; p4_max = reg2_max_sec6_outb_loose[4];
    }
  }

  if(region == 3){
    if(part_DC_sector[j] == 1 && inbending == true){
      p0_min = reg3_min_sec1_inb_loose[0]; p1_min = reg3_min_sec1_inb_loose[1]; p2_min = reg3_min_sec1_inb_loose[2]; p3_min = reg3_min_sec1_inb_loose[3]; p4_min = reg3_min_sec1_inb_loose[4];
      p0_max = reg3_max_sec1_inb_loose[0]; p1_max = reg3_max_sec1_inb_loose[1]; p2_max = reg3_max_sec1_inb_loose[2]; p3_max = reg3_max_sec1_inb_loose[3]; p4_max = reg3_max_sec1_inb_loose[4];
    }
    if(part_DC_sector[j] == 2 && inbending == true){
      p0_min = reg3_min_sec2_inb_loose[0]; p1_min = reg3_min_sec2_inb_loose[1]; p2_min = reg3_min_sec2_inb_loose[2]; p3_min = reg3_min_sec2_inb_loose[3]; p4_min = reg3_min_sec2_inb_loose[4];
      p0_max = reg3_max_sec2_inb_loose[0]; p1_max = reg3_max_sec2_inb_loose[1]; p2_max = reg3_max_sec2_inb_loose[2]; p3_max = reg3_max_sec2_inb_loose[3]; p4_max = reg3_max_sec2_inb_loose[4];
    }
    if(part_DC_sector[j] == 3 && inbending == true){
      p0_min = reg3_min_sec3_inb_loose[0]; p1_min = reg3_min_sec3_inb_loose[1]; p2_min = reg3_min_sec3_inb_loose[2]; p3_min = reg3_min_sec3_inb_loose[3]; p4_min = reg3_min_sec3_inb_loose[4];
      p0_max = reg3_max_sec3_inb_loose[0]; p1_max = reg3_max_sec3_inb_loose[1]; p2_max = reg3_max_sec3_inb_loose[2]; p3_max = reg3_max_sec3_inb_loose[3]; p4_max = reg3_max_sec3_inb_loose[4];
    }
    if(part_DC_sector[j] == 4 && inbending == true){
      p0_min = reg3_min_sec4_inb_loose[0]; p1_min = reg3_min_sec4_inb_loose[1]; p2_min = reg3_min_sec4_inb_loose[2]; p3_min = reg3_min_sec4_inb_loose[3]; p4_min = reg3_min_sec4_inb_loose[4];
      p0_max = reg3_max_sec4_inb_loose[0]; p1_max = reg3_max_sec4_inb_loose[1]; p2_max = reg3_max_sec4_inb_loose[2]; p3_max = reg3_max_sec4_inb_loose[3]; p4_max = reg3_max_sec4_inb_loose[4];
    }
    if(part_DC_sector[j] == 5 && inbending == true){
      p0_min = reg3_min_sec5_inb_loose[0]; p1_min = reg3_min_sec5_inb_loose[1]; p2_min = reg3_min_sec5_inb_loose[2]; p3_min = reg3_min_sec5_inb_loose[3]; p4_min = reg3_min_sec5_inb_loose[4];
      p0_max = reg3_max_sec5_inb_loose[0]; p1_max = reg3_max_sec5_inb_loose[1]; p2_max = reg3_max_sec5_inb_loose[2]; p3_max = reg3_max_sec5_inb_loose[3]; p4_max = reg3_max_sec5_inb_loose[4];
    }
    if(part_DC_sector[j] == 6 && inbending == true){
      p0_min = reg3_min_sec6_inb_loose[0]; p1_min = reg3_min_sec6_inb_loose[1]; p2_min = reg3_min_sec6_inb_loose[2]; p3_min = reg3_min_sec6_inb_loose[3]; p4_min = reg3_min_sec6_inb_loose[4];
      p0_max = reg3_max_sec6_inb_loose[0]; p1_max = reg3_max_sec6_inb_loose[1]; p2_max = reg3_max_sec6_inb_loose[2]; p3_max = reg3_max_sec6_inb_loose[3]; p4_max = reg3_max_sec6_inb_loose[4];
    }
    if(part_DC_sector[j] == 1 && outbending == true){
      p0_min = reg3_min_sec1_outb_loose[0]; p1_min = reg3_min_sec1_outb_loose[1]; p2_min = reg3_min_sec1_outb_loose[2]; p3_min = reg3_min_sec1_outb_loose[3]; p4_min = reg3_min_sec1_outb_loose[4];
      p0_max = reg3_max_sec1_outb_loose[0]; p1_max = reg3_max_sec1_outb_loose[1]; p2_max = reg3_max_sec1_outb_loose[2]; p3_max = reg3_max_sec1_outb_loose[3]; p4_max = reg3_max_sec1_outb_loose[4];
    }
    if(part_DC_sector[j] == 2 && outbending == true){
      p0_min = reg3_min_sec2_outb_loose[0]; p1_min = reg3_min_sec2_outb_loose[1]; p2_min = reg3_min_sec2_outb_loose[2]; p3_min = reg3_min_sec2_outb_loose[3]; p4_min = reg3_min_sec2_outb_loose[4];
      p0_max = reg3_max_sec2_outb_loose[0]; p1_max = reg3_max_sec2_outb_loose[1]; p2_max = reg3_max_sec2_outb_loose[2]; p3_max = reg3_max_sec2_outb_loose[3]; p4_max = reg3_max_sec2_outb_loose[4];
    }
    if(part_DC_sector[j] == 3 && outbending == true){
      p0_min = reg3_min_sec3_outb_loose[0]; p1_min = reg3_min_sec3_outb_loose[1]; p2_min = reg3_min_sec3_outb_loose[2]; p3_min = reg3_min_sec3_outb_loose[3]; p4_min = reg3_min_sec3_outb_loose[4];
      p0_max = reg3_max_sec3_outb_loose[0]; p1_max = reg3_max_sec3_outb_loose[1]; p2_max = reg3_max_sec3_outb_loose[2]; p3_max = reg3_max_sec3_outb_loose[3]; p4_max = reg3_max_sec3_outb_loose[4];
    }
    if(part_DC_sector[j] == 4 && outbending == true){
      p0_min = reg3_min_sec4_outb_loose[0]; p1_min = reg3_min_sec4_outb_loose[1]; p2_min = reg3_min_sec4_outb_loose[2]; p3_min = reg3_min_sec4_outb_loose[3]; p4_min = reg3_min_sec4_outb_loose[4];
      p0_max = reg3_max_sec4_outb_loose[0]; p1_max = reg3_max_sec4_outb_loose[1]; p2_max = reg3_max_sec4_outb_loose[2]; p3_max = reg3_max_sec4_outb_loose[3]; p4_max = reg3_max_sec4_outb_loose[4];
    }
    if(part_DC_sector[j] == 5 && outbending == true){
      p0_min = reg3_min_sec5_outb_loose[0]; p1_min = reg3_min_sec5_outb_loose[1]; p2_min = reg3_min_sec5_outb_loose[2]; p3_min = reg3_min_sec5_outb_loose[3]; p4_min = reg3_min_sec5_outb_loose[4];
      p0_max = reg3_max_sec5_outb_loose[0]; p1_max = reg3_max_sec5_outb_loose[1]; p2_max = reg3_max_sec5_outb_loose[2]; p3_max = reg3_max_sec5_outb_loose[3]; p4_max = reg3_max_sec5_outb_loose[4];
    }
    if(part_DC_sector[j] == 6 && outbending == true){
      p0_min = reg3_min_sec6_outb_loose[0]; p1_min = reg3_min_sec6_outb_loose[1]; p2_min = reg3_min_sec6_outb_loose[2]; p3_min = reg3_min_sec6_outb_loose[3]; p4_min = reg3_min_sec6_outb_loose[4];
      p0_max = reg3_max_sec6_outb_loose[0]; p1_max = reg3_max_sec6_outb_loose[1]; p2_max = reg3_max_sec6_outb_loose[2]; p3_max = reg3_max_sec6_outb_loose[3]; p4_max = reg3_max_sec6_outb_loose[4];
    }
  }
}

  double phi_DC_min = p0_min + p1_min * log(theta_DC) + p2_min * theta_DC + p3_min * theta_DC*theta_DC + p4_min * theta_DC*theta_DC*theta_DC;
  double phi_DC_max = p0_max + p1_max * log(theta_DC) + p2_max * theta_DC + p3_max * theta_DC*theta_DC + p4_max * theta_DC*theta_DC*theta_DC;

  if(phi_DC_min < -25.5) phi_DC_min = -25.5;
  if(phi_DC_max > +25.5) phi_DC_max = +25.5;

  if(phi_DC > phi_DC_min && phi_DC < phi_DC_max) return true;
  else return false;
}



/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 2.2 DC fiducial cuts for the 3 regions based on the chi2/NDF value of the tracks (for elctrons and hadrons)
///     - The cut checks at which point the chi2/NDF value starts to increase
///     - works well for hadrons but for electrons no clear increase can be observed in many bins, which leads to soem uncertainty


bool FiducialCuts::DC_fiducial_cut_chi2(int j, int region, int part_pid){

  //fitted values
  const double maxparams[6][6][3][4] = {
      {{{-35.1716, 25.102, -0.750281, 5.34679e-05},
        {-39.1633, 28.5551, -1.13429, 0.00419047},
        {-33.7705, 24.8068, -0.811239, 0.00138345}},
       {{-36.2389, 26.7979, -1.08147, 0.0050898},
        {-43.643, 31.6783, -1.49203, 0.00872922},
        {-54.4042, 40.6516, -2.52393, 0.0205649}},
       {{-38.3238, 26.1667, -0.777077, 0.000264835},
        {-34.2011, 24.2843, -0.696392, 3.75866e-12},
        {-36.4636, 25.8712, -0.786592, 2.24421e-10}},
       {{-31.8019, 23.154, -0.653992, 2.69968e-05},
        {-34.6637, 24.6043, -0.714901, 2.02675e-10},
        {-36.7209, 26.2469, -0.828638, 0.000340435}},
       {{-33.4016, 24.6901, -0.779889, 0.000430557},
        {-35.4583, 24.7491, -0.707953, 2.18559e-10},
        {-37.7335, 28.1547, -1.1986, 0.00582395}},
       {{-34.7808, 24.6988, -0.719936, 5.73299e-10},
        {-54.5797, 40.9138, -2.57493, 0.0213354},
        {-38.4972, 28.3142, -1.21741, 0.00640373}}},
      {{{-2.25358e-08, 12.631, -0.767619, 0.00739811},
        {-8.09501, 15.9098, -0.844083, 0.00667995},
        {-1.48113e-06, 12.2061, -0.73167, 0.0074309}},
       {{-2.10872e-07, 12.6689, -0.765156, 0.00720044},
        {-4.88862, 14.0376, -0.687202, 0.00506307},
        {-4.59793e-06, 11.5553, -0.591469, 0.00536957}},
       {{-1.13504e-08, 12.6011, -0.746025, 0.00687498},
        {-6.97884, 15.1788, -0.765889, 0.00570532},
        {-1.29468, 12.3844, -0.667561, 0.00619226}},
       {{-2.91953e-09, 13.883, -0.999624, 0.0104257},
        {-4.9855, 13.8864, -0.661348, 0.0048371},
        {-2.29438e-08, 11.8341, -0.668486, 0.00669247}},
       {{-2.02824e-08, 13.3855, -0.91158, 0.00926769},
        {-3.29092e-08, 10.8294, -0.382323, 0.00178367},
        {-4.59027e-06, 11.9414, -0.663872, 0.00625769}},
       {{-3.73322e-09, 12.6126, -0.723548, 0.0062217},
        {-4.56248, 14.1574, -0.727805, 0.00560108},
        {-2.39381e-08, 12.0663, -0.6651, 0.00602544}}},
      {{{-1.45923e-08, 13.0297, -0.828302, 0.00795271},
        {-5.41905, 13.2753, -0.503236, 0.00255607},
        {-3.67719, 12.1358, -0.462905, 0.00308219}},
       {{-9.953e-10, 11.549, -0.52816, 0.00378771},
        {-8.47154, 15.9863, -0.826166, 0.0062936},
        {-6.43715, 13.9081, -0.618535, 0.0046102}},
       {{-4.68458e-08, 12.9481, -0.781613, 0.00689754},
        {-3.46617, 12.2786, -0.440121, 0.00205448},
        {-4.43519, 10.9372, -0.210059, 3.69283e-10}},
       {{-4.18414e-07, 13.1542, -0.811251, 0.00714402},
        {-4.63166, 13.7769, -0.657207, 0.0047586},
        {-1.99278e-05, 11.3993, -0.575232, 0.00532141}},
       {{-7.07189e-10, 13.2814, -0.88476, 0.00874389},
        {-5.08373, 14.4384, -0.750795, 0.00586116},
        {-6.9642e-05, 9.50651, -0.189316, 3.07274e-06}},
       {{-5.85515e-08, 12.5116, -0.688741, 0.00557297},
        {-1.86306, 11.985, -0.482567, 0.00279836},
        {-4.94295e-07, 10.1342, -0.316715, 0.00176254}}},
      {{{-0.0157256, 11.1508, -0.415185, 0.00186904},
        {-13.6561, 19.4418, -1.15773, 0.00989432},
        {-6.24969e-07, 10.5776, -0.329325, 0.00103488}},
       {{-2.5686e-08, 11.4797, -0.476772, 0.00264288},
        {-0.0475099, 10.1207, -0.244786, 3.13032e-06},
        {-4.6875e-07, 12.019, -0.63598, 0.00543214}},
       {{-0.00702545, 11.1294, -0.407207, 0.00171263},
        {-7.27687, 15.5, -0.807858, 0.0062086},
        {-5.15078, 12.6368, -0.348584, 9.2687e-12}},
       {{-8.14106e-08, 13.28, -0.818164, 0.00703758},
        {-7.60722, 14.4871, -0.588662, 0.00326244},
        {-1.70764e-06, 12.0413, -0.63961, 0.00541784}},
       {{-1.09281, 11.5573, -0.41311, 0.00155228},
        {-3.71599, 12.8335, -0.521472, 0.00296792},
        {-0.000410815, 12.4833, -0.72999, 0.0066601}},
       {{-0.652641, 12.2766, -0.554202, 0.00314615},
        {-8.42824, 15.5087, -0.710609, 0.00447051},
        {-14.9692, 21.5885, -1.47528, 0.0136615}}},
      {{{-5.58945, 17.4004, -1.34516, 0.0142099},
        {-14.9585, 20.4538, -1.25118, 0.0106617},
        {-12.0069, 16.4545, -0.727162, 0.00495418}},
       {{-7.03048, 17.3519, -1.1831, 0.0111308},
        {-7.30641, 15.8503, -0.850952, 0.00648446},
        {-10.2549, 15.6139, -0.648352, 0.00380506}},
       {{-9.73111e-09, 13.498, -0.932479, 0.00939708},
        {-8.38053, 15.5588, -0.711323, 0.00433827},
        {-12.3097, 16.6403, -0.741362, 0.0050708}},
       {{-7.38905, 17.2652, -1.15517, 0.0109165},
        {-1.11835e-07, 10.4637, -0.301972, 0.000612754},
        {-12.2182, 17.4958, -0.919555, 0.00747512}},
       {{-0.492676, 14.4148, -1.0959, 0.0116708},
        {-5.34309, 14.3258, -0.691954, 0.00480109},
        {-12.5443, 16.1047, -0.59594, 0.00280171}},
       {{-4.08375e-07, 12.2846, -0.655278, 0.00525956},
        {-8.93101, 16.4266, -0.861853, 0.00644623},
        {-11.8406, 17.0417, -0.826301, 0.00596028}}},
      {{{-9.29415, 16.5566, -0.831923, 0.00562504},
        {-0.954483, 10.5813, -0.265766, 3.24615e-05},
        {-6.87423, 14.892, -0.76495, 0.00639603}},
       {{-18.8913, 19.3123, -0.711917, 0.00227889},
        {-13.9788, 18.5678, -0.940183, 0.00664397},
        {-11.7696, 18.3415, -1.04368, 0.0083506}},
       {{-3.82873, 12.7727, -0.425968, 0.000789835},
        {-9.81221, 14.6531, -0.471092, 0.00131406},
        {-14.2392, 15.9895, -0.430525, 2.20712e-12}},
       {{-1.76975e-07, 11.4006, -0.420134, 0.00141302},
        {-3.11764, 10.9707, -0.245823, 2.23044e-12},
        {-17.6005, 22.2881, -1.39992, 0.0117791}},
       {{-0.767518, 11.6824, -0.456275, 0.00214005},
        {-5.28047, 12.65, -0.350658, 9.80081e-05},
        {-0.0888832, 11.508, -0.49197, 0.00301269}},
       {{-4.72388, 15.8507, -1.00574, 0.00876768},
        {-2.80649, 11.4056, -0.301812, 0.000190262},
        {-13.0484, 18.665, -1.08614, 0.00960977}}}};
  const double minparams[6][6][3][4] = {
      {{{37.289, -27.5201, 1.12866, -0.00526111},
        {45.3103, -33.5226, 1.72923, -0.0114495},
        {61.5709, -47.6158, 3.4295, -0.0316429}},
       {{36.6259, -27.4064, 1.16617, -0.00604629},
        {50.3751, -37.5848, 2.19621, -0.0169241},
        {35.1563, -26.514, 1.09795, -0.00545864}},
       {{27.2367, -20.3068, 0.517752, -0.000335432},
        {39.0489, -28.6903, 1.24306, -0.0065226},
        {41.0208, -30.0339, 1.30776, -0.00626721}},
       {{29.261, -21.7041, 0.613556, -0.000774652},
        {39.5304, -29.1388, 1.34116, -0.00823818},
        {44.5313, -33.4056, 1.77581, -0.0123965}},
       {{36.5659, -25.119, 0.714074, -2.65397e-11},
        {31.6524, -22.6934, 0.613977, -5.46634e-10},
        {34.7312, -24.9901, 0.749061, -1.22922e-09}},
       {{33.154, -23.8803, 0.685794, -1.13236e-10},
        {42.6731, -31.0799, 1.40425, -0.00730816},
        {46.4732, -35.6988, 2.10144, -0.0164771}}},
      {{{2.40451, -15.0848, 1.05504, -0.0103356},
        {8.93825, -16.5995, 0.925874, -0.00767902},
        {7.23439e-08, -12.5963, 0.814574, -0.00864749}},
       {{6.2953e-07, -12.6365, 0.732206, -0.00639165},
        {12.6866, -18.7831, 1.0952, -0.00923029},
        {3.12805e-07, -12.5395, 0.795535, -0.00828991}},
       {{2.69495, -14.8778, 1.00751, -0.00975373},
        {6.05446, -14.6778, 0.767457, -0.00636729},
        {3.94741e-07, -11.1038, 0.524109, -0.00471514}},
       {{2.31558e-07, -11.5073, 0.494316, -0.00303611},
        {5.66995, -14.5948, 0.740956, -0.00561851},
        {4.40475e-06, -9.57062, 0.20354, -0.000213213}},
       {{2.74277e-08, -13.3573, 0.886651, -0.00857992},
        {9.98849e-05, -11.524, 0.531486, -0.00391441},
        {8.50811e-07, -9.72224, 0.240264, -0.000781498}},
       {{6.9021e-08, -11.8859, 0.53864, -0.00325092},
        {10.0169, -16.9153, 0.921593, -0.00752414},
        {9.90518e-07, -11.9578, 0.697029, -0.00717645}}},
      {{{6.87966e-10, -12.8497, 0.757379, -0.00651612},
        {16.2087, -19.3776, 0.951508, -0.00645029},
        {14.513, -18.8625, 1.05542, -0.00918985}},
       {{1.07197e-07, -12.5469, 0.703086, -0.00585238},
        {0.0871522, -9.22628, 0.159628, -0.000343326},
        {12.1181, -17.5575, 0.940249, -0.00788125}},
       {{2.10191e-09, -12.2782, 0.661926, -0.00555279},
        {12.5105, -17.9998, 0.951807, -0.00732845},
        {12.8043, -17.8322, 0.972401, -0.00841528}},
       {{8.11926e-10, -12.7225, 0.737941, -0.00647355},
        {7.50649, -15.987, 0.889398, -0.00729282},
        {0.174541, -10.0266, 0.306882, -0.00186093}},
       {{3.81202e-09, -12.0926, 0.598943, -0.00430458},
        {8.72368, -17.2511, 1.06348, -0.00953327},
        {1.5205, -9.86713, 0.183806, -6.40377e-12}},
       {{1.37378e-07, -12.9247, 0.769722, -0.00664936},
        {8.53877, -16.6167, 0.946138, -0.00788745},
        {8.47417, -14.3897, 0.581492, -0.00387111}}},
      {{{2.50079e-07, -12.5209, 0.678491, -0.00528954},
        {12.6171, -18.4205, 1.01802, -0.00807702},
        {10.4903, -18.0981, 1.10546, -0.00971519}},
       {{5.87069e-07, -12.0075, 0.585538, -0.00416654},
        {11.1348, -17.5468, 0.943652, -0.00729083},
        {0.949201, -10.5869, 0.267536, -6.04802e-05}},
       {{1.14857, -11.1478, 0.345528, -0.000841836},
        {10.9482, -17.1647, 0.909605, -0.00722404},
        {8.7569e-08, -10.4446, 0.316302, -0.00101964}},
       {{1.09759e-06, -11.5019, 0.48435, -0.00277852},
        {0.637937, -10.7065, 0.316211, -0.000801127},
        {5.67144e-07, -12.88, 0.831252, -0.00835441}},
       {{1.68853, -11.2582, 0.308152, -7.81686e-12},
        {9.44238, -17.1892, 1.00561, -0.00864837},
        {1.20713e-07, -12.2246, 0.669321, -0.0057622}},
       {{0.00217558, -10.8858, 0.347928, -0.000790679},
        {11.8583, -17.6423, 0.923581, -0.00703041},
        {3.24078, -13.4024, 0.668777, -0.00504175}}},
      {{{6.04158, -16.8155, 1.13335, -0.0105359},
        {8.24786, -17.0204, 1.05097, -0.00941875},
        {11.7617, -17.202, 0.864472, -0.00649032}},
       {{3.70947, -13.0663, 0.513818, -0.00222627},
        {16.7022, -21.9618, 1.42869, -0.012705},
        {6.8993, -14.8192, 0.740813, -0.00585407}},
       {{2.18472e-06, -11.9461, 0.583354, -0.00423414},
        {6.51489e-07, -10.5669, 0.353028, -0.00166977},
        {12.5113, -16.5038, 0.709888, -0.00471964}},
       {{0.812719, -11.3245, 0.390183, -0.00134086},
        {2.97251, -11.9374, 0.338592, -4.36096e-13},
        {13.8844, -17.5707, 0.818446, -0.00581811}},
       {{1.55496, -14.4569, 0.949497, -0.00857237},
        {0.34359, -10.5041, 0.286497, -0.000346977},
        {14.4141, -18.7457, 1.01652, -0.00845189}},
       {{1.26317e-08, -11.1424, 0.434251, -0.00236267},
        {6.58119, -15.8546, 0.930324, -0.00801288},
        {4.41865, -11.1991, 0.234652, -7.43723e-10}}},
      {{{6.87926, -12.8949, 0.334733, -6.38494e-06},
        {35.2336, -32.2007, 2.21489, -0.020555},
        {6.80949, -16.8945, 1.19056, -0.0127558}},
       {{0.95782, -12.4625, 0.599979, -0.00405342},
        {20.4051, -23.1936, 1.42408, -0.0120792},
        {10.277, -16.1457, 0.785186, -0.00612069}},
       {{0.236196, -11.6165, 0.458613, -0.002018},
        {12.8771, -19.6785, 1.26163, -0.0115917},
        {5.21194e-08, -12.551, 0.78718, -0.00794713}},
       {{8.40778, -14.9001, 0.534967, -0.00147246},
        {15.9376, -20.9945, 1.2908, -0.0110556},
        {10.4773, -16.2238, 0.783386, -0.00593478}},
       {{3.21187, -12.1221, 0.348938, -8.70415e-14},
        {13.8983, -19.1128, 1.04727, -0.00797426},
        {11.6342, -18.8428, 1.18853, -0.0107619}},
       {{3.7311, -12.4292, 0.419345, -0.00134704},
        {6.92884, -13.2494, 0.391862, -0.000767396},
        {5.5939, -14.4175, 0.729195, -0.00568477}}}};


  double theta_DCr = 5000;
  double phi_DCr_raw = 5000;

  switch (region)
  {
  case 1:
    theta_DCr = 180 / Pival * acos(part_DC_c1z[j] / sqrt(pow(part_DC_c1x[j], 2) + pow(part_DC_c1y[j], 2) + pow(part_DC_c1z[j], 2)));
    phi_DCr_raw = 180 / Pival * atan2(part_DC_c1y[j] / sqrt(pow(part_DC_c1x[j], 2) + pow(part_DC_c1y[j], 2) + pow(part_DC_c1z[j], 2)), part_DC_c1x[j] / sqrt(pow(part_DC_c1x[j], 2) + pow(part_DC_c1y[j], 2) + pow(part_DC_c1z[j], 2)));
    break;

  case 2:
    theta_DCr = 180 / Pival * acos(part_DC_c2z[j] / sqrt(pow(part_DC_c2x[j], 2) + pow(part_DC_c2y[j], 2) + pow(part_DC_c2z[j], 2)));
    phi_DCr_raw = 180 / Pival * atan2(part_DC_c2y[j] / sqrt(pow(part_DC_c2x[j], 2) + pow(part_DC_c2y[j], 2) + pow(part_DC_c2z[j], 2)), part_DC_c2x[j] / sqrt(pow(part_DC_c2x[j], 2) + pow(part_DC_c2y[j], 2) + pow(part_DC_c2z[j], 2)));
    break;

  case 3:
    theta_DCr = 180 / Pival * acos(part_DC_c3z[j] / sqrt(pow(part_DC_c3x[j], 2) + pow(part_DC_c3y[j], 2) + pow(part_DC_c3z[j], 2)));
    phi_DCr_raw = 180 / Pival * atan2(part_DC_c3y[j] / sqrt(pow(part_DC_c3x[j], 2) + pow(part_DC_c3y[j], 2) + pow(part_DC_c3z[j], 2)), part_DC_c3x[j] / sqrt(pow(part_DC_c3x[j], 2) + pow(part_DC_c3y[j], 2) + pow(part_DC_c3z[j], 2)));
    break;

  default:
    return false;
    break;
  }

  double phi_DCr = 5000;
  if (part_DC_sector[j] == 1) phi_DCr = phi_DCr_raw;
  if (part_DC_sector[j] == 2) phi_DCr = phi_DCr_raw - 60;
  if (part_DC_sector[j] == 3) phi_DCr = phi_DCr_raw - 120;
  if (part_DC_sector[j] == 4 && phi_DCr_raw > 0) phi_DCr = phi_DCr_raw - 180;
  if (part_DC_sector[j] == 4 && phi_DCr_raw < 0) phi_DCr = phi_DCr_raw + 180;
  if (part_DC_sector[j] == 5) phi_DCr = phi_DCr_raw + 120;
  if (part_DC_sector[j] == 6) phi_DCr = phi_DCr_raw + 60;

  int pid = 0;

  switch (part_pid){

  case 11:
    pid = 0;
    break;
  case 2212:
    pid = 1;
    break;
  case 211:
    pid = 2;
    break;
  case -211:
    pid = 3;
    break;
  case 321:
    pid = 4;
    break;
  case -321:
    pid = 5;
    if (part_DC_sector[j] == 6 || (part_DC_sector[j] == 5 && region == 3))  // use K+ cuts in some cases
    {
      pid = 4;
    }
    break;

  default:
    return false;
    break;
  }

  double calc_phi_min = minparams[pid][part_DC_sector[j]][region][0] + minparams[pid][part_DC_sector[j]][region][1] * std::log(theta_DCr) 
                        + minparams[pid][part_DC_sector[j]][region][2] * theta_DCr + minparams[pid][part_DC_sector[j]][region][3] * theta_DCr * theta_DCr;

  double calc_phi_max = maxparams[pid][part_DC_sector[j]][region][0] + maxparams[pid][part_DC_sector[j]][region][1] * std::log(theta_DCr) 
                        + maxparams[pid][part_DC_sector[j]][region][2] * theta_DCr + maxparams[pid][part_DC_sector[j]][region][3] * theta_DCr * theta_DCr;

  return ((phi_DCr > calc_phi_min) && (phi_DCr < calc_phi_max));
}


