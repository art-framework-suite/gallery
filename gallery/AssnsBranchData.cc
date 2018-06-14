#include "gallery/AssnsBranchData.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Utilities/Exception.h"

#include <utility>

using namespace std;

namespace gallery {

  AssnsBranchData::~AssnsBranchData() = default;

  AssnsBranchData::AssnsBranchData(art::TypeID const& type,
                                   TClass* iTClass,
                                   TBranch* iBranch,
                                   EventNavigator const* eventNavigator,
                                   art::PrincipalBase const* finder,
                                   string&& iBranchName,
                                   art::TypeID const& infoType,
                                   art::TypeID const& infoPartnerType)
    : BranchData{type,
                 iTClass,
                 iBranch,
                 eventNavigator,
                 finder,
                 move(iBranchName)}
  {
    if (type == infoType) {
      secondary_wrapper_type_ = infoPartnerType;
    } else {
      secondary_wrapper_type_ = infoType;
    }
  }

  void
  AssnsBranchData::updateFile(TBranch* iBranch)
  {
    secondaryLastProduct_ = -1;
    secondaryProduct_.reset();
    BranchData::updateFile(iBranch);
  }

  art::EDProduct const*
  AssnsBranchData::getIt_() const
  {
    throw art::Exception(art::errors::LogicError, "AmbiguousProduct")
      << "AssnsBranchData uniqueProduct() called without specifying which type "
         "was wanted.\n"
      << "branch name is " << branchName() << "\n"
      << "Possibly you tried to use a Ptr that points into an Assns, which is "
         "not allowed and does not work\n";
  }

  art::EDProduct const*
  AssnsBranchData::uniqueProduct_() const
  {
    throw art::Exception(art::errors::LogicError, "AmbiguousProduct")
      << "AssnsBranchData uniqueProduct() called without specifying which type "
         "was wanted.\n"
      << "branch name is " << branchName() << "\n"
      << "Possibly you tried to use a Ptr that points into an Assns, which is "
         "not allowed and does not work\n";
  }

  art::EDProduct const*
  AssnsBranchData::uniqueProduct_(art::TypeID const& wanted_wrapper_type) const
  {
    art::EDProduct const* primaryAssns = BranchData::getIt_();
    if (primaryAssns && (wanted_wrapper_type == secondary_wrapper_type_)) {
      if (secondaryLastProduct_ != lastProduct()) {
        secondaryProduct_ =
          primaryAssns->makePartner(wanted_wrapper_type.typeInfo());
        secondaryLastProduct_ = lastProduct();
      }
      return secondaryProduct_.get();
    }
    return primaryAssns;
  }

} // namespace gallery
