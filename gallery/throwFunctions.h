#ifndef art_FWLite_throwFunctions_h
#define art_FWLite_throwFunctions_h

#include <string>

namespace canvas {

  void throwTreeNotFound(std::string const& treeName);
  void throwBranchNotFound(std::string const& branchName);
}

#endif /* art_FWLite_throwFunctions_h */

// Local Variables:
// mode: c++
// End:
