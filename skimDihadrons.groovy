// a more general monitor, for things like <sinPhiH> or helicity
// - this reads DST files or skim files
// - can be run on slurm
// - note: search for 'CUT' to find which cuts are applied

import org.jlab.io.hipo.HipoDataSource
import org.jlab.clas.physics.Particle
import org.jlab.detector.base.DetectorType
import org.jlab.jroot.ROOTFile
import org.jlab.jroot.TNtuple
import groovy.json.JsonOutput
import java.lang.Math.*


////////////////////////
// ARGUMENTS
def inHipo = "../data/skim/skim4_5052.hipo" // skim file
if(args.length>=1) inHipo = args[0]
////////////////////////
// OPTIONS
def verbose = true
////////////////////////


// list of hipo files (one for now, but allows for future
// capability to read DST files in a list)
def inHipoList = []
inHipoList << inHipo


// set up list of hadrons to pair into dihadrons
hadPIDs = [ 211, -211 ]


// get runnum
def runnum
if(inHipo.contains('postprocess'))
  runnum = inHipo.tokenize('.')[-2].tokenize('/')[-1].toInteger()
else
  runnum = inHipo.tokenize('.')[-2].tokenize('_')[-1].toInteger()
println "runnum=$runnum"



def pPrint = { str -> JsonOutput.prettyPrint(JsonOutput.toJson(str)) }


// define variables
def event
def particleBank, configBank, eventBank, calBank
def eleMap
def hadMapList
def eleDIS
def evnum
def helicity
def reader
def evCount
def detIdEC = DetectorType.ECAL.getDetectorId()


// subroutine which, for a specified PID, returns a map 
// REC::Particle row -> coatjava Particle object
def pidList = []
def findParticles = { pid ->

  // get list of bank rows and Particle objects corresponding to this PID
  def rowList = pidList.findIndexValues{ it == pid }.collect{it as Integer}
  def particleMap = rowList.collectEntries { row ->
    [
      row,
      new Particle(
        pid,
        *['px','py','pz'].collect{particleBank.getFloat(it,row)}
      )
    ]
  }

  // verbose printing
  if(verbose) {
    println "- pid=$pid  found in rows $rowList"
    particleMap.each{ row, par ->
      def status = particleBank.getShort('status',row)
      def chi2pid = particleBank.getFloat('chi2pid',row)
      println " row=$row  status=$status  chi2pid=$chi2pid"
      println par
    }
  }

  return particleMap
}




// setup diskim file
"mkdir -p diskim".execute()
def diskimFile = new ROOTFile('diskim/test.root')


// setup ntuples
def buildParticleNt = { name ->
  def vars = [
    'Px','Py','Pz',
    'E'
  ].join(":${name}")
  vars = "${name}vars"
  return diskimFile.makeNtuple("${name}Nt","${name}Nt",vars)
}
def eleNt = buildParticleNt('ele')
def hadANt = buildParticleNt('hadA')
def hadBNt = buildParticleNt('hadB')
def evNt = diskimFile.makeNtuple("evNt","evNt",
  [ 'evnum',
    'helicity',
  ].join(':')
)


// subroutine to fill ntuple with particle data
def fillParticleNt = { nt, par ->
  nt.fill(
    par.px(), par.py(), par.pz(),
    par.e()
  )
}





//----------------------
// event loop
//----------------------
evCount = 0
inHipoList.each { inHipoFile ->

  // open skim/DST file
  reader = new HipoDataSource()
  reader.open(inHipoFile)

  // EVENT LOOP
  while(reader.hasEvent()) {
    if(evCount>10000) break // limiter
    evCount++
    if(evCount % 100000 == 0) println "read $evCount events"
    if(verbose) { 30.times{print '='}; println " begin event" }
    event = reader.getNextEvent()

    if(event.hasBank("REC::Particle") &&
       event.hasBank("REC::Event") &&
       event.hasBank("RUN::config") ) {

      // get banks
      particleBank = event.getBank("REC::Particle")
      eventBank = event.getBank("REC::Event")
      configBank = event.getBank("RUN::config")
      calBank = event.getBank("REC::Calorimeter")


      // get event-level information
      helicity = eventBank.getByte('helicity',0)
      evnum = BigInteger.valueOf(configBank.getInt('event',0))


      // get list of PIDs, with list index corresponding to bank row
      pidList = (0..<particleBank.rows()).collect{ 
        particleBank.getInt('pid',it)
      }
      if(verbose) println "pidList = $pidList"


      
      // find the scattered electron
      // - first get the electrons which satisfy cuts
      // - then choose from this list the electron with the highest E
      eleMap = findParticles(11).findAll { row, ele ->
        def status = particleBank.getShort('status',row)
        def chi2pid = particleBank.getFloat('chi2pid',row)
        status<0 &&
          ( Math.abs(status/1000).toInteger() & 0x2 || 
            Math.abs(status/1000).toInteger() & 0x4 ) &&
          Math.abs(chi2pid)<3
      }
      if(verbose) {
        println "----- candidate electrons:"
        println eleMap
      }

      if(eleMap.size()==0) continue
      if(eleMap.size()>1) {
        System.err << 
          "WARNING: found more than 1 trigger e- in event; " <<
          " using highest-E one\n"
      }
      eleDIS = eleMap.max{it.value.e()}.value
      if(verbose) {
        println "- eleDIS:"
        println eleDIS
      }


      // get hadrons which will be paired
      hadMapList = hadPIDs.collect{ findParticles(it) }
      if(verbose) {
        println "----- candidate hadrons"
        println hadMapList
      }

      // loop over pairs of hadron PIDs
      hadMapList.eachWithIndex { hadMapA, hadIdxA ->
        hadMapList.eachWithIndex { hadMapB, hadIdxB ->
          if( hadIdxB < hadIdxA ) return

          // loop over pairs of hadrons with the specified PIDs
          hadMapA.each { rowA, hadA ->
            hadMapB.each { rowB, hadB ->
              if(hadIdxA==hadIdxB && rowB<=rowA) return

              // fill ntuples
              fillParticleNt(eleNt,eleDIS)
              fillParticleNt(hadANt,hadA)
              fillParticleNt(hadBNt,hadB)
              evNt.fill(
                evnum,
                helicity
              )

              if(verbose) {
                20.times{print '.'}
                println " dihadrons"
              }


            }
          }
        }
      }



    } // end if event has specific banks

  } // end event loop
  reader.close()


  // write out to diskim file
  evNt.write()
  eleNt.write()
  hadANt.write()
  hadBNt.write()
  diskimFile.close()


  // close reader
  reader = null
  System.gc()
} // end loop over hipo files
