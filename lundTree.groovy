// read MC HIPO files and prints tree of lund
// fragmentation from MC::Lund

import org.jlab.io.hipo.HipoDataSource
import groovy.json.JsonOutput
import clasqa.Tools
Tools T = new Tools()

def inHipoName
if(args.length>=1) inHipoName = args[0]
else {
  System.err << "ERROR: specify hipo file\n"
  return
}

println "INPUT FILE: $inHipoName"

reader = new HipoDataSource()
reader.open(inHipoName)

def evCount=0
def lundBank
def index, parent
def pid
def DT = [:]
def DTpath = []
def pathMap = [:]
while(reader.hasEvent()) {
  event = reader.getNextEvent()
  evCount++
  if(evCount % 100000 == 0) println "read $evCount events"

  // reset decay tree
  DT = [:]
  pathMap = [:]
  
  // loop through MC::Lund
  lundBank = event.getBank("MC::Lund")
  (0..<lundBank.rows()).each{

    index = lundBank.getByte('index',it)
    parent = lundBank.getByte('parent',it)
    pid = lundBank.getInt('pid',it)

    // DTpath is the decay path to current particle
    DTpath = pathMap.containsKey(parent) ? pathMap[parent] : [parent]
    DTpath << index

    // pathMap maps the particle with this index to its decay path
    pathMap[index] = DTpath

    // add this particle to the decay tree
    T.addLeaf(DT,DTpath,{['pid':pid]})
  }

  // print decay tree
  println T.pPrint(DT)
  print '-'*40
}
