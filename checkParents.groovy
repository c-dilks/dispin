// test script for MC::Lund

import org.jlab.io.hipo.HipoDataSource
import java.lang.Math.*


////////////////////////
// ARGUMENTS
def inHipoName = "../data/skim/skim4_005052.hipo" // skim file
if(args.length>=1) inHipoName = args[0]


// open skim HIPO file
def reader = new HipoDataSource()
reader.open(inHipoName)
def event
def lundBank

// begin event loop
def evCount = 0
def pids = []
while(reader.hasEvent()) {
  //if(evCount>100000) break // limiter
  evCount++
  if(evCount % 100000 == 0) println "read $evCount events"

  event = reader.getNextEvent()

  if(event.hasBank("MC::Lund")) {

    lundBank = event.getBank("MC::Lund")
    pids = []
    (0..lundBank.rows()).each { r ->
      if(lundBank.getByte('parent',r)==1) {
        pids << lundBank.getInt('pid',r)
      }
    }
    println pids

  } // end if event has specific banks

} // end event loop
reader.close()


// close reader
reader = null
