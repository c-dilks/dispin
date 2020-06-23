// counts number of pions in REC::Particle compared to number of corresponding
// trajectories in a DC layer
// - used for checking something with fiducial cuts

import org.jlab.io.hipo.HipoDataSource
import org.jlab.detector.base.DetectorType

def inHipo
if(args.length>=1) inHipo = args[0]
else { System.err << "ERROR: specify a hipo file\n"; return; }

def particleBank, trajBank

def evCount = 0
def piCount = 0
def trajCount = 0
def rowList
def trajList
reader = new HipoDataSource()
reader.open(inHipo)
while(reader.hasEvent()) {
  //if(evCount>10000) break // limiter
  if(evCount % 100000 == 0) println "read $evCount events"

  event = reader.getNextEvent()

  if(event.hasBank("REC::Particle")) {
    evCount++
    particleBank = event.getBank("REC::Particle")
    trajBank = event.getBank("REC::Traj")

    rowList = (0..<particleBank.rows()).findAll{
      particleBank.getInt('pid',it) == -211 &&
      ( particleBank.getShort('status',it) < 4000 ||
        particleBank.getShort('status',it) > 4999 )
    }
    piCount += rowList.size()

    /*
    // count if in layer 6 (region 1)
    trajList = (0..<trajBank.rows()).findAll{
      (int) (trajBank.getShort('pindex',it)) in rowList &&
      trajBank.getByte('detector',it) == DetectorType.DC.getDetectorId() &&
      trajBank.getByte('layer',it) == 6
    }
    trajCount += trajList.size()
    */
    // count if in any layer
    ///*
    trajList = rowList
    //println "----"
    (0..<trajBank.rows()).each{
      //println trajList
      def pindex = (int) trajBank.getShort('pindex',it)
      if( pindex in rowList &&
        trajBank.getByte('detector',it) == DetectorType.DC.getDetectorId()
      ) {
        //println "found $pindex"
        trajList = trajList.collect{
          if(it==pindex) return it+1000
          else return it
        }
      }

    }
    trajCount += trajList.findAll{it>1000}.size()
    //*/
  }
}
println "number of events with REC::Particle bank: $evCount"
println "number of pions in REC::Particle: $piCount"
println "number of matching pions in REC::Traj for DC: $trajCount"
