#include "FiducialCuts.h"

// dihbsa implementation of Stefan Diehl's fiducial volume cut functions


ClassImp(FiducialCuts)


FiducialCuts::FiducialCuts() {
  enableFiducialCut = false; 
  debug = true;
};

FiducialCuts::~FiducialCuts() {};


bool FiducialCuts::SetSwitches(int lev) {

  // set cut level
  tight=false;
  medium=false;
  loose=false;
  switch(lev) {
    case cutTight: tight=true; break;
    case cutMedium: medium=true; break;
    case cutLoose: loose=true; break;
    default: return ErrPrint("level unknown");
  };

  // set torus setting
  inbending=false;
  outbending=false;
  switch(torus) {
    case -1: inbending=true; break;
    case  1: outbending=true; break;
    default: return ErrPrint("torus unknown");
  };


  return true;
};


// check detector IDs for DC
bool FiducialCuts::CheckDCid() {
  if(dcTrackDetector!=6) return false;/*ErrPrint("dcTrackDetector unknown");*/
  for(int r=0; r<nReg; r++) {
    if(dcTrajDetector[r]!=6) return false;/*ErrPrint("dcTrajDetector unknown");*/
    if(dcTrajLayer[r]!=regLayer(r)) {
      sprintf(msg,"dcTrajLayer[r%d]=%d is not correct",r+1,dcTrajLayer[r]);
      return false;/*ErrPrint(msg);*/
    };
  };
  return true;
};

/// PCAL fiducial cuts:

bool FiducialCuts::EC_hit_position_fiducial_cut(int level){

  success = this->SetSwitches(level);
  if(!success) return false;
  if(pcalLayer!=1) return false;/*ErrPrint("pcalLayer unknown");*/


// Cut using the natural directions of the scintillator bars/ fibers:

   
  /// v + w is going from the side to the back end of the PCAL, u is going from side to side
  /// 1 scintillator bar is 4.5 cm wide. In the outer regions (back) double bars are used.

  ///////////////////////////////////////////////////////////////////
  /// inbending:

  // tight: only background outside 3 sigma, medium: 10 % outside 3 sigma, loose: 50% outside 3 sigma 
  double min_u_tight_inb[] = {42, 32, 38, 27.5, 32, 29};
  double min_u_med_inb[]   = {33, 26, 34, 22,   27, 25};
  double min_u_loose_inb[] = {28, 22, 30, 18,   22, 22};

  // u shows a slight fall of the sampling fraction for high values
  double max_u_tight_inb[] = {398, 398, 398, 398, 398, 398}; 
  double max_u_med_inb[]   = {408, 408, 408, 408, 408, 408}; 
  double max_u_loose_inb[] = {420, 420, 420, 420, 420, 420}; 

  // tight: only background outside 3 sigma, medium: 10 % outside 3 sigma, loose: 50% outside 3 sigma 
  double min_v_tight_inb[] = {18.0, 12.0, 19.5,  15.5,  20.0, 13.0};
  double min_v_med_inb[]   = {16.0, 10.5, 17.0,  14.25, 18.0, 11.0};
  double min_v_loose_inb[] = {10.25, 8.0, 12.75, 12.5,  13.25, 9.0};

  // the maximum of v is never reached
  double max_v_tight_inb[] = {400, 400, 400, 400, 400, 400};
  double max_v_med_inb[]   = {400, 400, 400, 400, 400, 400};
  double max_v_loose_inb[] = {400, 400, 400, 400, 400, 400};

  // tight: only background outside 3 sigma, medium: 10 % outside 3 sigma, loose: 50% outside 3 sigma 
  double min_w_tight_inb[] = {14.0, 18.7, 18.7,  12.0, 16.0, 13.0};
  double min_w_med_inb[]   = {11.0, 17.5, 16.25, 7.5,  14.5, 9.25};
  double min_w_loose_inb[] = {7.25, 11.0, 13.0,  5.5,  10.0, 6.0};

  // the maximum of w is never reached
  double max_w_tight_inb[] = {400, 400, 400, 400, 400, 400};
  double max_w_med_inb[]   = {400, 400, 400, 400, 400, 400};
  double max_w_loose_inb[] = {400, 400, 400, 400, 400, 400};


  ///////////////////////////////////////////////////////////////////////
  /// outbending (not adjusted up to now, same as inbending!):

  // tight: only background outside 3 sigma, medium: 10 % outside 3 sigma, loose: 50% outside 3 sigma 
  double min_u_tight_out[] = {42, 32, 38, 27.5, 32, 29};
  double min_u_med_out[]   = {33, 26, 34, 22,   27, 25};
  double min_u_loose_out[] = {28, 22, 30, 18,   22, 22};

  // u shows a slight fall of the sampling fraction for high values
  double max_u_tight_out[] = {398, 398, 398, 398, 398, 398}; 
  double max_u_med_out[]   = {408, 408, 408, 408, 408, 408}; 
  double max_u_loose_out[] = {420, 420, 420, 420, 420, 420}; 

  // tight: only background outside 3 sigma, medium: 10 % outside 3 sigma, loose: 50% outside 3 sigma 
  double min_v_tight_out[] = {18.0, 12.0, 19.5,  15.5,  20.0, 13.0};
  double min_v_med_out[]   = {16.0, 10.5, 17.0,  14.25, 18.0, 11.0};
  double min_v_loose_out[] = {10.25, 8.0, 12.75, 12.5,  13.25, 9.0};

  // the maximum of v is never reached
  double max_v_tight_out[] = {400, 400, 400, 400, 400, 400};
  double max_v_med_out[]   = {400, 400, 400, 400, 400, 400};
  double max_v_loose_out[] = {400, 400, 400, 400, 400, 400};

  // tight: only background outside 3 sigma, medium: 10 % outside 3 sigma, loose: 50% outside 3 sigma 
  double min_w_tight_out[] = {14.0, 18.7, 18.7,  12.0, 16.0, 13.0};
  double min_w_med_out[]   = {11.0, 17.5, 16.25, 7.5,  14.5, 9.25};
  double min_w_loose_out[] = {7.25, 11.0, 13.0,  5.5,  10.0, 6.0};

  // the maximum of w is never reached
  double max_w_tight_out[] = {400, 400, 400, 400, 400, 400};
  double max_w_med_out[]   = {400, 400, 400, 400, 400, 400};
  double max_w_loose_out[] = {400, 400, 400, 400, 400, 400};

  //////////////////////////////////////////////////////////////

  double min_u = 0; double max_u = 0; double min_v = 0; double max_v = 0; double min_w = 0; double max_w = 0;  

  for(Int_t k = 0; k < 6; k++){  
    if(pcalSec-1 == k && inbending == true){
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
    if(pcalSec-1 == k && outbending == true){
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

  if(pcalL[v] > min_v && pcalL[v] < max_v && pcalL[w] > min_w && pcalL[w] < max_w) return true;
  else return false;
}



// DC fiducial cuts for the 3 regions

bool FiducialCuts::DC_hit_position_region1_fiducial_cut_triangle(int level){
  
  success = this->SetSwitches(level) && this->CheckDCid();
  if(!success) return false;


  double add = 0;  // value in cm added to the height and radius of the cut
  if(tight == true){  add = 1.0; }
  if(medium == true){ add = 0.0; }
  if(loose == true){  add = -1.0; }

  double angle = 60; 
  double height = 29;
  double radius = 29;

  double height_inb[]  = {27, 22, 22, 27, 22, 22};
  double height_outb[] = {15, 15, 15, 15, 15, 15};

  int sec = dcSec-1;   

  for(Int_t k = 0; k < 6; k++){  
    if(sec == k && inbending == true){
      height = height_inb[k] + add;
      radius = 29 + add;
    }
    if(sec == k && outbending == true){
      height = height_outb[k] + add;
      radius = 25 + add;
    }
  }

  double x1_rot = dcTraj[r1][y] * sin(sec*60.0*PI/180) + dcTraj[r1][x] * cos(sec*60.0*PI/180);
  double y1_rot = dcTraj[r1][y] * cos(sec*60.0*PI/180) - dcTraj[r1][x] * sin(sec*60.0*PI/180);

  double slope = 1/tan(0.5*angle*PI/180);
  double left  = (height - slope * y1_rot);
  double right = (height + slope * y1_rot);

  double radius2_DCr1 = pow(radius,2)-pow(y1_rot,2);    // cut out the inner circle

  if (x1_rot > left && x1_rot > right && pow(x1_rot,2) > radius2_DCr1) return true;
  else return false;

}


bool FiducialCuts::DC_hit_position_region2_fiducial_cut_triangle(int level){

  success = this->SetSwitches(level) && this->CheckDCid();
  if(!success) return false;


  double add = 0;  // value in cm added to the height and radius of the cut
  if(tight == true){  add = 2.0; }
  if(medium == true){ add = 0.0; }
  if(loose == true){  add = -2.0; }

  double angle = 60; 
  double height = 35;
  double radius = 40;

  double height_inb[]  = {40, 34, 34, 40, 34, 34};
  double height_outb[] = {25, 25, 25, 25, 25, 25};

  int sec = dcSec-1;

  for(Int_t k = 0; k < 6; k++){  
    if(sec == k && inbending == true){
      height = height_inb[k] + add;
      radius = 38 + add;
    }
    if(sec == k && outbending == true){
      height = height_outb[k] + add;
      radius = 39 + add;
    }
  }
    
  double x2_rot = dcTraj[r2][y] * sin(sec*60.0*PI/180) + dcTraj[r2][x] * cos(sec*60.0*PI/180);
  double y2_rot = dcTraj[r2][y] * cos(sec*60.0*PI/180) - dcTraj[r2][x] * sin(sec*60.0*PI/180);

  double slope = 1/tan(0.5*angle*PI/180);
  double left  = (height - slope * y2_rot);
  double right = (height + slope * y2_rot);

  double radius2_DCr2 = pow(radius,2)-pow(y2_rot,2);    // cut out the inner circle

  //return true;
  if (x2_rot > left && x2_rot > right && pow(x2_rot,2) > radius2_DCr2) return true;
  else return false;

}


bool FiducialCuts::DC_hit_position_region3_fiducial_cut_triangle(int level){

  success = this->SetSwitches(level) && this->CheckDCid();
  if(!success) return false;


  double add = 0;  // value in cm added to the height and radius of the cut
  if(tight == true){  add = 3.0; }
  if(medium == true){ add = 0.0; }
  if(loose == true){  add = -3.0; }

  double angle = 60; 
  double height = 48;
  double radius = 49;

  double height_inb[]  = {47, 39, 39, 47, 39, 39};
  double height_outb[] = {48, 48, 48, 48, 48, 48};

  int sec = dcSec-1;

  for(Int_t k = 0; k < 6; k++){  
    if(sec == k && inbending == true){
      height = height_inb[k] + add;
      radius = 50 + add;
    }
    if(sec == k && outbending == true){
      height = height_outb[k] + add;
      radius = 65 + add;
    }
  }

  double x3_rot = dcTraj[r3][y] * sin(sec*60.0*PI/180) + dcTraj[r3][x] * cos(sec*60.0*PI/180);
  double y3_rot = dcTraj[r3][y] * cos(sec*60.0*PI/180) - dcTraj[r3][x] * sin(sec*60.0*PI/180);

  double slope = 1/tan(0.5*angle*PI/180);
  double left  = (height - slope * y3_rot);
  double right = (height + slope * y3_rot);

  double radius2_DCr3 = pow(radius,2)-pow(y3_rot,2);    // cut out the inner circle

  if (x3_rot > left && x3_rot > right && pow(x3_rot,2) > radius2_DCr3) return true;
  else return false;

}



// DC fiducial cuts for the 3 regions based on 90% level of teh hit distribution

bool FiducialCuts::DC_hit_position_region1_fiducial_cut(int level){

  success = this->SetSwitches(level) && this->CheckDCid();
  if(!success) return false;


  double add = 0;  // value in cm added to the height and radius of the cut
  if(tight == true){  add = 0.5; }
  if(medium == true){ add = 0.0; }
  if(loose == true){  add = -0.5; }

  // calculate theta and phi local:

  double theta_DCr1 = 180/PI * acos(dcTraj[r1][z]/sqrt(pow(dcTraj[r1][x], 2) + pow(dcTraj[r1][y], 2) + pow(dcTraj[r1][z],2)));
  double phi_DCr1_raw = 180/PI * atan2(dcTraj[r1][y]/sqrt(pow(dcTraj[r1][x], 2) + pow(dcTraj[r1][y], 2) + pow(dcTraj[r1][z],2)), 
                                            dcTraj[r1][x]/sqrt(pow(dcTraj[r1][x], 2) + pow(dcTraj[r1][y], 2) + pow(dcTraj[r1][z],2)));
  double phi_DCr1 = 0;
  if(dcSec == 1) phi_DCr1 = phi_DCr1_raw;
  if(dcSec == 2) phi_DCr1 = phi_DCr1_raw - 60;
  if(dcSec == 3) phi_DCr1 = phi_DCr1_raw - 120;
  if(dcSec == 4 && phi_DCr1_raw > 0) phi_DCr1 = phi_DCr1_raw - 180;
  if(dcSec == 4 && phi_DCr1_raw < 0) phi_DCr1 = phi_DCr1_raw + 180;
  if(dcSec == 5) phi_DCr1 = phi_DCr1_raw + 120;
  if(dcSec == 6) phi_DCr1 = phi_DCr1_raw + 60;

  // calculate cut borders

  double p0_reg1_min_inb[]  = {100.317, -8.5579, 33.3805, -54.6661, 29.8076, -3.37618};
  double p1_reg1_min_inb[]  = {-54.3431, -10.2352, -25.312, 8.82734, -24.211, -11.9529};
  double p2_reg1_min_inb[]  = {-77.6878, 93.9246, 22.3518, 180.334, 32.7038, 87.8542};
  double p3_reg1_min_inb[]  = {6.2097800, 1.265860, 2.775140, -0.920097, 2.682680, 1.4199500};

  double p0_reg1_max_inb[]  = {36.3036, -25.4511, -14.4527, -111.204, -27.6039, 54.671};
  double p1_reg1_max_inb[]  = {-1.10543, 20.5772, 18.1882, 57.9931, 23.4017, -8.54834}; 
  double p2_reg1_max_inb[]  = {-149.594, -41.2337, -67.3151, 104.602, -39.4612, -178.519};
  double p3_reg1_max_inb[]  = {0.0424972, -2.05591, -2.01028, -6.455960, -2.58092, 0.8923610}; 

  double p0_reg1_min_out[]  = {100.317, -8.5579, 33.3805, -54.6661, 29.8076, -3.37618};
  double p1_reg1_min_out[]  = {-54.3431, -10.2352, -25.312, 8.82734, -24.211, -11.9529};
  double p2_reg1_min_out[]  = {-77.6878, 93.9246, 22.3518, 180.334, 32.7038, 87.8542};
  double p3_reg1_min_out[]  = {6.2097800, 1.265860, 2.775140, -0.920097, 2.682680, 1.4199500}; 

  double p0_reg1_max_out[]  = {36.3036, -25.4511, -14.4527, -111.204, -27.6039, 54.671};
  double p1_reg1_max_out[]  = {-1.10543, 20.5772, 18.1882, 57.9931, 23.4017, -8.54834}; 
  double p2_reg1_max_out[]  = {-149.594, -41.2337, -67.3151, 104.602, -39.4612, -178.519};
  double p3_reg1_max_out[]  = {0.0424972, -2.05591, -2.01028, -6.455960, -2.58092, 0.8923610}; 

  double p0_min = 0; double p1_min = 0; double p2_min = 0; double p3_min = 0;
  double p0_max = 0; double p1_max = 0; double p2_max = 0; double p3_max = 0;

  for(Int_t k = 0; k < 6; k++){  
    if(dcSec-1 == k && inbending == true){
      p0_min = p0_reg1_min_inb[k];
      p1_min = p1_reg1_min_inb[k];
      p2_min = p2_reg1_min_inb[k];
      p3_min = p3_reg1_min_inb[k];
      p0_max = p0_reg1_max_inb[k];
      p1_max = p1_reg1_max_inb[k];
      p2_max = p2_reg1_max_inb[k];
      p3_max = p3_reg1_max_inb[k];
    }
    if(dcSec-1 == k && outbending == true){
      p0_min = p0_reg1_min_out[k];
      p1_min = p1_reg1_min_out[k];
      p2_min = p2_reg1_min_out[k];
      p3_min = p3_reg1_min_out[k];
      p0_max = p0_reg1_max_out[k];
      p1_max = p1_reg1_max_out[k];
      p2_max = p2_reg1_max_out[k];
      p3_max = p3_reg1_max_out[k];
    }
  }

  double phi_DCr1_min = p0_min + p1_min * sqrt(theta_DCr1) + p2_min/theta_DCr1 + p3_min *theta_DCr1 + add;
  double phi_DCr1_max = p0_max + p1_max * sqrt(theta_DCr1) + p2_max/theta_DCr1 + p3_max *theta_DCr1 + add;


  if(theta_DCr1 > 17){ 
    phi_DCr1_min = p0_min + p1_min * sqrt(17) + p2_min/17 + p3_min * 17 + add;
  }
  if(theta_DCr1 > 22){ 
    phi_DCr1_max = p0_max + p1_max * sqrt(22) + p2_max/22 + p3_max * 22 + add;
  }


  if(phi_DCr1 > phi_DCr1_min && phi_DCr1 < phi_DCr1_max) return true;
  else return false;
}


bool FiducialCuts::DC_hit_position_region2_fiducial_cut(int level){

  success = this->SetSwitches(level) && this->CheckDCid();
  if(!success) return false;


  double add = 0;  // value in cm added to the height and radius of the cut
  if(tight == true){  add = 0.5; }
  if(medium == true){ add = 0.0; }
  if(loose == true){  add = -0.5; }

  // calculate theta and phi local:

  double theta_DCr2 = 180/PI * acos(dcTraj[r2][z]/sqrt(pow(dcTraj[r2][x], 2) + pow(dcTraj[r2][y], 2) + pow(dcTraj[r2][z],2)));
  double phi_DCr2_raw = 180/PI * atan2(dcTraj[r2][y]/sqrt(pow(dcTraj[r2][x], 2) + pow(dcTraj[r2][y], 2) + pow(dcTraj[r2][z],2)), 
                                            dcTraj[r2][x]/sqrt(pow(dcTraj[r2][x], 2) + pow(dcTraj[r2][y], 2) + pow(dcTraj[r2][z],2)));
  double phi_DCr2 = 0;
  if(dcSec == 1) phi_DCr2 = phi_DCr2_raw;
  if(dcSec == 2) phi_DCr2 = phi_DCr2_raw - 60;
  if(dcSec == 3) phi_DCr2 = phi_DCr2_raw - 120;
  if(dcSec == 4 && phi_DCr2_raw > 0) phi_DCr2 = phi_DCr2_raw - 180;
  if(dcSec == 4 && phi_DCr2_raw < 0) phi_DCr2 = phi_DCr2_raw + 180;
  if(dcSec == 5) phi_DCr2 = phi_DCr2_raw + 120;
  if(dcSec == 6) phi_DCr2 = phi_DCr2_raw + 60;

  // calculate cut borders

  double p0_reg2_min_inb[]  = {80.7923, 46.4053, -21.7329, -86.1723, 6.08924, 48.8282};
  double p1_reg2_min_inb[]  = {-47.5243, -34.2009, -3.35624, 21.8993, -14.3698, -36.1718};
  double p2_reg2_min_inb[]  = {-39.0975, 9.78629, 115.836, 227.581, 68.6816, 18.8391};
  double p3_reg2_min_inb[]  = {5.5683600, 4.141200, 0.368403, -2.401570, 1.565810, 4.4901200};

  double p0_reg2_max_inb[]  = {41.5402, -37.0049, 1.94269, -76.0226, -19.6333, 71.7106};
  double p1_reg2_max_inb[]  = {-2.98754, 25.2973, 12.1843, 44.4232, 19.8858, -14.2227};  
  double p2_reg2_max_inb[]  = {-157.258, -19.5048, -97.0197, 47.9323, -44.4464, -212.969};
  double p3_reg2_max_inb[]  = {0.2210050, -2.57855, -1.41432, -5.046280, -2.18194, 1.3971200}; 

  double p0_reg2_min_out[]  = {80.7923, 46.4053, -21.7329, -86.1723, 6.08924, 48.8282};
  double p1_reg2_min_out[]  = {-47.5243, -34.2009, -3.35624, 21.8993, -14.3698, -36.1718};
  double p2_reg2_min_out[]  = {-39.0975, 9.78629, 115.836, 227.581, 68.6816, 18.8391};
  double p3_reg2_min_out[]  = {5.5683600, 4.141200, 0.368403, -2.401570, 1.565810, 4.4901200};

  double p0_reg2_max_out[]  = {41.5402, -37.0049, 1.94269, -76.0226, -19.6333, 71.7106};
  double p1_reg2_max_out[]  = {-2.98754, 25.2973, 12.1843, 44.4232, 19.8858, -14.2227};  
  double p2_reg2_max_out[]  = {-157.258, -19.5048, -97.0197, 47.9323, -44.4464, -212.969};
  double p3_reg2_max_out[]  = {0.2210050, -2.57855, -1.41432, -5.046280, -2.18194, 1.3971200}; 

  double p0_min = 0; double p1_min = 0; double p2_min = 0; double p3_min = 0;
  double p0_max = 0; double p1_max = 0; double p2_max = 0; double p3_max = 0;

  for(Int_t k = 0; k < 6; k++){  
    if(dcSec-1 == k && inbending == true){
      p0_min = p0_reg2_min_inb[k];
      p1_min = p1_reg2_min_inb[k];
      p2_min = p2_reg2_min_inb[k];
      p3_min = p3_reg2_min_inb[k];
      p0_max = p0_reg2_max_inb[k];
      p1_max = p1_reg2_max_inb[k];
      p2_max = p2_reg2_max_inb[k];
      p3_max = p3_reg2_max_inb[k];
    }
    if(dcSec-1 == k && outbending == true){
      p0_min = p0_reg2_min_out[k];
      p1_min = p1_reg2_min_out[k];
      p2_min = p2_reg2_min_out[k];
      p3_min = p3_reg2_min_out[k];
      p0_max = p0_reg2_max_out[k];
      p1_max = p1_reg2_max_out[k];
      p2_max = p2_reg2_max_out[k];
      p3_max = p3_reg2_max_out[k];
    }
  }

  double phi_DCr2_min = p0_min + p1_min * sqrt(theta_DCr2) + p2_min/theta_DCr2 + p3_min *theta_DCr2 + add;
  double phi_DCr2_max = p0_max + p1_max * sqrt(theta_DCr2) + p2_max/theta_DCr2 + p3_max *theta_DCr2 + add;

  if(theta_DCr2 > 17){ 
    phi_DCr2_min = p0_min + p1_min * sqrt(17) + p2_min/17 + p3_min * 17 + add;
  }
  if(theta_DCr2 > 22){ 
    phi_DCr2_max = p0_max + p1_max * sqrt(22) + p2_max/22 + p3_max * 22 + add;
  }


  if(phi_DCr2 > phi_DCr2_min && phi_DCr2 < phi_DCr2_max) return true;
  else return false;
}


bool FiducialCuts::DC_hit_position_region3_fiducial_cut(int level){

  success = this->SetSwitches(level) && this->CheckDCid();
  if(!success) return false;


  double add = 0;  // value in cm added to the height and radius of the cut
  if(tight == true){  add = 0.5; }
  if(medium == true){ add = 0.0; }
  if(loose == true){  add = -0.5; }

  // calculate theta and phi local:

  double theta_DCr3 = 180/PI * acos(dcTraj[r3][z]/sqrt(pow(dcTraj[r3][x], 2) + pow(dcTraj[r3][y], 2) + pow(dcTraj[r3][z],2)));
  double phi_DCr3_raw = 180/PI * atan2(dcTraj[r3][y]/sqrt(pow(dcTraj[r3][x], 2) + pow(dcTraj[r3][y], 2) + pow(dcTraj[r3][z],2)), 
                                            dcTraj[r3][x]/sqrt(pow(dcTraj[r3][x], 2) + pow(dcTraj[r3][y], 2) + pow(dcTraj[r3][z],2)));
  double phi_DCr3 = 0;
  if(dcSec == 1) phi_DCr3 = phi_DCr3_raw;
  if(dcSec == 2) phi_DCr3 = phi_DCr3_raw - 60;
  if(dcSec == 3) phi_DCr3 = phi_DCr3_raw - 120;
  if(dcSec == 4 && phi_DCr3_raw > 0) phi_DCr3 = phi_DCr3_raw - 180;
  if(dcSec == 4 && phi_DCr3_raw < 0) phi_DCr3 = phi_DCr3_raw + 180;
  if(dcSec == 5) phi_DCr3 = phi_DCr3_raw + 120;
  if(dcSec == 6) phi_DCr3 = phi_DCr3_raw + 60;

  // calculate cut borders

  double p0_reg3_min_inb[]  = {112.851, 48.2343, 30.4458, -24.0206, 25.5385, 62.1018};
  double p1_reg3_min_inb[]  = {-61.5314, -34.6579, -24.9961, -3.67361, -23.2832, -42.4949}; 
  double p2_reg3_min_inb[]  = {-92.5007, -2.08871, 26.0078, 119.247, 40.2002, -5.77501};
  double p3_reg3_min_inb[]  = {7.3317800, 4.216710, 2.890850, 0.5438030, 2.714030, 5.4125100};

  double p0_reg3_max_inb[]  = {29.7655, -41.117, -44.3955, -71.2037, -26.2156, 42.0677};
  double p1_reg3_max_inb[]  = {2.04267, 27.0055, 30.5722, 42.2813, 23.2056, -2.40097};
  double p2_reg3_max_inb[]  = {-136.376, -9.54583, -15.5584, 42.8946, -37.348, -158.947};
  double p3_reg3_max_inb[]  = {-0.399112, -2.80870, -3.43194, -4.801160, -2.64121, 0.0779435};

  double p0_reg3_min_out[]  = {112.851, 48.2343, 30.4458, -24.0206, 25.5385, 62.1018};
  double p1_reg3_min_out[]  = {-61.5314, -34.6579, -24.9961, -3.67361, -23.2832, -42.4949}; 
  double p2_reg3_min_out[]  = {-92.5007, -2.08871, 26.0078, 119.247, 40.2002, -5.77501};
  double p3_reg3_min_out[]  = {7.3317800, 4.216710, 2.890850, 0.5438030, 2.714030, 5.4125100};

  double p0_reg3_max_out[]  = {29.7655, -41.117, -44.3955, -71.2037, -26.2156, 42.0677};
  double p1_reg3_max_out[]  = {2.04267, 27.0055, 30.5722, 42.2813, 23.2056, -2.40097};
  double p2_reg3_max_out[]  = {-136.376, -9.54583, -15.5584, 42.8946, -37.348, -158.947};
  double p3_reg3_max_out[]  = {-0.399112, -2.80870, -3.43194, -4.801160, -2.64121, 0.0779435};

  double p0_min = 0; double p1_min = 0; double p2_min = 0; double p3_min = 0;
  double p0_max = 0; double p1_max = 0; double p2_max = 0; double p3_max = 0;

  for(Int_t k = 0; k < 6; k++){  
    if(dcSec-1 == k && inbending == true){
      p0_min = p0_reg3_min_inb[k];
      p1_min = p1_reg3_min_inb[k];
      p2_min = p2_reg3_min_inb[k];
      p3_min = p3_reg3_min_inb[k];
      p0_max = p0_reg3_max_inb[k];
      p1_max = p1_reg3_max_inb[k];
      p2_max = p2_reg3_max_inb[k];
      p3_max = p3_reg3_max_inb[k];
    }
    if(dcSec-1 == k && outbending == true){
      p0_min = p0_reg3_min_out[k];
      p1_min = p1_reg3_min_out[k];
      p2_min = p2_reg3_min_out[k];
      p3_min = p3_reg3_min_out[k];
      p0_max = p0_reg3_max_out[k];
      p1_max = p1_reg3_max_out[k];
      p2_max = p2_reg3_max_out[k];
      p3_max = p3_reg3_max_out[k];
    }
  }

  double phi_DCr3_min = p0_min + p1_min * sqrt(theta_DCr3) + p2_min/theta_DCr3 + p3_min *theta_DCr3 + add;
  double phi_DCr3_max = p0_max + p1_max * sqrt(theta_DCr3) + p2_max/theta_DCr3 + p3_max *theta_DCr3 + add;

  if(theta_DCr3 > 17){ 
    phi_DCr3_min = p0_min + p1_min * sqrt(17) + p2_min/17 + p3_min * 17 + add;
  }
  if(theta_DCr3 > 22){ 
    phi_DCr3_max = p0_max + p1_max * sqrt(22) + p2_max/22 + p3_max * 22 + add;
  }

  if(phi_DCr3 > phi_DCr3_min && phi_DCr3 < phi_DCr3_max) return true;
  else return false;
}


void FiducialCuts::PrintFiducialCuts(int lev) {
  printf("----- FIDUCIAL CUTS -----\n");
  // PCAL
  printf("pcalSec=%d ",pcalSec);
  printf("pcalLayer=%d ",pcalLayer);
  printf("pcalL[u]=%.2f ",pcalL[u]);
  printf("pcalL[v]=%.2f ",pcalL[v]);
  printf("pcalL[w]=%.2f ",pcalL[w]);
  printf("\n");
  // DC tracks
  printf("dcSec=%d ",dcSec);
  printf("dcTrackDetector=%d ",dcTrackDetector);
  printf("\n");
  // DC trajectories
  for(int r=0; r<nReg; r++) {
    printf("dcTrajDetector[r%d]=%d ",r+1,dcTrajDetector[r]);
    printf("dcTrajLayer[r%d]=%d ",r+1,dcTrajLayer[r]);
    printf("dcTraj[r%d][x]=%.2f ",r+1,dcTraj[r][x]);
    printf("dcTraj[r%d][y]=%.2f ",r+1,dcTraj[r][y]);
    printf("dcTraj[r%d][z]=%.2f ",r+1,dcTraj[r][z]);
    printf("\n");
  };
  printf("---\n");
  // Stefan's cut booleans
  printf("EC_hit_position_fiducial_cut=%d\n",
          EC_hit_position_fiducial_cut(lev));
  printf("DC_hit_position_region1_fiducial_cut_triangle=%d\n",
          DC_hit_position_region1_fiducial_cut_triangle(lev));
  printf("DC_hit_position_region2_fiducial_cut_triangle=%d\n",
          DC_hit_position_region2_fiducial_cut_triangle(lev));
  printf("DC_hit_position_region3_fiducial_cut_triangle=%d\n",
          DC_hit_position_region3_fiducial_cut_triangle(lev));
  printf("DC_hit_position_region1_fiducial_cut=%d\n",
          DC_hit_position_region1_fiducial_cut(lev));
  printf("DC_hit_position_region2_fiducial_cut=%d\n",
          DC_hit_position_region2_fiducial_cut(lev));
  printf("DC_hit_position_region3_fiducial_cut=%d\n",
          DC_hit_position_region3_fiducial_cut(lev));
  printf("---\n");
  // full cut booleans
  printf("FidPCAL=%d\n",FidPCAL(lev));
  printf("FidDC=%d\n",FidDC(lev));
  printf("-------------------------\n");
};

