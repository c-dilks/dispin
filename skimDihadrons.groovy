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


// list of hipo files (one for now, but allows for future
// capability to read DST files in a list)
def inHipoList = []
inHipoList << inHipo


// get runnum
def runnum
if(inHipo.contains('postprocess'))
  runnum = inHipo.tokenize('.')[-2].tokenize('/')[-1].toInteger()
else
  runnum = inHipo.tokenize('.')[-2].tokenize('_')[-1].toInteger()
println "runnum=$runnum"


"mkdir -p diskim".execute()

def pPrint = { str -> JsonOutput.prettyPrint(JsonOutput.toJson(str)) }


// define variables
def event
def particleBank, configBank, eventBank, calBank
def eleMap
def disElectron
def eventNum
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
  println "pid=$pid  found in rows $rowList"
  particleMap.each{ row, par ->
    def status = particleBank.getShort('status',row)
    def chi2pid = particleBank.getFloat('chi2pid',row)
    println " row=$row  status=$status  chi2pid=$chi2pid"
    println par
  }
  return particleMap
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
    //if(evCount>100000) break // limiter
    evCount++
    if(evCount % 100000 == 0) println "read $evCount events"
    println "============================="
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
      eventNum = BigInteger.valueOf(configBank.getInt('event',0))


      // get list of PIDs, with list index corresponding to bank row
      pidList = (0..<particleBank.rows()).collect{ 
        particleBank.getInt('pid',it)
      }
      println "pidList = $pidList"


      
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
      println "----------------"
      println eleMap
      if(eleMap.size()>1) {
        System.err << 
          "WARNING: found more than 1 trigger e- in event; " <<
          " using highest-E one\n"
      }
      if(eleMap.size()>0) {
        disElectron = eleMap.max{it.value.e()}.value
        println "disElectron:"
        println disElectron
      }



      // CUT: find scattered electron: highest-E electron such that 2 < E < 11
      /*
      disElectron = eleMap.findAll{ it.e()>2 && it.e()<11 }.max{it.e()}
      if(disElectron) {

        // calculate Q2
        vecQ.copy(vecBeam)
        vecEle.copy(disElectron.vector())
        vecQ.sub(vecEle) 
        Q2 = -1*vecQ.mass2()

        // calculate W
        vecW.copy(vecBeam)
        vecW.add(vecTarget)
        vecW.sub(vecEle)
        W = vecW.mass()

        // calculate x and y
        nu = vecBeam.e() - vecEle.e()
        x = Q2 / ( 2 * 0.938272 * nu )
        y = nu / EBEAM


        // CUT: Q2 and W and y
        if( Q2>1 && W>2 && y<0.8) {

          // get lists of pions
          pipList = findParticles(211)
          pimList = findParticles(-211)

          // calculate pion kinematics and fill histograms
          // countEvent will be set to true if a pion is added to the histos 
          countEvent = false
          fillHistos(pipList,'pip')
          fillHistos(pimList,'pim')

          if(countEvent) {

            // fill event-level histograms
            histTree.DIS.Q2.fill(Q2)
            histTree.DIS.W.fill(W)
            histTree.DIS.x.fill(x)
            histTree.DIS.y.fill(y)
            histTree.DIS.Q2VsW.fill(W,Q2)

            // increment event counter

          }
        }
      }
      */


    } // end if event has specific banks

  } // end event loop
  reader.close()


  // close reader
  reader = null
  System.gc()
} // end loop over hipo files
