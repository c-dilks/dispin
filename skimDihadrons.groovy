import org.jlab.io.hipo.HipoDataSource
import org.jlab.clas.physics.Particle
import org.jlab.detector.base.DetectorLayer
import org.jlab.detector.base.DetectorType
import org.jlab.jroot.ROOTFile
import org.jlab.jroot.TNtuple
import groovy.json.JsonOutput
import java.lang.Math.*
import clasqa.QADB


////////////////////////
// ARGUMENTS
def inHipo = "../data/skim/skim4_5052.hipo" // skim file
if(args.length>=1) inHipo = args[0]
////////////////////////
// OPTIONS
def verbose = 0
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

// define root file
"mkdir -p diskim".execute()
def diskimFile = new ROOTFile('diskim/test.root')



def pPrint = { str -> JsonOutput.prettyPrint(JsonOutput.toJson(str)) }
def undef = -10000.0


// define variables
def event
def particleBank, configBank, eventBank, calBank, trkBank, trajBank
def eleTree
def hadTreeList
def eleDIS
def evnum
def evnumLo, evnumHi
def helicity
def reader
def evCount
def pidList = []


// setup QA database
QADB qa = new QADB()


//-----------------------
// detector leaves
//-----------------------

// read calorimeter bank entries for specified row
def getCalorimeterLeaves = { c ->
  def calBr = [:]
  calBr['sector'] = (float) calBank.getByte('sector',c)
  calBr['energy'] = calBank.getFloat('energy',c)
  calBr['time'] = calBank.getFloat('time',c)
  calBr['path'] = calBank.getFloat('path',c)
  calBr << ['x','y','z'].collectEntries{[it,calBank.getFloat(it,c)]}
  calBr << ['lu','lv','lw'].collectEntries{[it,calBank.getFloat(it,c)]}
  return calBr
}

// read detector banks for specified particle
// - `pidx` is the row of `REC::Particle` of the associated particle
def getDetectorBranch = { pidx ->
  def detBr = [:]

  // calorimeter bank
  (0..calBank.rows()).each { r ->
    if(calBank.getShort('pindex',r) == pidx) {
      def layer = calBank.getByte('layer',r)
      def calStr = ''
      if(layer == DetectorLayer.PCAL_U) calStr = 'pcal' // layer 1
      //else if(layer == DetectorLayer.EC_INNER_U) calStr = 'ecin' // layer 4
      //else if(layer == DetectorLayer.EC_OUTER_U) calStr = 'ecout' // layer 7
      if(!calStr.isEmpty()) detBr[calStr] = getCalorimeterLeaves(r)
      if(verbose) println "-> calorimeter layer $layer"
    }
  }

  // tracking bank
  (0..trkBank.rows()).each { r ->
    if(trkBank.getShort('pindex',r) == pidx) {
      def detector = trkBank.getByte('detector',r)
      if(detector == DetectorType.DC.getDetectorId()) { // DC = 6
        if(!detBr.containsKey('dcTrk')) detBr['dcTrk'] = [:]
        detBr['dcTrk']['chi2'] = trkBank.getFloat('chi2',r)
        detBr['dcTrk']['ndf'] = (float) trkBank.getShort('NDF',r)
        detBr['dcTrk']['status'] = (float) trkBank.getShort('status',r)
      }
    }
  }

  // trajectory bank
  (0..trajBank.rows()).each { r ->
    if(trajBank.getShort('pindex',r) == pidx) {
      def detector = trajBank.getByte('detector',r)
      if(detector == DetectorType.DC.getDetectorId()) { // DC = 6
        if(!detBr.containsKey('dcTraj')) detBr['dcTraj'] = [:]
        def layer = trajBank.getByte('layer',r)
        def region = 0
        if(layer==6) region=1
        else if(layer==18) region=2
        else if(layer==36) region=3
        if(region>0) {
          detBr['dcTraj']["c$region"] = ['x','y','z'].collectEntries{
            [it,trajBank.getFloat(it,r)]
          }
        }
      }
    }
  }

  return detBr
}

// list of variables associated to calorimeters
def calorimeterLeafList = [
  'sector', 'energy', 'time', 'path',
  'x', 'y', 'z',
  'lu', 'lv', 'lw'
]

// list of  calorimeters
//def calorimeterList = ['pcal','ecin','ecout'] 
def calorimeterList = ['pcal'] // pcal only

// define ntuple leaves for detectors
def buildDetectorLeaves = { par ->
  return [
    /* calorimeters */
    calorimeterList.collect{ detName ->
      ["${detName}_found"] +
      calorimeterLeafList.collect{ varName -> "${detName}_${varName}" }
    },
    /* tracking */
    ['found','chi2','ndf','status'].collect{ varName -> "dcTrk_${varName}" },
    /* trajectories */
    'dcTraj_found',
    (1..3).collect{ reg ->
      ['x','y','z'].collect{ coord -> "dcTraj_c${reg}${coord}" }
    }
  ].flatten().collect{par+'_'+it}
}


// fill detector ntuple leaves
def fillDetectorLeaves = { br ->
  def leaves = []
  def brDet = br['detector']
  def found
  /* calorimeters */
  calorimeterList.each{ det ->
    found = brDet.containsKey(det)
    leaves << (found ? 1.0 : 0.0)
    leaves << calorimeterLeafList.collect{ leaf ->
      found ? brDet[det][leaf] : undef
    }
  }
  /* tracking */
  found = brDet.containsKey('dcTrk')
  leaves << (found ? 1.0 : 0.0)
  leaves << ['chi2','ndf','status'].collect{
    found ? brDet['dcTrk'][it] : undef
  }
  /* trajectories */
  found = brDet.containsKey('dcTraj')
  leaves << (found ? 1.0 : 0.0)
  (1..3).each{ reg ->
    leaves << ['x','y','z'].collect{ 
      found ? brDet['dcTraj']["c$reg"][it] : undef
    }
  }
  return leaves.flatten()
}




//------------------------------------------
// particle leaves
//------------------------------------------

// read REC::Particle bank entries; this subroutine returns a tree
// associated with specified PID
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
      'row': (float) row,
      'particle':new Particle(
        pid,
        *['px','py','pz'].collect{particleBank.getFloat(it,row)}
      ),
      *:['vx','vy','vz'].collectEntries{[it,particleBank.getFloat(it,row)]},
      'chi2pid':particleBank.getFloat('chi2pid',row),
      'status': (float) particleBank.getShort('status',row),
      'beta':particleBank.getFloat('beta',row),
      'detector':getDetectorBranch(row)
    ]
  }

  // verbose printing
  if(verbose) {
    println "- pid=$pid  found in rows $rowList"
    particleTree.each{ parBr ->
      //print " row=" + parBr.row
      //print " status=" + parBr.status
      //println " chi2pid=" + parBr.chi2pid
      //println parBr.particle
      println pPrint(parBr)
    }
  }

  return particleTree
}

// define ntuple leaves for particles
def buildParticleLeaves = { par ->
  return [
    'Row',
    'Pid',
    'Px','Py','Pz',
    'E',
    'Vx','Vy','Vz',
    'chi2pid','status','beta'
  ].collect{par+'_'+it}
}

// fill particle ntuple leaves
def fillParticleLeaves = { br ->
  def pid = br.particle.pid()
  //if(pid==11) // TODO for electrons, set pid to something useful, 
                // e.g. +1 for trigger elec, -1 for FT elec
  return [
    br.row,
    pid,
    br.particle.px(), br.particle.py(), br.particle.pz(),
    br.particle.e(),
    br.vx, br.vy, br.vz,
    br.chi2pid, br.status, br.beta,
  ]
}


//-----------------------------
// define the full ntuple
//-----------------------------
def NTleafNames = [
  *buildParticleLeaves('ele'), *buildDetectorLeaves('ele'),
  *buildParticleLeaves('hadA'),
  *buildParticleLeaves('hadB'),
  'evnumLo','evnumHi',
  'helicity'
].join(':')
//println NTleafNames
def NT = diskimFile.makeNtuple("ditr","ditr",NTleafNames)
def NTleaves





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
      trkBank = event.getBank("REC::Track")
      trajBank = event.getBank("REC::Traj")


      // get event-level information
      // - evnum may surpass float precision limit, if it has more than 7
      //   digits; since ntuples only store floats, we split evnum into
      //   two 16-bit halves; reconstruct full evnum with 
      //   `evnumLo+(evnumHi<<16)`
      helicity = eventBank.getByte('helicity',0)
      evnum = configBank.getInt('event',0)
      evnumLo = evnum & 0xFFFF
      evnumHi = (evnum>>16) & 0xFFFF


      // query QA database
      if(!qa.golden(runnum,evnum)) {
        //println "toss file " + qa.getFilenum() + " (evnum=$evnum)"
        continue;
      }


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
              NTleaves = [
                *fillParticleLeaves(eleDIS), *fillDetectorLeaves(eleDIS),
                *fillParticleLeaves(hadA),
                *fillParticleLeaves(hadB),
                evnumLo,evnumHi,
                helicity
              ]
              NT.fill(*NTleaves)
              //println NTleaves//.size()

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
