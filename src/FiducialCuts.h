#ifndef FiducialCuts_
#define FiducialCuts_

#include <math.h>

#include "TObject.h"
#include "Constants.h"

// dihbsa implementation of Stefan Diehl's fiducial volume cut functions

/*
 * VARIABLES WHICH MUST BE DEFINED TO USE CUTS
 *
 * set enableFiducialCut to true
 *  
 * PCAL vars
 * ---------
 * - from REC::Calorimeter
 * pcalSec
 * pcalLayer
 * pcalL[u,v,w]
 *  
 * DC vars
 * --------
 * - from REC::Track
 * dcSec
 * dcTrackDetector
 * - from REC::Traj
 * dcTrajDetector[r1,r2,r3]
 * dcTrajLayer[r1,r2,r3]
 * dcTraj[r1,r2,r3][x,y,z]
 *  
 * other vars
 * ----------
 * torus
 */
    


class FiducialCuts : public TObject
{
  public:
    FiducialCuts();
    ~FiducialCuts();

    
    enum levelEnum { cutTight, cutMedium, cutLoose, nLevel }; // cut levels
    bool enableFiducialCut; // if false, cuts will return false
    bool debug;


    // PCAL cuts
    bool FidPCAL(int level) {
      if(!enableFiducialCut) return false;
      return EC_hit_position_fiducial_cut(level);
    };
    // PCAL variables
    int pcalSec;
    int pcalLayer;
    enum pcalLenum {u,v,w,nL};
    double pcalL[nL];


    // DC cuts
    bool FidDC(int level) {
      if(!enableFiducialCut) return false;
      ///*
      return DC_hit_position_region1_fiducial_cut(level) &&
             DC_hit_position_region2_fiducial_cut(level) &&
             DC_hit_position_region3_fiducial_cut(level);
             //*/
      /*
      // old cuts
      return DC_hit_position_region1_fiducial_cut_triangle(level) && 
             DC_hit_position_region2_fiducial_cut_triangle(level) &&
             DC_hit_position_region2_fiducial_cut_triangle(level);
             */
    };
    // DC variables
    // - tracks
    int dcSec;
    int dcTrackDetector;
    // - trajectories
    enum dcRegEnum {r1,r2,r3,nReg};
    int dcTrajDetector[nReg];
    int dcTrajLayer[nReg];
    enum dcDirEnum {x,y,z,nDir};
    double dcTraj[nReg][nDir];

    static int regLayer(int rr) {
      switch(rr) {
        case r1: return 6; break;
        case r2: return 18; break;
        case r3: return 30; break;
        default: 
          fprintf(stderr,"ERROR: unknown FiducialCuts::regLayer");
          return -1;
      };
    };

    
    // torus B-field (-1=inbending, +1=outbending)
    int torus;


    bool ErrPrint(const char * str) {
      fprintf(stderr,"ERROR: FiducialCuts::%s\n",str);
      return false;
    };
    void PrintFiducialCuts(int lev);


  protected:
    // Stefan's cuts
    bool EC_hit_position_fiducial_cut(int level);
    bool DC_hit_position_region1_fiducial_cut_triangle(int level);
    bool DC_hit_position_region2_fiducial_cut_triangle(int level);
    bool DC_hit_position_region3_fiducial_cut_triangle(int level);
    bool DC_hit_position_region1_fiducial_cut(int level);
    bool DC_hit_position_region2_fiducial_cut(int level);
    bool DC_hit_position_region3_fiducial_cut(int level);

    bool SetSwitches(int lev);
    bool CheckDCid();

    bool inbending,outbending;
    bool tight,medium,loose;
    bool success;

    char msg[256];

  private:
  ClassDef(FiducialCuts,1);
};

#endif
