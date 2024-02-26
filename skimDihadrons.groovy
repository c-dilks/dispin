// reads HIPO skim files and outputs a TTree containing dihadron momenta, and
// the scattered electron momentum, along with everything else needed for dihadron
// spin asymmetry analysis
//
// some basic cuts are applied here; search for CUT (in all caps)

// there may be additional an additional CUT or two in calcKinematics.cpp
//

import org.jlab.io.hipo.HipoDataSource
import org.jlab.clas.physics.Particle
import org.jlab.detector.base.DetectorLayer
import org.jlab.detector.base.DetectorType
import org.jlab.jroot.ROOTFile
import groovy.json.JsonOutput
import java.lang.Math.*
import clasqa.QADB


////////////////////////
// ARGUMENTS
def inHipoName = "../data/skim/skim4_005052.hipo" // skim file, or directory of DST files
def dataStream = 'data' // 'data', 'mcrec', 'mcgen'
def inHipoType = "skim" // options: "skim", "dst"
if(args.length>=1) inHipoName = args[0]
if(args.length>=2) dataStream = args[1]
if(args.length>=3) inHipoType = args[2]
////////////////////////
// OPTIONS
def verbose = 0
hadPIDlist = [ 211, -211 ] // list of hadron PIDs which will be paired
//hadPIDlist += [ 2212 ] // proton, antiproton
//hadPIDlist += [ 22 ] // photons (not a hadron, but to be paired for pi0s)
//hadPIDlist += [ 321, -321 ] // kaons
//hadPIDlist += [ 2212, -2212 ] // proton, antiproton
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
def lundBank
def eleTree
def hadTreeList
def mcgenTreeList
def eleDIS
def runnum
def evnum
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
def useMC    = false // true if any MC mode is set
def useMCgen = false // true if reading only MC generated particles
def useMCrec = false // true if reading MC reconstructed particles, with matched generated particles
if(dataStream=='data') {
  useMC = false
}
else if(dataStream=='mcrec') {
  useMC    = true
  useMCrec = true
}
else if(dataStream=='mcgen') {
  useMC    = true
  useMCgen = true
}
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
  calBr['sector'] = calBank.getByte('sector',c)
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
      //if(verbose) println "-> calorimeter layer $layer"
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
        detBr['dcTrk']['status'] = trkBank.getShort('status',r)
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
  'sector/I', 'energy', 'time', 'path',
  'x', 'y', 'z',
  'lu', 'lv', 'lw'
]

// list of  calorimeters
def calorimeterList = ['pcal','ecin','ecout']
//def calorimeterList = ['pcal'] // pcal only

// closure to define tree leaves for detectors
def buildDetectorLeaves = {
  return [
    /* calorimeters */
    calorimeterList.collect{ detName ->
      ["${detName}_found/I"] +
      calorimeterLeafList.collect{ varName -> "${detName}_${varName}" }
    },
    /* tracking */
    ['found/I','chi2','ndf','status/I'].collect{ varName -> "dcTrk_${varName}" },
    /* trajectories */
    'dcTraj_found/I',
    (1..3).collect{ reg ->
      ['x','y','z'].collect{ coord -> "dcTraj_c${reg}${coord}" }
    }
  ].flatten()
}


// closure for filling detector tree leaves
def fillDetectorLeaves = { br ->
  def leaves = []
  def brDet = br['detector']
  def found
  /* calorimeters */
  calorimeterList.each{ det ->
    found = brDet.containsKey(det)
    leaves << (found ? 1 : 0)
    // if !found, all vars set to `undef`, except for energy, set to 0
    if(found) leaves << calorimeterLeafList.collect{ leaf -> brDet[det][leaf.split('/').first()] }
    else leaves << calorimeterLeafList.collect{ leaf -> 
      leaf=='energy' ? 0 : undef
    }

  }
  /* tracking */
  found = brDet.containsKey('dcTrk')
  leaves << (found ? 1 : 0)
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
  leaves << (found ? 1 : 0)
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
    def result = [
      'row':row,
      'particle':new Particle(
        pid,
        *['px','py','pz'].collect{particleBank.getFloat(it,row)}
      ),
      *:['vx','vy','vz'].collectEntries{[it,particleBank.getFloat(it,row)]},
    ]
    if(!useMCgen) {
      result += [
        'chi2pid':  particleBank.getFloat('chi2pid',row),
        'status':   particleBank.getShort('status',row),
        'beta':     particleBank.getFloat('beta',row),
        'detector': getDetectorBranch(row),
      ]
    }
    else { // if(useMCgen)
      // get parent info (NOTE: `particleBank` in this context is actually `MC::Lund`)
      def parent_idx = particleBank.getByte('parent',row)
      def parent_row = (0..<particleBank.rows()).find{ parent_idx == particleBank.getByte('index',it) }
      def parent_pid
      if(parent_row != null) {
        parent_pid = particleBank.getInt('pid',parent_row)
      } else {
        parent_idx = -1
        parent_pid = -1
      }
      result += [
        'chi2pid':   0.0,
        'status':    0,
        'beta':      0.0,
        'parentIdx': parent_idx,
        'parentPid': parent_pid,
        'type':      particleBank.getByte('type',row),
      ]
    }
    return result
  }
  .findAll{ // filter out MC-generated beam particles
    if(useMCgen) {
      return it.type != 21 // FIXME: 21 seems to be beam particle for data we looked at so far, but this may not be generally true
    }
    else {
      return true
    }
  }


  // verbose printing
  if(verbose) {
    println "- pid=$pid  found in rows $rowList"
    particleTree.each{ parBr ->
      print " row=" + parBr.row
      //print " status=" + parBr.status
      //println " chi2pid=" + parBr.chi2pid
      println parBr.particle
      //println pPrint(parBr)
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
  // -- these are the generated particles from MC::Particle
  def particleTree = rowList.collect { row ->
    [
      'row':row,
      'particle':new Particle(
        pid,
        *['px','py','pz'].collect{mcParticleBank.getFloat(it,row)}
      ),
      *:['vx','vy','vz'].collectEntries{[it,mcParticleBank.getFloat(it,row)]},
      'matchDist':10000
    ]
  }

  // obtain parent particle info from MC::Lund; particles in MC::Lund are matched
  // to those in MC::Particle by requiring matching momentum
  def lundIdx,parentIdx
  def lundPid,parentPid
  def lundPx,lundPy,lundPz
  def genPx,genPy,genPz
  def genPid
  // - loop through MC::Particle particles, stored in particleTree
  particleTree.each{ br ->
    genPid = br.particle.pid()
    genPx = br.particle.px()
    genPy = br.particle.py()
    genPz = br.particle.pz()
    // - search for matching particle in MC::Lund
    lundIdx = -1
    (0..<lundBank.rows()).each{
      lundPid = lundBank.getInt('pid',it)
      lundPx = lundBank.getFloat('px',it)
      lundPy = lundBank.getFloat('py',it)
      lundPz = lundBank.getFloat('pz',it)
      if(genPid==lundPid) {
        if( Math.abs(genPx-lundPx)<0.0001 && 
            Math.abs(genPy-lundPy)<0.0001 && 
            Math.abs(genPz-lundPz)<0.0001 ) {
          lundIdx = lundBank.getByte('index',it)
          parentIdx = lundBank.getByte('parent',it)
        }
      }
    }
    // - if match found in MC::Lund, get parent PID
    if(lundIdx>=0) {
      def parentRow = (0..<lundBank.rows()).find{
        parentIdx == lundBank.getByte('index',it)
      }
      parentPid = lundBank.getInt('pid',parentRow)
    }
    else {
      parentIdx = -1
      parentPid = -1
    }
    br['parentIdx'] = parentIdx
    br['parentPid'] = parentPid
  }


  // verbose printing
  if(verbose) {
    println "- MC pid=$pid  found in rows $rowList"
    particleTree.each{ parBr -> println pPrint(parBr) }
  }

  return particleTree
}



// closure to define tree leaves for particles
def buildParticleLeaves = { par ->
  def result = [
    'Row/I',
    'Pid/I',
    'Px','Py','Pz',
    'E',
    'Vx','Vy','Vz',
    'chi2pid','status/I','beta'
  ]
  if(!useMCgen) {
    result += buildDetectorLeaves()
  }
  else {
    result += [
      'parentIdx/I',
      'parentPid/I',
    ]
  }
  return result.collect{par+'_'+it}
}
def buildMCrecLeaves = { par ->
  return [
    'Row/I',
    'Pid/I',
    'Px','Py','Pz',
    'E',
    'Vx','Vy','Vz',
    'matchDist',
    'parentIdx/I','parentPid/I'
  ].collect{par+'_'+it}
}

// closure to fill particle tree leaves
def fillParticleLeaves = { br ->
  def pid = br.particle.pid()
  result = [
    br.row,
    pid,
    br.particle.px(), br.particle.py(), br.particle.pz(),
    br.particle.e(),
    br.vx, br.vy, br.vz,
    br.chi2pid, br.status, br.beta,
  ]
  if(!useMCgen) {
    result += fillDetectorLeaves(br)
  }
  else {
    result += [
      br.parentIdx,
      br.parentPid,
    ]
  }
  return result
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
      br.matchDist,
      br.parentIdx,br.parentPid
    ]
  } else {
    // match was not found
    return [
      -1,
      undef,
      undef,undef,undef,
      undef,
      undef,undef,undef,
      undef,
      undef,undef
    ]
  }
}


//----------------------------------------------
// define the full tree: `ditr` (diskim tree)
//----------------------------------------------

// tree leaf names
def ditrLeafNames = [
  'runnum/I', 'evnum/I', 'helicity/I',
  *buildParticleLeaves('ele'),
  *buildParticleLeaves('hadA'),
  *buildParticleLeaves('hadB'),
]
if(useMCrec) {
  ditrLeafNames += [
    *buildMCrecLeaves('gen_ele'),
    *buildMCrecLeaves('gen_hadA'),
    *buildMCrecLeaves('gen_hadB')
  ]
}

// set default datatype to `float` (not necessary, since j2root will do this, but safe)
ditrLeafNames = ditrLeafNames.collect{ it.contains('/') ? it : it+'/F' }
println "ditr will have ${ditrLeafNames.size()} branches:"
println pPrint(ditrLeafNames)

// define the tree
def ditr = diskimFile.makeTree("ditr","ditr",ditrLeafNames.join(':'))
def ditrLeafVals


//----------------------
// event loop
//----------------------
def once = true
def eventHasPipPim = false
def cntEventHasPipPim = 0
def cntPairPipPim = 0
def cnt
evCount = 0

// get list of HIPO files (if reading DSTs)
def inHipoList = []
if(inHipoType=="dst") {
  def inHipoDirObj = new File(inHipoName)
  def inHipoFilter = ~/.*\.hipo/
  inHipoDirObj.traverse( type: groovy.io.FileType.FILES, nameFilter: inHipoFilter ) {
    if(it.size()>0) inHipoList << inHipoName+"/"+it.getName()
  }
  inHipoList.sort()
  if(inHipoList.size()==0) {
    System.err << "ERROR: no hipo files found in this directory\n"
    return
  }
}
else if(inHipoType=="skim") { inHipoList << inHipoName }
else {
  System.err << "ERROR: unknown inHipoType setting\n"
  return
}


// HIPO file loop (only 1 file, if reading a skim file)
inHipoList.each { inHipoFile ->

  // open HIPO file
  reader = new HipoDataSource()
  reader.open(inHipoFile)

  // begin event loop
  while(reader.hasEvent()) {
    //if(evCount>500) break // limiter
    evCount++
    if(evCount % 100000 == 0) println "read $evCount events"
    if(verbose) { 30.times{print '='}; println " begin event" }

    event = reader.getNextEvent()

    if(event.hasBank("REC::Particle") &&
       event.hasBank("REC::Event") &&
       event.hasBank("RUN::config") ) {

      // get banks
      particleBank = useMCgen ? event.getBank("MC::Lund") : event.getBank("REC::Particle")
      eventBank    = event.getBank("REC::Event")
      configBank   = event.getBank("RUN::config")
      calBank      = event.getBank("REC::Calorimeter")
      trkBank      = event.getBank("REC::Track")
      trajBank     = event.getBank("REC::Traj")
      if(useMCrec) {
        mcParticleBank = event.getBank("MC::Particle")
        lundBank       = event.getBank("MC::Lund")
      }


      // get event-level information
      helicity = eventBank.getByte('helicity',0)
      runnum   = configBank.getInt('run',0)
      evnum    = configBank.getInt('event',0)
      if(once) {
        println "ANALYZING RUN $runnum"
        once = false
      }
      if(verbose) println "evnum = $evnum"


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


      // read MC generated particles, for matching to MC reconstructed particles
      // - mcgenTreeList is a list of trees; one list element = one PID;
      //   each list element is a 'tree': a list of subtrees (branches), one for
      //   for each particle with that PID
      if(useMCrec) {
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
      eleTree = growParticleTree(pids,11)
      //if(verbose) { println "----- eleTree:"; println eleTree; }

      // skip event if no electron found
      if(eleTree.size()==0) continue


      // CUT: choose maximum energy electron branch (only needed if more than
      //      one candidate DIS electron was found)
      eleDIS = eleTree.max{it.particle.e()}
      // CUT: electron must be in FD trigger
      if(!useMCgen) {
        if(eleDIS.status <= -3000 || eleDIS.status > -2000) continue
      }

      if(verbose) { println "----- eleDIS:"; println eleDIS.particle; }


      //------------------------------
      // dihadron pairing
      //------------------------------

      // first build a list of hadron trees; one list element = one PID
      //   each list element is a 'tree': a list of subtrees (branches), one for
      //   for each particle with that PID
      eventHasPipPim = false
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
          if( hadTreeA.size()==0 || hadTreeB.size()==0) return

          // loop over pairs of hadrons with the specified PIDs
          hadTreeA.each { hadA ->
            hadTreeB.each { hadB ->

              // like PIDs -> take all permutations of pairs (no repetition)
              // unlike PIDs -> take all combinations of pairs
              if(hadIdxA==hadIdxB && hadB.row <= hadA.row) return


              // CUT: reject hadrons which are only in the CD; they will fail
              //      fiducial cuts because they do not have a DC trajectory,
              //      but rather only a CVT trajectory
              if(!useMCgen) {
                if( (hadA.status>=4000 && hadA.status<5000) ||
                    (hadB.status>=4000 && hadB.status<5000) ) return
              }

              // MC matching
              if(useMCrec) {
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
                          match.matchDist = dist // for output tree
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


              // fill tree (be sure order matches defined order from `ditrLeafNames`)
              ditrLeafVals = [
                runnum, evnum, helicity,
                *fillParticleLeaves(eleDIS),
                *fillParticleLeaves(hadA),
                *fillParticleLeaves(hadB),
              ]
              if(useMCrec)
                ditrLeafVals += [
                  *fillMCleaves(mcEle),
                  *fillMCleaves(mcHadA),
                  *fillMCleaves(mcHadB)
                ]

              // typecast to expected TTree datatypes
              ditrLeafVals_casted = [ditrLeafNames, ditrLeafVals].transpose().collect{ leafName, leafVal ->
                if      (leafName.contains('/I')) (int)    leafVal
                else if (leafName.contains('/F')) (float)  leafVal
                else {
                  System.err << "WARNING: j2root does not support datatype for branch $leafName\n"
                  (float) leafVal
                }
              }
              // [ ditrLeafNames, ditrLeafVals_casted, ditrLeafVals_casted.collect{it.getClass()} ].transpose().each{println it}

              // fill tree
              ditr.fill(ditrLeafVals_casted)

              // print dihadron hadrons
              if(verbose) {
                20.times{print '.'}
                println " dihadron "+
                  hadPIDlist[hadIdxA]+" "+hadPIDlist[hadIdxB]+"  rows "+
                  hadA.row+" "+hadB.row
                println hadA.particle
                println hadB.particle
              }

              if( (hadA.particle.pid()==211 && hadB.particle.pid()==-211) ||
                  (hadA.particle.pid()==-211 && hadB.particle.pid()==211)) {
                eventHasPipPim = true
                cntPairPipPim++
              }

            }
          }
        }
      }

      if(eventHasPipPim) cntEventHasPipPim++

    } // end if event has specific banks

  } // end event loop
  println "END EVENT LOOP"
  reader.close()
  reader = null
} // end HIPO file loop


// write out to diskim file
ditr.write()
diskimFile.close()

println "number of events with at least one pi+pi- pair: $cntEventHasPipPim"
println "number of pi+pi- pairs: $cntPairPipPim"
