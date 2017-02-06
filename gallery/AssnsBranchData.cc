#include "gallery/AssnsBranchData.h"

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Utilities/Exception.h"

#include <algorithm>
#include <utility>

#include "cetlib/container_algorithms.h"
#include <iostream>
#include <iterator>

namespace {
  std::vector<art::TypeID>
  calcTypes(art::TypeID const& type,
            art::TypeID infoType,
            std::vector<art::TypeID> infoPartnerTypes)
  {
    using std::swap;
    std::cerr << type << ", " << infoType << ", ";
    cet::copy_all(infoPartnerTypes, std::ostream_iterator<art::TypeID>(std::cerr, ", "));
    if (type != infoType) {
      auto const it = std::find(infoPartnerTypes.begin(), infoPartnerTypes.end(), type);
      if (it != infoPartnerTypes.cend()) {
        swap(infoType, *it);
      } else {
        throw art::Exception(art::errors::LogicError, "AssnsBranchData()")
          << "constructed with an inconsistent sequence of types!";
      }
    }
    std::cerr << "\n  -> ";
    cet::copy_all(infoPartnerTypes, std::ostream_iterator<art::TypeID>(std::cerr, ", "));
    std::cerr << "\n";
    return infoPartnerTypes;
  }
}

namespace gallery {

  AssnsBranchData::AssnsBranchData(art::TypeID const& type,
                                   TClass* iTClass,
                                   TBranch* iBranch,
                                   EventNavigator const* eventNavigator,
                                   art::EDProductGetterFinder const* finder,
                                   std::string&& iBranchName,
                                   art::TypeID const& infoType,
                                   std::vector<art::TypeID> const & infoPartnerTypes) :
    BranchData(type, iTClass, iBranch,
               eventNavigator, finder, std::move(iBranchName)),
    secondary_wrapper_types_(calcTypes(type, infoType, infoPartnerTypes)),
    secondaryProducts_()
  { }

  AssnsBranchData::~AssnsBranchData() { }

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
    
    if (primaryAssns) {
      auto const i = std::find(secondary_wrapper_types_.cbegin(),
                               secondary_wrapper_types_.cend(),
                               wanted_wrapper_type);
      if (i != secondary_wrapper_types_.cend()) {
        auto sp = secondaryProducts_.begin();
        std::advance(sp, std::distance(secondary_wrapper_types_.cbegin(), i));
        if (sp->get() == nullptr) {
          *sp = (primaryAssns->makePartner(wanted_wrapper_type.typeInfo()));
        }
        return sp->get();
      }
    }
    return primaryAssns;
  }

  void
  AssnsBranchData::doResetProducts() const
  {
    secondaryProducts_.clear();
  }
}
