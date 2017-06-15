#ifndef gallery_BranchMapReader_h
#define gallery_BranchMapReader_h

// BranchMapReader can find the BranchDescription corresponding to a
// ProductID.

// BranchMapReader also maintains a set of all ProductIDs associated
// with branches in the Event and seen in all input files opened so
// far.

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchListIndex.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"

#include <map>
#include <memory>
#include <set>

class TFile;

namespace gallery {

  class EventHistoryGetter;

  class BranchMapReader {
  public:

    void updateFile(TFile* tFile);

    cet::exempt_ptr<art::BranchIDLists const> branchIDLists() const { return branchIDLists_.get(); }
    art::BranchDescription const* productToBranch(art::ProductID const& pid) const;
    bool branchInRegistryOfAnyOpenedFile(art::ProductID const&) const;

  private:

    std::unique_ptr<art::BranchIDLists> branchIDLists_{nullptr}; // Only for backwards compatibility
    std::map<art::ProductID, art::BranchDescription> productIDToDescriptionMap_{};
    std::set<art::ProductID> allSeenProductIDs_{};
  };
}
#endif /* gallery_BranchMapReader_h */

// Local Variables:
// mode: c++
// End:
