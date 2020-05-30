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
def verbose = false
////////////////////////


// list of hipo files (one for now, but allows for future
// capability to read DST files in a list)
def inHipoList = []
inHipoList << inHipo


// set up list of hadrons to pair into dihadrons
hadPIDlist = [ 211, -211 ]
//hadPIDlist += [ 321, -321 ] // include kaons


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
def eleTree
def hadTreeList
def eleDIS
def evnum
def evnumLo, evnumHi
def helicity
def reader
def evCount
def detIdEC = DetectorType.ECAL.getDetectorId()


// subroutine which returns a tree of information about particles
// with the specified PID; the tree is called `particleTree`
def pidList = []
def growParticleTree = { pid ->

  // get list of bank row numbers corresponding to this PID
  def rowList = pidList.findIndexValues{ it == pid }.collect{it as Integer}

  // define particleTree, which is a list of branches, which are maps with
  //   'row' -> bank row number
  //   'particle' -> COATJAVA Particle object
  //   'status' -> REC::Particle::status
  //   'chi2pid' -> REC::Particle::chi2pid
  def particleTree = rowList.collect { row ->
    [
      'row':row,
      'particle':new Particle(
        pid,
        *['px','py','pz'].collect{particleBank.getFloat(it,row)}
      ),
      'status':particleBank.getShort('status',row),
      'chi2pid':particleBank.getFloat('chi2pid',row)
    ]
  }

  // verbose printing
  if(verbose) {
    println "- pid=$pid  found in rows $rowList"
    particleTree.each{ parBr ->
      //print " row=" + parBr.row
      //print " status=" + parBr.status
      //println " chi2pid=" + parBr.chi2pid
      println parBr.particle
    }
  }

  return particleTree
}




//------------------------------------------
// setup diskim file and ntuple `NT`
//------------------------------------------

"mkdir -p diskim".execute()
def diskimFile = new ROOTFile('diskim/test.root')


// subroutines for particle ntuple leaves
// - return list of particle leaf names
def buildParticleLeaves = { name ->
  return [
    'Pid',
    'Px','Py','Pz',
    'E',
    'chi2pid','status'
  ].collect{name+it}
}
// - return list of leaf values associated with the particle branch b
def fillParticleLeaves = { br ->
  def pid = br.particle.pid()
  //if(pid==11) // TODO for electrons, set pid to something useful, 
                // e.g. +1 for trigger elec, -1 for FT elec
  return [
    pid,
    br.particle.px(), br.particle.py(), br.particle.pz(),
    br.particle.e(),
    br.chi2pid, br.status
  ]
}


// define ntuple NT (be sure order matches NT.fill call)
def NT = diskimFile.makeNtuple("ditr","ditr",[
  *buildParticleLeaves('ele'),
  *buildParticleLeaves('hadA'),
  *buildParticleLeaves('hadB'),
  'evnumLo','evnumHi',
  'helicity'
].join(':'))




//-------------------
// read HIPO file(s)
//-------------------

// loop over hipo files (single file if skim file)
evCount = 0
inHipoList.each { inHipoFile ->

  // open skim/DST file
  reader = new HipoDataSource()
  reader.open(inHipoFile)


  //----------------------
  // event loop
  //----------------------
  while(reader.hasEvent()) {
    //if(evCount>10000) break // limiter
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
      // - evnum may surpass float precision limit, if it has more than 7
      //   digits; since ntuples only store floats, we split evnum into
      //   two 16-bit halves; reconstruct full evnum with 
      //   `evnumLo+(evnumHi<<16)`
      helicity = eventBank.getByte('helicity',0)
      evnum = configBank.getInt('event',0)
      evnumLo = evnum & 0xFFFF
      evnumHi = (evnum>>16) & 0xFFFF


      // get list of PIDs, with list index corresponding to bank row
      pidList = (0..<particleBank.rows()).collect{ 
        particleBank.getInt('pid',it)
      }
      if(verbose) println "pidList = $pidList"


      
      //----------------------------------
      // find the scattered electron
      //----------------------------------
      // - first get the electrons which satisfy cuts
      // - then choose from this list the electron with the highest E
      eleTree = growParticleTree(11).findAll { br ->
        def status = br.status
        def chi2pid = br.chi2pid
        status<0 &&
          ( Math.abs(status/1000).toInteger() & 0x2 || 
            Math.abs(status/1000).toInteger() & 0x4 ) &&
          Math.abs(chi2pid)<3
      }
      //if(verbose) { println "----- eleTree:"; println eleTree; }

      if(eleTree.size()==0) continue
      if(eleTree.size()>1) {
        System.err << 
          "WARNING: found more than 1 trigger e- in event; " <<
          " using highest-E one\n"
      }

      // choose maximum energy electron branch
      eleDIS = eleTree.max{it.particle.e()}
      if(verbose) { println "----- eleDIS:"; println eleDIS.particle; }


      //------------------------------
      // dihadron pairing
      //------------------------------

      // first build a list of hadron trees; one list element = one PID
      if(verbose) println "..... hadrons:"
      hadTreeList = hadPIDlist.collect{ growParticleTree(it) }
      //if(verbose) { println "--- hadTreeList:"; println hadTreeList; }

      // then loop over pairs of hadron PIDs
      // (`Idx` is a local ID, defined as the index of the PID in `hadPIDlist`)
      hadTreeList.eachWithIndex { hadTreeA, hadIdxA ->
        hadTreeList.eachWithIndex { hadTreeB, hadIdxB ->

          // take only permutations of PIDs, with repetition allowed
          if( hadIdxB < hadIdxA ) return

          // proceed only if there are one or more hadrons for each PID
          if( hadTreeA.size()==0 || hadTreeB.size==0) return;
  
          // loop over pairs of hadrons with the specified PIDs
          hadTreeA.each { hadA ->
            hadTreeB.each { hadB ->

              // like PIDs -> take all permutations of pairs (no repetition)
              // unlike PIDs -> take all combinations of pairs
              if(hadIdxA==hadIdxB && hadB.row <= hadA.row) return

              // fill ntuple (be sure order matches defined order)
              NT.fill(
                *fillParticleLeaves(eleDIS),
                *fillParticleLeaves(hadA),
                *fillParticleLeaves(hadB),
                evnumLo,evnumHi,
                helicity
              )

              // print dihadron hadrons
              if(verbose) { 
                20.times{print '.'}
                println " dihadron "+
                  hadPIDlist[hadIdxA]+" "+hadPIDlist[hadIdxB];
                println hadA.particle
                println hadB.particle
              }

            }
          }
        }
      }



    } // end if event has specific banks

  } // end event loop
  reader.close()


  // write out to diskim file
  NT.write()
  diskimFile.close()


  // close reader
  reader = null
  System.gc()
} // end loop over hipo files
