#include "gallery/throwFunctions.h"

#include "canvas/Utilities/Exception.h"

namespace canvas {

  void throwTreeNotFound(std::string const& treeName) {
    throw art::Exception(art::errors::FileReadError)
      << "Could not find the TTree named \'"
      << treeName
      << "\' in the input file.\nMaybe it is not an art format file or maybe it is corrupted.\n";
  }

  void throwBranchNotFound(std::string const& branchName) {
    throw art::Exception(art::errors::FileReadError)
      << "Could not find the TBranch named \'"
      << branchName
      << "\' in the input file.\nMaybe it is not an art format file or maybe it is corrupted.\n";
  }
}
