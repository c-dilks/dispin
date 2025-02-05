// get beam energy from RCDB
import org.rcdb.*

// arguments
if(args.length<1) {
  System.err.println "USAGE: run-groovy ${this.class.getSimpleName()}.groovy [LIST_OF_RUNS]"
  System.out.println ''
  System.exit(101)
}
runlist_file_name = args[0]

// open run list
def runlist_file = new File(runlist_file_name)
if(!runlist_file.exists()) {
  throw new Exception("$runlist_file_name not found")
}
runlist = []
runlist_file.eachLine{ runlist << it.toInteger() }

// connect to RCDB
def rcdbURL = 'mysql://rcdb@clasdb-farm.jlab.org/rcdb'
def rcdbProvider = RCDB.createProvider(rcdbURL)
try {
  rcdbProvider.connect()
}
catch(Exception e) {
  System.err.println "ERROR: unable to connect to RCDB"
  System.out.println ''
  System.exit(100)
}

runlist.each{ runnum ->
  result = rcdbProvider.getCondition(runnum, 'beam_energy')
  if(result==null) {
    System.err.println "ERROR: cannot find run $runnum in RCDB, thus cannot get beam energy"
    System.out.println ''
    System.exit(100)
  }
  beamEn = result.toDouble() / 1e3 // [MeV] -> [GeV]
  System.out.println "$runnum $beamEn"
}
