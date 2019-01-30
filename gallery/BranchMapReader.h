#ifndef gallery_BranchMapReader_h
#define gallery_BranchMapReader_h

// BranchMapReader can find the BranchDescription corresponding to a
// ProductID.

// BranchMapReader also maintains a set of all ProductIDs associated
// with branches in the Event and seen in all input files opened so
// far.

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"
#include "gallery/InfoForTypeLabelInstance.h"

#include <map>
#include <memory>
#include <set>

class TFile;

namespace gallery {

  class EventHistoryGetter;

  class BranchMapReader {
  public:
    void updateFile(TFile* tFile);

    art::BranchDescription const* productDescription(
      InfoForTypeLabelInstance const& info,
      std::string const& process) const;

    art::BranchDescription const* productDescription(
      art::TypeID const& type,
      std::string const& label,
      std::string const& instance,
      std::string const& process) const;

    art::BranchDescription const* productDescription(
      art::ProductID const pid) const;

    std::map<art::ProductID, art::BranchDescription> const&
    productDescriptions() const;

    cet::exempt_ptr<art::BranchIDLists const> branchIDLists() const;

    bool branchInRegistryOfAnyOpenedFile(art::ProductID const&) const;

  private:
    std::unique_ptr<art::BranchIDLists> branchIDLists_{
      nullptr}; // Only for backwards compatibility
    std::map<art::ProductID, art::BranchDescription>
      productIDToDescriptionMap_{};
    std::set<art::ProductID> allSeenProductIDs_{};
  };
} // namespace gallery
#endif /* gallery_BranchMapReader_h */

// Local Variables:
// mode: c++
// End:
