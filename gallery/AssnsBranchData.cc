#include "gallery/AssnsBranchData.h"

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Utilities/Exception.h"

#include <utility>

namespace gallery {

  AssnsBranchData::AssnsBranchData(art::TypeID const& type,
                                   TClass* iTClass,
                                   TBranch* iBranch,
                                   EventNavigator const* eventNavigator,
                                   art::EDProductGetterFinder const* finder,
                                   std::string&& iBranchName,
                                   art::TypeID const& infoType,
                                   art::TypeID const& infoPartnerType) :
    BranchData{type, iTClass, iBranch, eventNavigator, finder, std::move(iBranchName)}
  {
    if (type == infoType) {
      secondary_wrapper_type_ = infoPartnerType;
    }
    else {
      secondary_wrapper_type_ = infoType;
    }
  }

  void AssnsBranchData::updateFile(TBranch* iBranch)
  {
    secondaryLastProduct_ = -1;
    secondaryProduct_.reset();
    BranchData::updateFile(iBranch);
  }

  art::EDProduct const*
  AssnsBranchData::getIt() const
  {
    return uniqueProduct();
  }

  art::EDProduct const*
  AssnsBranchData::uniqueProduct() const
  {
    throw art::Exception(art::errors::LogicError, "AmbiguousProduct")
      << "AssnsBranchData uniqueProduct() called without specifying which type was wanted.\n"
      << "branch name is " << branchName() << "\n"
      << "Possibly you tried to use a Ptr that points into an Assns, which is not allowed and does not work\n";
  }

  art::EDProduct const*
  AssnsBranchData::uniqueProduct(art::TypeID const& wanted_wrapper_type) const
  {
    art::EDProduct const* primaryAssns = BranchData::getIt();
    if (primaryAssns &&
        wanted_wrapper_type == secondary_wrapper_type_) {

      if (secondaryLastProduct_ != lastProduct()) {
        secondaryProduct_ = primaryAssns->makePartner(wanted_wrapper_type.typeInfo());
        secondaryLastProduct_ = lastProduct();
      }
      return secondaryProduct_.get();
    }
    return primaryAssns;
  }
}
