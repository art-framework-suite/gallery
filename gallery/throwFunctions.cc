#include "gallery/throwFunctions.h"

#include "canvas/Utilities/Exception.h"

namespace gallery {

  void
  throwTreeNotFound(std::string const& treeName)
  {
    throw art::Exception(art::errors::FileReadError)
      << "Could not find the TTree named \'" << treeName
      << "\' in the input file.\nMaybe it is not an art format file or maybe "
         "it is corrupted.\n";
  }

  void
  throwBranchNotFound(std::string const& branchName)
  {
    throw art::Exception(art::errors::FileReadError)
      << "Could not find the TBranch named \'" << branchName
      << "\' in the input file.\nMaybe it is not an art format file or maybe "
         "it is corrupted.\n";
  }

  void
  throwIllegalRandomAccess()
  {
    throw art::Exception(art::errors::FileReadError)
      << "Random access is not allowed.\n"
      << "Please make sure your gallery::Event was not created using more than "
         "one file.\n";
  }

  void
  throwIllegalDecrement()
  {
    throw art::Exception(art::errors::LogicError)
      << "This Event can not be decremented.\n"
      << "Please make sure atEnd() is not true for this Event.";
  }
} // namespace gallery
