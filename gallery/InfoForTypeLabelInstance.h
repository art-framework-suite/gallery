#ifndef gallery_InfoForTypeLabelInstance_h
#define gallery_InfoForTypeLabelInstance_h
// vim: set sw=2 expandtab :

// ===================================================================
// Type to facilitate lookup a product given the given module label,
// instance name, type and optionally the process name.
// ===================================================================

#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/TypeID.h"

#include <string>
#include <utility>
#include <vector>

class TClass;

namespace gallery {

  using IndexProductIDPair = std::pair<unsigned int, art::ProductID>;

  class InfoForTypeLabelInstance {
  public:
    explicit InfoForTypeLabelInstance(art::TypeID const& iType,
                                      std::string const& iLabel,
                                      std::string const& iInstance);

    art::TypeID const& type() const noexcept;
    std::string const& label() const noexcept;
    std::string const& instance() const noexcept;

    TClass* tClass() const noexcept;
    bool isAssns() const noexcept;
    art::TypeID const& partnerType() const noexcept;

    std::vector<IndexProductIDPair>& processIndexToProductID() const noexcept;
    std::vector<art::ProductID>& productIDsOrderedByHistory() const noexcept;

  private:
    art::TypeID const type_;
    std::string const label_;
    std::string const instance_;
    TClass* const tClass_;
    bool const isAssns_;
    art::TypeID const partnerType_;

    // There is an entry here for each process with a branch that is
    // in the ProductRegistry and in the input file for any input file
    // that contains events and was opened so far and the product
    // registry entry must be associated with the Event and have a
    // valid ProductID (the corresponding TBranch does not necessarily
    // need to exist in the current input file).  These are maintained
    // in the order of the processIndex. The second part of the pair
    // is an index into branchDataVector_.
    mutable std::vector<IndexProductIDPair> processIndexToProductID_;

    // There is an entry here for each process in the current process
    // history with a branch in the current input ROOT file.  They are
    // maintained in process history order.
    mutable std::vector<art::ProductID> productIDsOrderedByHistory_;
  };
} // namespace gallery

#endif /* gallery_InfoForTypeLabelInstance_h */

// Local Variables:
// mode: c++
// End:
