#include <iostream>
#include <vector>
#include <memory>
#include <TString.h>

#include "src/Constants.h"
#include "src/Tools.h"
#include "src/EventTree.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char** argv) {

  enum file_classes {
    cElectronInbending,
    cPositronInbending,
    cElectronOutbending,
    cPositronOutbending
  };
  std::vector<TString> classNames{
    "electron-inbending",
    "positron-inbending",
    "electron-outbending",
    "positron-outbending"
  };

  // parse arguments
  std::vector<TString> outrootDirs;
  if(argc<5) {
    cerr << "USAGE: " << argv[0];
    for(const auto& className : classNames)
      cerr << " [" << className << "]";
    cerr << endl << " - each argument should be an outroot dir" << endl;
    return 2;
  }
  for(int a=1; a<5; a++)
    outrootDirs.push_back(TString(argv[a]));
  auto whichPair = EncodePairType(kPip,kPim);

  // read event trees
  std::vector<std::unique_ptr<EventTree>> evTrees;
  cout << "READING DATA FROM:" << endl;
  for(const auto& outrootDir : outrootDirs) {
    cout << " - " << outrootDir << endl;
    evTrees.push_back(std::move(std::make_unique<EventTree>(outrootDir + "/*.root", whichPair)));
  }

  return 0;
}
