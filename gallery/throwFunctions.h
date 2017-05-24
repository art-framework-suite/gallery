#ifndef gallery_throwFunctions_h
#define gallery_throwFunctions_h

#include <string>

namespace gallery {

  void throwTreeNotFound(std::string const& treeName);
  void throwBranchNotFound(std::string const& branchName);
  void throwIllegalRandomAccess();
  void throwIllegalDecrement();
}

#endif /* gallery_throwFunctions_h */

// Local Variables:
// mode: c++
// End:
