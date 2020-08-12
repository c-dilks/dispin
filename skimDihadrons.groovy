// reads HIPO skim files and outputs an NTuple containing dihadron momenta, and
// the scattered electron momentum, along with everything else needed for dihadron
// spin asymmetry analysis
//
// some basic cuts are applied here; search for CUT (in all caps)
//

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
def inHipoName = "../data/skim/skim4_005052.hipo" // skim file
def dataStream = 'data' // 'data', 'mcrec', 'mcgen'
if(args.length>=1) inHipoName = args[0]
if(args.length>=2) dataStream = args[1]
////////////////////////
// OPTIONS
def verbose = 0
hadPIDlist = [ 211, -211 ] // list of hadron PIDs which will be paired
//hadPIDlist += [ 321, -321 ] // include kaons
////////////////////////



// define root file
"mkdir -p diskim".execute()
def diskimName = 'diskim/' + inHipoName.tokenize('/')[-1] + '.root'
def diskimFile = new ROOTFile(diskimName)
println "INPUT FILE: $inHipoName"
println "OUTPUT FILE: $diskimName"



def pPrint = { str -> JsonOutput.prettyPrint(JsonOutput.toJson(str)) }
def adjAngle = { ang ->
  while(ang<-Math.PI) ang += 2*Math.PI
  while(ang>=Math.PI) ang -= 2*Math.PI
  return ang
}
def undef = -10000.0


// define variables
def event
def particleBank, configBank, eventBank, calBank, trkBank, trajBank
def mcParticleBank
def eleTree
def hadTreeList
def mcgenTreeList
def eleDIS
def runnum
def evnum
def evnumLo, evnumHi
def helicity
def reader
def evCount
def pids = []
def pidsMC = []
def mcgenSet = []
def mcEle,mcHadA,mcHadB


// setup QA database
QADB qa = new QADB()


// check `dataStream` variable
def useMC
if(dataStream=='data') useMC=false
else if(dataStream=='mcrec') useMC=true
else if(dataStream=='mcgen') useMC=true
else {
  System.err << "ERROR: unrecognized dataStream\n"
  return
}
if(useMC) println "READING MONTE CARLO FILE"



//-----------------------
// detector leaves
//-----------------------

// closure to read calorimeter bank entries for specified row
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

// closure to read detector banks for specified particle
// - `pidx` is the row of `REC::Particle` of the associated particle
def getDetectorBranch = { pidx ->
  def detBr = [:]

  // calorimeter bank
  (0..calBank.rows()).each { r ->
    if(calBank.getShort('pindex',r) == pidx) {
      def layer = calBank.getByte('layer',r)
      def calStr = ''
      if(layer == DetectorLayer.PCAL_U) calStr = 'pcal' // layer 1
      else if(layer == DetectorLayer.EC_INNER_U) calStr = 'ecin' // layer 4
      else if(layer == DetectorLayer.EC_OUTER_U) calStr = 'ecout' // layer 7
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
def calorimeterList = ['pcal','ecin','ecout'] 
//def calorimeterList = ['pcal'] // pcal only

// closure to define ntuple leaves for detectors
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


// closure for filling detector ntuple leaves
def fillDetectorLeaves = { br ->
  def leaves = []
  def brDet = br['detector']
  def found
  /* calorimeters */
  calorimeterList.each{ det ->
    found = brDet.containsKey(det)
    leaves << (found ? 1.0 : 0.0)
    // if !found, all vars set to `undef`, except for energy, set to 0
    if(found) leaves << calorimeterLeafList.collect{ leaf -> brDet[det][leaf] }
    else leaves << calorimeterLeafList.collect{ leaf -> 
      leaf=='energy' ? 0 : undef
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
  if(found) {
    found = brDet['dcTraj'].containsKey('c1') &&
            brDet['dcTraj'].containsKey('c2') &&
            brDet['dcTraj'].containsKey('c3');
  }
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

// closure to read REC::Particle bank entries
// returns a tree (nested map) associated with specified PID
def growParticleTree = { pidList, pid ->

  // get list of bank row numbers corresponding to this PID
  def rowList = pidList.findIndexValues{ it == pid }.collect{it as Integer}

  // define particleTree, which is a list of branches, which are maps with
  //   'row' -> bank row number
  //   'particle' -> COATJAVA Particle object
  //   'status' -> REC::Particle::status
  //   'chi2pid' -> REC::Particle::chi2pid
  //   'vx,vy,vz' -> vertex
  //   etc.
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


// closure to read MC::Particle bank entries
// returns a tree (nested map) associated with specified PID
def growMCtree = { pidList, pid ->

  // get list of bank row numbers corresponding to this PID
  def rowList = pidList.findIndexValues{ it == pid }.collect{it as Integer}

  // define particleTree, which is a list of branches, which are maps with
  //   'row' -> bank row number
  //   'particle' -> COATJAVA Particle object
  //   'vx,vy,vz' -> vertex
  def particleTree = rowList.collect { row ->
    [
      'row': (float) row,
      'particle':new Particle(
        pid,
        *['px','py','pz'].collect{mcParticleBank.getFloat(it,row)}
      ),
      *:['vx','vy','vz'].collectEntries{[it,mcParticleBank.getFloat(it,row)]},
      'matchDist':10000
    ]
  }

  // verbose printing
  if(verbose) {
    println "- MC pid=$pid  found in rows $rowList"
    particleTree.each{ parBr -> println pPrint(parBr) }
  }

  return particleTree
}



// closure to define ntuple leaves for particles
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
def buildMCleaves = { par ->
  return [
    'Row',
    'Pid',
    'Px','Py','Pz',
    'E',
    'Vx','Vy','Vz',
    'matchDist'
  ].collect{par+'_'+it}
}

// closure to fill particle ntuple leaves
def fillParticleLeaves = { br ->
  def pid = br.particle.pid()
  return [
    br.row,
    pid,
    br.particle.px(), br.particle.py(), br.particle.pz(),
    br.particle.e(),
    br.vx, br.vy, br.vz,
    br.chi2pid, br.status, br.beta,
  ]
}
def fillMCleaves = { br ->
  if(br!=null) {
    // match was found
    return [
      br.row,
      br.particle.pid(),
      br.particle.px(), br.particle.py(), br.particle.pz(),
      br.particle.e(),
      br.vx, br.vy, br.vz,
      br.matchDist
    ]
  } else {
    // match was not found
    return [
      -1,
      undef,
      undef,undef,undef,
      undef,
      undef,undef,undef,
      undef
    ]
  }
}


//-----------------------------
// define the full ntuple
//-----------------------------
def NTleafNames = [
  *buildParticleLeaves('ele'), *buildDetectorLeaves('ele'),
  *buildParticleLeaves('hadA'), *buildDetectorLeaves('hadA'),
  *buildParticleLeaves('hadB'), *buildDetectorLeaves('hadB'),
  'runnum','evnumLo','evnumHi',
  'helicity'
].join(':')
if(useMC) {
  NTleafNames = [
    NTleafNames,
    *buildMCleaves('gen_ele'),
    *buildMCleaves('gen_hadA'),
    *buildMCleaves('gen_hadB')
  ].join(':')
}
//println NTleafNames
def NT = diskimFile.makeNtuple("ditr","ditr",NTleafNames)
def NTleaves



//----------------------
// event loop
//----------------------
def once = true
evCount = 0

// open skim HIPO file
reader = new HipoDataSource()
reader.open(inHipoName)

// begin event loop
while(reader.hasEvent()) {
  //if(evCount>100000) break // limiter
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
    if(useMC) {
      mcParticleBank = event.getBank("MC::Particle")
    }


    // get event-level information
    // - evnum may surpass float precision limit, if it has more than 7
    //   digits; since ntuples only store floats, we split evnum into
    //   two 16-bit halves; reconstruct full evnum with 
    //   `evnumLo+(evnumHi<<16)`
    helicity = eventBank.getByte('helicity',0)
    runnum = configBank.getInt('run',0)
    evnum = configBank.getInt('event',0)
    evnumLo = evnum & 0xFFFF
    evnumHi = (evnum>>16) & 0xFFFF
    if(once) {
      println "ANALYZING RUN $runnum"
      once = false
    }


    // CUT: data monitoring QA cut (not applied to MC)
    if(!useMC) {
      if(!qa.OkForAsymmetry(runnum,evnum)) {
        //println "toss file " + qa.getFilenum() + " (evnum=$evnum)"
        continue;
      }
    }


    // get list of PIDs, with list index corresponding to bank row
    pids = (0..<particleBank.rows()).collect{ 
      particleBank.getInt('pid',it)
    }
    if(verbose) println "PIDs = $pids"


    // read MC generated particles
    // - mcgenTreeList is a list of trees; one list element = one PID;
    //   each list element is a 'tree': a list of subtrees (branches), one for 
    //   for each particle with that PID
    if(useMC) {
      mcPids = (0..<mcParticleBank.rows()).collect{ 
        mcParticleBank.getInt('pid',it)
      }
      if(verbose) println "MC PIDs = $mcPids"
      mcgenTreeList = hadPIDlist.collect{ growMCtree(mcPids,it) }
      mcgenTreeList << growMCtree(mcPids,11)
      if(verbose) {
        println "MCGENTREELIST"
        println pPrint(mcgenTreeList)
      }
    }


    
    //----------------------------------
    // find the DIS electron
    //----------------------------------
    // CUT: select FD trigger electrons
    eleTree = growParticleTree(pids,11).findAll { br ->
      def status = br.status
      def chi2pid = br.chi2pid
      status<0 &&
        ( Math.abs(status/1000).toInteger() & 0x2 || 
          Math.abs(status/1000).toInteger() & 0x4 )
    }
    //if(verbose) { println "----- eleTree:"; println eleTree; }

    // skip event if no electron found
    if(eleTree.size()==0) continue

    // warn if more than one candidate DIS electron is found
    if(eleTree.size()>1) {
      System.err << 
        "WARNING: found more than 1 trigger e- in event; " <<
        " using highest-E one\n"
    }

    // CUT: choose maximum energy electron branch (only needed if more than
    //      one candidate DIS electron was found
    eleDIS = eleTree.max{it.particle.e()}
    if(verbose) { println "----- eleDIS:"; println eleDIS.particle; }


    //------------------------------
    // dihadron pairing
    //------------------------------

    // first build a list of hadron trees; one list element = one PID
    //   each list element is a 'tree': a list of subtrees (branches), one for 
    //   for each particle with that PID
    if(verbose) println "..... hadrons:"
    hadTreeList = hadPIDlist.collect{ growParticleTree(pids,it) }
    //if(verbose) { println "--- hadTreeList:"; println pPrint(hadTreeList); }

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


            // CUT: reject hadrons which are only in the CD; they will fail
            //      fiducial cuts because they do not have a DC trajectory,
            //      but rather only a CVT trajectory
            if( (hadA.status>=4000 && hadA.status<5000) ||
                (hadB.status>=4000 && hadB.status<5000) ) return


            // MC matching
            if(useMC) {
              mcgenSet = [eleDIS,hadA,hadB].collect{ rec ->
                def minDist = 10000.0
                def dist
                def match
                mcgenTreeList.each{ genTree ->
                  genTree.each{ gen ->
                    if(gen.particle.pid()==rec.particle.pid()) {
                      // calculate distance between gen and rec particles
                      dist = Math.sqrt(
                        Math.pow(adjAngle(gen.particle.phi()-rec.particle.phi()),2) +
                        Math.pow(adjAngle(Math.toRadians(gen.particle.theta()-rec.particle.theta())),2)
                      )
                      // find gen particle with minimum dist from rec
                      if(dist<minDist) {
                        match = gen // `match` is the matched mcgen particle
                        match.matchDist = dist // for ntuple
                        minDist = dist
                      }
                    }
                  }
                }
                // threshold distance for matching
                return minDist<10 ? match : null
              }
              mcEle = mcgenSet[0]
              mcHadA = mcgenSet[1]
              mcHadB = mcgenSet[2]
            }
                  

            // fill ntuple (be sure order matches defined order)
            NTleaves = [
              *fillParticleLeaves(eleDIS), *fillDetectorLeaves(eleDIS),
              *fillParticleLeaves(hadA), *fillDetectorLeaves(hadA),
              *fillParticleLeaves(hadB), *fillDetectorLeaves(hadB),
              runnum,evnumLo,evnumHi,
              helicity
            ]
            if(useMC) NTleaves += [
              *fillMCleaves(mcEle),
              *fillMCleaves(mcHadA),
              *fillMCleaves(mcHadB)
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
