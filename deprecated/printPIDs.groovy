import org.jlab.io.hipo.HipoDataSource
import org.jlab.clas.physics.Particle
import clasqa.QADB

// ARGUMENTS
def inHipoFile = "../data/skim/skim4_005052.hipo"
if(args.length>=1) inHipoName = args[0]


QADB qa = new QADB()

reader = new HipoDataSource()
reader.open(inHipoFile)

// begin event loop
while(reader.hasEvent()) {
  event = reader.getNextEvent()
  if(event.hasBank("REC::Particle")) {
    particleBank = event.getBank("REC::Particle")
    (0..<particleBank.rows()).each { row ->
      println particleBank.getInt('pid',row)
    }
    println '-----'
  }
}


/*
0 undefined
11 positron
-11 electron
22 gamma
211 pi+
-211 pi-
321 K+
-321 K-
2212 proton
-2212 antiproton
2112 neutron
45 deuteron
*/
