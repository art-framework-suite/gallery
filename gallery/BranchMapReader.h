#ifndef gallery_BranchMapReader_h
#define gallery_BranchMapReader_h

// BranchMapReader can find the BranchDescription
// corresponding to a ProductID. One step in this
// is being able to convert a ProductID to a
// BranchID.

// BranchMapReader also maintains a set of all
// BranchIDs associated with branches in the Event
// and seen in all input files opened so far.

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchIDList.h"
#include "canvas/Persistency/Provenance/BranchListIndex.h"

#include <map>
#include <set>

class TFile;

namespace gallery {

  class EventHistoryGetter;

  class BranchMapReader {
  public:

    void updateFile(TFile* tFile);
    void updateEvent(EventHistoryGetter*  historyGetter);

    art::BranchID productToBranchID(art::ProductID const& pid) const;
    art::BranchDescription const* productToBranch(art::ProductID const& pid) const;
    art::BranchDescription const* branchIDToBranch(art::BranchID const& bid) const;
    bool branchInRegistryOfAnyOpenedFile(art::BranchID const&) const;

  private:

    art::BranchIDLists branchIDLists_;
    std::map<art::BranchID, art::BranchDescription> branchIDToDescriptionMap_;
    art::BranchListIndexes branchListIndexes_;
    std::set<art::BranchID> allSeenBranchIDs_;
  };
}
#endif /* gallery_BranchMapReader_h */

// Local Variables:
// mode: c++
// End:
