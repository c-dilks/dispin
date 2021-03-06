/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Fiducial cuts (June 30 2020)
///
/// --> So far optmized for the inbending configuration
/// --> The cuts are not officially approved as the standard procedure by the collaboration
///

/// /////////////////////////////////////////////////////////
/// required vartiables
///

void assign_particles(void){

  Npart = vpart_px->size();

  for(Int_t i = 0; i < Npart; i++){
    if( i < BUFFER){
      part_pid[i] = vpart_pid->at(i);      // <--- required for hadron cuts
    }
  } 

  int Cal_Nentries = 0; int Traj_Nentries = 0; int TRK_Nentries = 0;

  Cal_Nentries  = vCal_pindex->size();
  TRK_Nentries  = vTRK_pindex->size();
  Traj_Nentries = vTraj_pindex->size();


  // Calorimeter bank (detector = 7  ---  layer:  PCAL = 1, ECin = 4, ECout = 7)

  if(Cal_Nentries > 0){
    for(int i = 0; i < Cal_Nentries; i++){
      if(vCal_pindex->at(i) >= 0 && vCal_pindex->at(i) < BUFFER && vCal_layer->at(i) == 1){
        part_Cal_PCAL_sector[vCal_pindex->at(i)] = vCal_sector->at(i); 
        part_Cal_PCAL_x[vCal_pindex->at(i)] = vCal_x->at(i);
        part_Cal_PCAL_y[vCal_pindex->at(i)] = vCal_y->at(i);
        part_Cal_PCAL_z[vCal_pindex->at(i)] = vCal_z->at(i);
        part_Cal_PCAL_lu[vCal_pindex->at(i)] = vCal_lu->at(i);
        part_Cal_PCAL_lv[vCal_pindex->at(i)] = vCal_lv->at(i);
        part_Cal_PCAL_lw[vCal_pindex->at(i)] = vCal_lw->at(i);
      }
    }
  }


  // tracking banks  (detectors: DC = 6, BST = 2,  BMT = 1, FMT = 8) 
  
  if(TRK_Nentries > 0){
    for(int i = 0; i < TRK_Nentries; i++){
      if(vTRK_pindex->at(i) < BUFFER && vTRK_detector->at(i) == 6){     // DC
        part_DC_Track_chi2[vTRK_pindex->at(i)] = vTRK_chi2->at(i);
        part_DC_Track_NDF[vTRK_pindex->at(i)] = vTRK_NDF->at(i);
      }
    }
  }
  

  // trajectory crosses  (layers: 6 = DC region 1 start,  18 = DC region 2 center,  36 = DC region 3 end ) 

  if(Traj_Nentries > 0){
    for(int i = 0; i < Traj_Nentries; i++){
      if(vTraj_pindex->at(i) >= 0 && vTraj_pindex->at(i) < BUFFER && vTraj_detID->at(i) == 6 && vTraj_layerID->at(i) == 6){    
          part_DC_c1x[vTraj_pindex->at(i)] = vTraj_x->at(i);
          part_DC_c1y[vTraj_pindex->at(i)] = vTraj_y->at(i);
          part_DC_c1z[vTraj_pindex->at(i)] = vTraj_z->at(i);
      }
      if(vTraj_pindex->at(i) >= 0 && vTraj_pindex->at(i) < BUFFER && vTraj_detID->at(i) == 6 && vTraj_layerID->at(i) == 18){  
          part_DC_c2x[vTraj_pindex->at(i)] = vTraj_x->at(i);
          part_DC_c2y[vTraj_pindex->at(i)] = vTraj_y->at(i);
          part_DC_c2z[vTraj_pindex->at(i)] = vTraj_z->at(i);
      }
      if(vTraj_pindex->at(i) >= 0 && vTraj_pindex->at(i) < BUFFER && vTraj_detID->at(i) == 6 && vTraj_layerID->at(i) == 36){  
          part_DC_c3x[vTraj_pindex->at(i)] = vTraj_x->at(i);
          part_DC_c3y[vTraj_pindex->at(i)] = vTraj_y->at(i);
          part_DC_c3z[vTraj_pindex->at(i)] = vTraj_z->at(i);
      }
    }
  }

  for(int i = 0; i < BUFFER; ++i)
  {
    part_DC_sector[i] = determineSector(i);
  }


  int determineSector(int i){

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




/////////////////////////////////////////////////////////////
/// EC hit position homogenous cut

/// This is the main cut for PCAL fiducial cut of electrons.
/// A cut is performed on v and w
/// Different versions are available: For SDIS I use the loose versions, For cross sectiosn I would recommend the medium or tigth version.


bool EC_hit_position_fiducial_cut_homogeneous(int j){

  ///////////////////////////
  bool tight = false;
  bool medium = false;
  bool loose = true;
  //////////////////////////

// Cut using the natural directions of the scintillator bars/ fibers:

  double u = part_Cal_PCAL_lu[j];
  double v = part_Cal_PCAL_lv[j];
  double w = part_Cal_PCAL_lw[j];
   
  /// v + w is going from the side to the back end of the PCAL, u is going from side to side
  /// 1 scintillator bar is 4.5 cm wide. In the outer regions (back) double bars are used.
  /// a cut is only applied on v and w

  ///////////////////////////////////////////////////////////////////
  /// inbending:
  // 
  double min_u_tight_inb[] = {19.0, 19.0, 19.0, 19.0, 19.0, 19.0};
  double min_u_med_inb[]   = {14.0, 14.0, 14.0, 14.0, 14.0, 14.0};
  double min_u_loose_inb[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_u_tight_inb[] = {398, 398, 398, 398, 398, 398}; 
  double max_u_med_inb[]   = {408, 408, 408, 408, 408, 408}; 
  double max_u_loose_inb[] = {420, 420, 420, 420, 420, 420}; 
  // 
  double min_v_tight_inb[] = {19.0, 19.0, 19.0, 19.0, 19.0, 19.0};
  double min_v_med_inb[]   = {14.0, 14.0, 14.0, 14.0, 14.0, 14.0};
  double min_v_loose_inb[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  //
  double max_v_tight_inb[] = {400, 400, 400, 400, 400, 400};
  double max_v_med_inb[]   = {400, 400, 400, 400, 400, 400};
  double max_v_loose_inb[] = {400, 400, 400, 400, 400, 400};
  //
  double min_w_tight_inb[] = {19.0, 19.0, 19.0, 19.0, 19.0, 19.0};
  double min_w_med_inb[]   = {14.0, 14.0, 14.0, 14.0, 14.0, 14.0};
  double min_w_loose_inb[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_w_tight_inb[] = {400, 400, 400, 400, 400, 400};
  double max_w_med_inb[]   = {400, 400, 400, 400, 400, 400};
  double max_w_loose_inb[] = {400, 400, 400, 400, 400, 400};


  ///////////////////////////////////////////////////////////////////////
  /// outbending (not adjusted up to now, same as inbending!):
  // 
  double min_u_tight_out[] = {19.0, 19.0, 19.0, 19.0, 19.0, 19.0};
  double min_u_med_out[]   = {14.0, 14.0, 14.0, 14.0, 14.0, 14.0};
  double min_u_loose_out[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_u_tight_out[] = {398, 398, 398, 398, 398, 398}; 
  double max_u_med_out[]   = {408, 408, 408, 408, 408, 408}; 
  double max_u_loose_out[] = {420, 420, 420, 420, 420, 420}; 
  // 
  double min_v_tight_out[] = {19.0, 19.0, 19.0, 19.0, 19.0, 19.0};
  double min_v_med_out[]   = {14.0, 14.0, 14.0, 14.0, 14.0, 14.0};
  double min_v_loose_out[] = {9.0,  9.0,  9.0,  9.0,  9.0,  9.0 };
  // 
  double max_v_tight_out[] = {400, 400, 400, 400, 400, 400};
  double max_v_med_out[]   = {400, 400, 400, 400, 400, 400};
  double max_v_loose_out[] = {400, 400, 400, 400, 400, 400};
  //
  double min_w_tight_out[] = {19.0, 19.0, 19.0, 19.0, 19.0, 19.0};
  double min_w_med_out[]   = {14.0, 14.0, 14.0, 14.0, 14.0, 14.0};
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



/////////////////////////////////////////////////////////////
/// EC hit position based on fitted sampling fraction

/// This cut can be used for photons
/// Alternatively a medium or tight homogenous cut can be used.

bool EC_hit_position_fiducial_cut(int j){

  //////////////////////////////////////////////
  bool tight   = false;   // MEAN - 1.0 RMS
  bool medium  = true;    // MEAN - 1.5 RMS
  bool loose   = false;   // MEAN - 2.0 RMS
  //////////////////////////////////////////////

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
          par0_min = par_0_min_1sigma_out[d]; par1_min = par_1_min_1sigma_out[d]; par2_min = par_2_min_1sigma_out[d]; 
          par3_min = par_3_min_1sigma_out[d]; par4_min = par_4_min_1sigma_out[d]; 
          par0_max = par_0_max_1sigma_out[d]; par1_max = par_1_max_1sigma_out[d]; par2_max = par_2_max_1sigma_out[d]; 
          par3_max = par_3_max_1sigma_out[d]; par4_max = par_4_max_1sigma_out[d]; 
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
          par0_min = par_0_min_15sigma_out[d]; par1_min = par_1_min_15sigma_out[d]; par2_min = par_2_min_15sigma_out[d]; par3_min = par_3_min_15sigma_out[d]; 
          par4_min = par_4_min_15sigma_out[d]; 
          par0_max = par_0_max_15sigma_out[d]; par1_max = par_1_max_15sigma_out[d]; par2_max = par_2_max_15sigma_out[d]; par3_max = par_3_max_15sigma_out[d]; 
          par4_max = par_4_max_15sigma_out[d]; 
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
          par0_min = par_0_min_2sigma_out[d]; par1_min = par_1_min_2sigma_out[d]; par2_min = par_2_min_2sigma_out[d]; par3_min = par_3_min_2sigma_out[d]; 
          par4_min = par_4_min_2sigma_out[d]; 
          par0_max = par_0_max_2sigma_out[d]; par1_max = par_1_max_2sigma_out[d]; par2_max = par_2_max_2sigma_out[d]; par3_max = par_3_max_2sigma_out[d]; 
          par4_max = par_4_max_2sigma_out[d]; 
        }
      }
    }
  }

  double min_phi = par0_min + par1_min * log(theta_PCAL) + par2_min * theta_PCAL + par3_min *theta_PCAL*theta_PCAL + par4_min *theta_PCAL*theta_PCAL*theta_PCAL;
  double max_phi = par0_max + par1_max * log(theta_PCAL) + par2_max * theta_PCAL + par3_max *theta_PCAL*theta_PCAL + par4_max *theta_PCAL*theta_PCAL*theta_PCAL;

  if(phi_PCAL >= min_phi && phi_PCAL <= max_phi) return true; 
  else return false;
}



/// //////////////////////////////////////////////////////////////////
/// finalized DC fiducial cuts:

/// Cut based on chi2/NDF, straight lines are used in the x-y hit plane
/// THis is the recommened cut for electrons
/// only for electrons (uses straigt line in x,y fitted to chi2/NDF)


bool DC_fiducial_cut_XY(int j, int region){     

//fitted values

const double maxparams[6][3][2] =
{{{-7.49907,0.583375},{-18.8174,0.599219},{-23.9353,0.574699}},
{{-14.0547,0.631533},{-14.4223,0.597079},{-14.838,0.547436}},
{{-7.72508,0.578501},{-18.7928,0.56725},{-29.9003,0.612354}},
{{-6.12844,0.566777},{-13.6772,0.573262},{-26.1895,0.591816}},
{{-20.0718,0.670941},{-9.4775,0.511748},{-28.0869,0.590488}},
{{-9.52924,0.591687},{-17.8564,0.596417},{-23.5661,0.576317}}};

const double minparams[6][3][2] =
{{{7.62814, -0.56319},{18.2833, -0.587275},{20.2027, -0.54605}},
{{9.20907, -0.586977},{10.493, -0.544243},{23.0759, -0.581959}},
{{12.5459, -0.631322},{20.5635, -0.618555},{26.3621, -0.576806}},
{{8.36343, -0.552394},{14.7596, -0.554798},{29.5554, -0.60545}},
{{16.3732, -0.663303},{10.0255, -0.533019},{31.6086, -0.617053}},
{{8.20222, -0.567211},{20.0181, -0.605458},{22.2098, -0.567599}}};

double X;
double Y;

switch (region){
case 1:
X = part_DC_c1x[j];
Y = part_DC_c1y[j];
break;

case 2:
X = part_DC_c2x[j];
Y = part_DC_c2y[j];
break;

case 3:
X = part_DC_c3x[j];
Y = part_DC_c3y[j];
break;

default:
X = 0;
Y = 0;
break;
}


if (part_DC_sector[j] == 2){
const double X_new = X * std::cos(-60 * Pival / 180) - Y * std::sin(-60 * Pival / 180);
Y = X * std::sin(-60 * Pival / 180) + Y * std::cos(-60 * Pival / 180);
X = X_new;
}

if (part_DC_sector[j] == 3){
const double X_new = X * std::cos(-120 * Pival / 180) - Y * std::sin(-120 * Pival / 180);
Y = X * std::sin(-120 * Pival / 180) + Y * std::cos(-120 * Pival / 180);
X = X_new;
}

if (part_DC_sector[j] == 4){
const double X_new = X * std::cos(-180 * Pival / 180) - Y * std::sin(-180 * Pival / 180);
Y = X * std::sin(-180 * Pival / 180) + Y * std::cos(-180 * Pival / 180);
X = X_new;
}

if (part_DC_sector[j] == 5){
const double X_new = X * std::cos(120 * Pival / 180) - Y * std::sin(120 * Pival / 180);
Y = X * std::sin(120 * Pival / 180) + Y * std::cos(120 * Pival / 180);
X = X_new;
}

if (part_DC_sector[j] == 6){
const double X_new = X * std::cos(60 * Pival / 180) - Y * std::sin(60 * Pival / 180);
Y = X * std::sin(60 * Pival / 180) + Y * std::cos(60 * Pival / 180);
X = X_new;
}

--region;

double calc_min = minparams[part_DC_sector[j] - 1][region][0] + minparams[part_DC_sector[j] - 1][region][1] * X;
double calc_max = maxparams[part_DC_sector[j] - 1][region][0] + maxparams[part_DC_sector[j] - 1][region][1] * X;

return ((Y > calc_min) && (Y < calc_max));

}


/// Cut based on chi2/NDF, a polynomial border in the theta-phi plane is used
/// Can be used for electrons and hadrons.
/// The correct hadron is determined automatically (The variable part_pid[j] has to contain the PID of particle j in the event)
/// This cut is recommended for all charged hadrons

bool DC_fiducial_cut_theta_phi(int j, int  region){   // for electrons and hadrons (uses polynomial in detector coordinates)

//fitted values

const double maxparams[6][6][3][4] = {
{{{-35.1716, 25.102, -0.750281, 5.34679e-05},{-39.1633, 28.5551, -1.13429, 0.00419047},{-33.7705, 24.8068, -0.811239, 0.00138345}},
{{-36.2389, 26.7979, -1.08147, 0.0050898},{-43.643, 31.6783, -1.49203, 0.00872922},{-54.4042, 40.6516, -2.52393, 0.0205649}},
{{-38.3238, 26.1667, -0.777077, 0.000264835},{-34.2011, 24.2843, -0.696392, 3.75866e-12},{-36.4636, 25.8712, -0.786592, 2.24421e-10}},
{{-31.8019, 23.154, -0.653992, 2.69968e-05},{-34.6637, 24.6043, -0.714901, 2.02675e-10},{-36.7209, 26.2469, -0.828638, 0.000340435}},
{{-33.4016, 24.6901, -0.779889, 0.000430557},{-35.4583, 24.7491, -0.707953, 2.18559e-10},{-37.7335, 28.1547, -1.1986, 0.00582395}},
{{-34.7808, 24.6988, -0.719936, 5.73299e-10},{-54.5797, 40.9138, -2.57493, 0.0213354},{-38.4972, 28.3142, -1.21741, 0.00640373}}},
{{{-2.25358e-08, 12.631, -0.767619, 0.00739811},{-8.09501, 15.9098, -0.844083, 0.00667995},{-1.48113e-06, 12.2061, -0.73167, 0.0074309}},
{{-2.10872e-07, 12.6689, -0.765156, 0.00720044},{-4.88862, 14.0376, -0.687202, 0.00506307},{-4.59793e-06, 11.5553, -0.591469, 0.00536957}},
{{-1.13504e-08, 12.6011, -0.746025, 0.00687498},{-6.97884, 15.1788, -0.765889, 0.00570532},{-1.29468, 12.3844, -0.667561, 0.00619226}},
{{-2.91953e-09, 13.883, -0.999624, 0.0104257},{-4.9855, 13.8864, -0.661348, 0.0048371},{-2.29438e-08, 11.8341, -0.668486, 0.00669247}},
{{-2.02824e-08, 13.3855, -0.91158, 0.00926769},{-3.29092e-08, 10.8294, -0.382323, 0.00178367},{-4.59027e-06, 11.9414, -0.663872, 0.00625769}},
{{-3.73322e-09, 12.6126, -0.723548, 0.0062217},{-4.56248, 14.1574, -0.727805, 0.00560108},{-2.39381e-08, 12.0663, -0.6651, 0.00602544}}},
{{{-1.45923e-08, 13.0297, -0.828302, 0.00795271},{-5.41905, 13.2753, -0.503236, 0.00255607},{-3.67719, 12.1358, -0.462905, 0.00308219}},
{{-9.953e-10, 11.549, -0.52816, 0.00378771},{-8.47154, 15.9863, -0.826166, 0.0062936},{-6.43715, 13.9081, -0.618535, 0.0046102}},
{{-4.68458e-08, 12.9481, -0.781613, 0.00689754},{-3.46617, 12.2786, -0.440121, 0.00205448},{-4.43519, 10.9372, -0.210059, 3.69283e-10}},
{{-4.18414e-07, 13.1542, -0.811251, 0.00714402},{-4.63166, 13.7769, -0.657207, 0.0047586},{-1.99278e-05, 11.3993, -0.575232, 0.00532141}},
{{-7.07189e-10, 13.2814, -0.88476, 0.00874389},{-5.08373, 14.4384, -0.750795, 0.00586116},{-6.9642e-05, 9.50651, -0.189316, 3.07274e-06}},
{{-5.85515e-08, 12.5116, -0.688741, 0.00557297},{-1.86306, 11.985, -0.482567, 0.00279836},{-4.94295e-07, 10.1342, -0.316715, 0.00176254}}},
{{{-0.0157256, 11.1508, -0.415185, 0.00186904},{-13.6561, 19.4418, -1.15773, 0.00989432},{-6.24969e-07, 10.5776, -0.329325, 0.00103488}},
{{-2.5686e-08, 11.4797, -0.476772, 0.00264288},{-0.0475099, 10.1207, -0.244786, 3.13032e-06},{-4.6875e-07, 12.019, -0.63598, 0.00543214}},
{{-0.00702545, 11.1294, -0.407207, 0.00171263},{-7.27687, 15.5, -0.807858, 0.0062086},{-5.15078, 12.6368, -0.348584, 9.2687e-12}},
{{-8.14106e-08, 13.28, -0.818164, 0.00703758},{-7.60722, 14.4871, -0.588662, 0.00326244},{-1.70764e-06, 12.0413, -0.63961, 0.00541784}},
{{-1.09281, 11.5573, -0.41311, 0.00155228},{-3.71599, 12.8335, -0.521472, 0.00296792},{-0.000410815, 12.4833, -0.72999, 0.0066601}},
{{-0.652641, 12.2766, -0.554202, 0.00314615},{-8.42824, 15.5087, -0.710609, 0.00447051},{-14.9692, 21.5885, -1.47528, 0.0136615}}},
{{{-5.58945, 17.4004, -1.34516, 0.0142099},{-14.9585, 20.4538, -1.25118, 0.0106617},{-12.0069, 16.4545, -0.727162, 0.00495418}},
{{-7.03048, 17.3519, -1.1831, 0.0111308},{-7.30641, 15.8503, -0.850952, 0.00648446},{-10.2549, 15.6139, -0.648352, 0.00380506}},
{{-9.73111e-09, 13.498, -0.932479, 0.00939708},{-8.38053, 15.5588, -0.711323, 0.00433827},{-12.3097, 16.6403, -0.741362, 0.0050708}},
{{-7.38905, 17.2652, -1.15517, 0.0109165},{-1.11835e-07, 10.4637, -0.301972, 0.000612754},{-12.2182, 17.4958, -0.919555, 0.00747512}},
{{-0.492676, 14.4148, -1.0959, 0.0116708},{-5.34309, 14.3258, -0.691954, 0.00480109},{-12.5443, 16.1047, -0.59594, 0.00280171}},
{{-4.08375e-07, 12.2846, -0.655278, 0.00525956},{-8.93101, 16.4266, -0.861853, 0.00644623},{-11.8406, 17.0417, -0.826301, 0.00596028}}},
{{{-9.29415, 16.5566, -0.831923, 0.00562504},{-0.954483, 10.5813, -0.265766, 3.24615e-05},{-6.87423, 14.892, -0.76495, 0.00639603}},
{{-18.8913, 19.3123, -0.711917, 0.00227889},{-13.9788, 18.5678, -0.940183, 0.00664397},{-11.7696, 18.3415, -1.04368, 0.0083506}},
{{-3.82873, 12.7727, -0.425968, 0.000789835},{-9.81221, 14.6531, -0.471092, 0.00131406},{-14.2392, 15.9895, -0.430525, 2.20712e-12}},
{{-1.76975e-07, 11.4006, -0.420134, 0.00141302},{-3.11764, 10.9707, -0.245823, 2.23044e-12},{-17.6005, 22.2881, -1.39992, 0.0117791}},
{{-0.767518, 11.6824, -0.456275, 0.00214005},{-5.28047, 12.65, -0.350658, 9.80081e-05},{-0.0888832, 11.508, -0.49197, 0.00301269}},
{{-4.72388, 15.8507, -1.00574, 0.00876768},{-2.80649, 11.4056, -0.301812, 0.000190262},{-13.0484, 18.665, -1.08614, 0.00960977}}}};

const double minparams[6][6][3][4] = {
{{{37.289, -27.5201,1.12866, -0.00526111},{45.3103, -33.5226,1.72923, -0.0114495},{61.5709, -47.6158,3.4295, -0.0316429}},
{{36.6259, -27.4064,1.16617, -0.00604629},{50.3751, -37.5848,2.19621, -0.0169241},{35.1563, -26.514,1.09795, -0.00545864}},
{{27.2367, -20.3068,0.517752, -0.000335432},{39.0489, -28.6903,1.24306, -0.0065226},{41.0208, -30.0339,1.30776, -0.00626721}},
{{29.261, -21.7041,0.613556, -0.000774652},{39.5304, -29.1388,1.34116, -0.00823818},{44.5313, -33.4056,1.77581, -0.0123965}},
{{36.5659, -25.119,0.714074, -2.65397e-11},{31.6524, -22.6934,0.613977, -5.46634e-10},{34.7312, -24.9901,0.749061, -1.22922e-09}},
{{33.154, -23.8803,0.685794, -1.13236e-10},{42.6731, -31.0799,1.40425, -0.00730816},{46.4732, -35.6988,2.10144, -0.0164771}}},
{{{2.40451, -15.0848,1.05504, -0.0103356},{8.93825, -16.5995,0.925874, -0.00767902},{7.23439e-08, -12.5963,0.814574, -0.00864749}},
{{6.2953e-07, -12.6365,0.732206, -0.00639165},{12.6866, -18.7831,1.0952, -0.00923029},{3.12805e-07, -12.5395,0.795535, -0.00828991}},
{{2.69495, -14.8778,1.00751, -0.00975373},{6.05446, -14.6778,0.767457, -0.00636729},{3.94741e-07, -11.1038,0.524109, -0.00471514}},
{{2.31558e-07, -11.5073,0.494316, -0.00303611},{5.66995, -14.5948,0.740956, -0.00561851},{4.40475e-06, -9.57062,0.20354, -0.000213213}},
{{2.74277e-08, -13.3573,0.886651, -0.00857992},{9.98849e-05, -11.524,0.531486, -0.00391441},{8.50811e-07, -9.72224,0.240264, -0.000781498}},
{{6.9021e-08, -11.8859,0.53864, -0.00325092},{10.0169, -16.9153,0.921593, -0.00752414},{9.90518e-07, -11.9578,0.697029, -0.00717645}}},
{{{6.87966e-10, -12.8497,0.757379, -0.00651612},{16.2087, -19.3776,0.951508, -0.00645029},{14.513, -18.8625,1.05542, -0.00918985}},
{{1.07197e-07, -12.5469,0.703086, -0.00585238},{0.0871522, -9.22628,0.159628, -0.000343326},{12.1181, -17.5575,0.940249, -0.00788125}},
{{2.10191e-09, -12.2782,0.661926, -0.00555279},{12.5105, -17.9998,0.951807, -0.00732845},{12.8043, -17.8322,0.972401, -0.00841528}},
{{8.11926e-10, -12.7225,0.737941, -0.00647355},{7.50649, -15.987,0.889398, -0.00729282},{0.174541, -10.0266,0.306882, -0.00186093}},
{{3.81202e-09, -12.0926,0.598943, -0.00430458},{8.72368, -17.2511,1.06348, -0.00953327},{1.5205, -9.86713,0.183806, -6.40377e-12}},
{{1.37378e-07, -12.9247,0.769722, -0.00664936},{8.53877, -16.6167,0.946138, -0.00788745},{8.47417, -14.3897,0.581492, -0.00387111}}},
{{{2.50079e-07, -12.5209,0.678491, -0.00528954},{12.6171, -18.4205,1.01802, -0.00807702},{10.4903, -18.0981,1.10546, -0.00971519}},
{{5.87069e-07, -12.0075,0.585538, -0.00416654},{11.1348, -17.5468,0.943652, -0.00729083},{0.949201, -10.5869,0.267536, -6.04802e-05}},
{{1.14857, -11.1478,0.345528, -0.000841836},{10.9482, -17.1647,0.909605, -0.00722404},{8.7569e-08, -10.4446,0.316302, -0.00101964}},
{{1.09759e-06, -11.5019,0.48435, -0.00277852},{0.637937, -10.7065,0.316211, -0.000801127},{5.67144e-07, -12.88,0.831252, -0.00835441}},
{{1.68853, -11.2582,0.308152, -7.81686e-12},{9.44238, -17.1892,1.00561, -0.00864837},{1.20713e-07, -12.2246,0.669321, -0.0057622}},
{{0.00217558, -10.8858,0.347928, -0.000790679},{11.8583, -17.6423,0.923581, -0.00703041},{3.24078, -13.4024,0.668777, -0.00504175}}},
{{{6.04158, -16.8155,1.13335, -0.0105359},{8.24786, -17.0204,1.05097, -0.00941875},{11.7617, -17.202,0.864472, -0.00649032}},
{{3.70947, -13.0663,0.513818, -0.00222627},{16.7022, -21.9618,1.42869, -0.012705},{6.8993, -14.8192,0.740813, -0.00585407}},
{{2.18472e-06, -11.9461,0.583354, -0.00423414},{6.51489e-07, -10.5669,0.353028, -0.00166977},{12.5113, -16.5038,0.709888, -0.00471964}},
{{0.812719, -11.3245,0.390183, -0.00134086},{2.97251, -11.9374,0.338592, -4.36096e-13},{13.8844, -17.5707,0.818446, -0.00581811}},
{{1.55496, -14.4569,0.949497, -0.00857237},{0.34359, -10.5041,0.286497, -0.000346977},{14.4141, -18.7457,1.01652, -0.00845189}},
{{1.26317e-08, -11.1424,0.434251, -0.00236267},{6.58119, -15.8546,0.930324, -0.00801288},{4.41865, -11.1991,0.234652, -7.43723e-10}}},
{{{6.87926, -12.8949,0.334733, -6.38494e-06},{35.2336, -32.2007,2.21489, -0.020555},{6.80949, -16.8945,1.19056, -0.0127558}},
{{0.95782, -12.4625,0.599979, -0.00405342},{20.4051, -23.1936,1.42408, -0.0120792},{10.277, -16.1457,0.785186, -0.00612069}},
{{0.236196, -11.6165,0.458613, -0.002018},{12.8771, -19.6785,1.26163, -0.0115917},{5.21194e-08, -12.551,0.78718, -0.00794713}},
{{8.40778, -14.9001,0.534967, -0.00147246},{15.9376, -20.9945,1.2908, -0.0110556},{10.4773, -16.2238,0.783386, -0.00593478}},
{{3.21187, -12.1221,0.348938, -8.70415e-14},{13.8983, -19.1128,1.04727, -0.00797426},{11.6342, -18.8428,1.18853, -0.0107619}},
{{3.7311, -12.4292,0.419345, -0.00134704},{6.92884, -13.2494,0.391862, -0.000767396},{5.5939, -14.4175,0.729195, -0.00568477}}}};



double theta_DCr = 5000;
double phi_DCr_raw = 5000;

switch (region){

case 1:
theta_DCr = 180 / Pival * acos(part_DC_c1z[j] / sqrt(pow(part_DC_c1x[j],2) + pow(part_DC_c1y[j],2) + pow(part_DC_c1z[j],2)));
phi_DCr_raw = 180 / Pival * atan2(part_DC_c1y[j] / sqrt(pow(part_DC_c1x[j],2) + pow(part_DC_c1y[j],2) + pow(part_DC_c1z[j],2)), part_DC_c1x[j] /sqrt(pow(part_DC_c1x[j],2) 
               + pow(part_DC_c1y[j],2) + pow(part_DC_c1z[j],2)));
break;

case 2:
theta_DCr = 180 / Pival * acos(part_DC_c2z[j] / sqrt(pow(part_DC_c2x[j],2) + pow(part_DC_c2y[j],2) + pow(part_DC_c2z[j],2)));
phi_DCr_raw = 180 / Pival * atan2(part_DC_c2y[j] / sqrt(pow(part_DC_c2x[j],2) + pow(part_DC_c2y[j],2) + pow(part_DC_c2z[j],2)), part_DC_c2x[j] /sqrt(pow(part_DC_c2x[j],2) 
              + pow(part_DC_c2y[j],2) + pow(part_DC_c2z[j],2)));
break;

case 3:
theta_DCr = 180 / Pival * acos(part_DC_c3z[j] / sqrt(pow(part_DC_c3x[j],2) + pow(part_DC_c3y[j],2) + pow(part_DC_c3z[j],2)));
phi_DCr_raw = 180 / Pival * atan2(part_DC_c3y[j] / sqrt(pow(part_DC_c3x[j],2) + pow(part_DC_c3y[j],2) + pow(part_DC_c3z[j],2)), part_DC_c3x[j] /sqrt(pow(part_DC_c3x[j],2) 
              + pow(part_DC_c3y[j],2) + pow(part_DC_c3z[j],2)));
break;

default: return false;
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

switch (part_pid[j]){

case 11: pid = 0;
break;
case 2212: pid = 1;
break;
case 211: pid = 2;
break;
case -211:
pid = 3;
break;
case 321: pid = 4;
break;
case -321: pid = 5;
break;
default: return false;
break;

}

--region;

double calc_phi_min = minparams[pid][part_DC_sector[j] - 1][region][0] + minparams[pid][part_DC_sector[j] - 1][region][1] * std::log(theta_DCr) 
                      + minparams[pid][part_DC_sector[j] - 1][region][2] * theta_DCr + minparams[pid][part_DC_sector[j] - 1][region][3] * theta_DCr * theta_DCr;

double calc_phi_max = maxparams[pid][part_DC_sector[j] - 1][region][0] + maxparams[pid][part_DC_sector[j] - 1][region][1] * std::log(theta_DCr) 
                      + maxparams[pid][part_DC_sector[j] - 1][region][2] * theta_DCr + maxparams[pid][part_DC_sector[j] - 1][region][3] * theta_DCr * theta_DCr;


return ((phi_DCr > calc_phi_min) && (phi_DCr < calc_phi_max));

}
