#include "gallery/InfoForTypeLabelInstance.h"
#include "canvas/Utilities/Exception.h"
#include "canvas_root_io/Utilities/TypeTools.h"

#include "TClass.h"

using namespace std;

namespace {
  art::TypeID
  getPartnerTypeID(TClass* tClass)
  {
    art::TypeID result;
    if (tClass) {
      // Note we allow tClass to be null at this point. If the branch is
      // actually in the input file and there is an attempt to construct
      // a BranchData object with it null, a missing dictionary
      // exception will be thrown. This also means we just assume it is
      // not an Assns. Even if this assumption is wrong, we will get the
      // correct behavior from getByLabel. A missing dictionary
      // exception will get thrown if the branch is in the input file
      // and a ProductNotFound exception will get thrown if it is not.
      auto const wrappedClass = art::name_of_template_arg(tClass->GetName(), 0);
      auto const assnsPartner = art::name_of_assns_partner(wrappedClass);
      if (assnsPartner.empty()) {
        return result;
      }
      auto const wrappedPartnerClassName = art::wrappedClassName(assnsPartner);
      art::root::TypeWithDict const wrappedPartner(wrappedPartnerClassName);
      if (!wrappedPartner) {
        throw art::Exception(art::errors::DictionaryNotFound)
          << "In InfoForTypeLabelInstance constructor.\nMissing dictionary for "
             "wrapped partner of Assns class.\n"
          << wrappedPartnerClassName << "\n";
      }
      result = wrappedPartner.id();
    }
    return result;
  }
}

namespace gallery {
  InfoForTypeLabelInstance::InfoForTypeLabelInstance(art::TypeID const& iType,
                                                     string const& iLabel,
                                                     string const& iInstance)
    : type_{iType}
    , label_{iLabel}
    , instance_{iInstance}
    , tClass_{TClass::GetClass(type_.typeInfo())}
    , isAssns_{art::is_assns(art::name_of_template_arg(type_.className(), 0))}
    , partnerType_{getPartnerTypeID(tClass_)}
  {}

  art::TypeID const&
  InfoForTypeLabelInstance::type() const noexcept
  {
    return type_;
  }

  std::string const&
  InfoForTypeLabelInstance::label() const noexcept
  {
    return label_;
  }

  std::string const&
  InfoForTypeLabelInstance::instance() const noexcept
  {
    return instance_;
  }

  TClass*
  InfoForTypeLabelInstance::tClass() const noexcept
  {
    return tClass_;
  }

  bool
  InfoForTypeLabelInstance::isAssns() const noexcept
  {
    return isAssns_;
  }

  art::TypeID const&
  InfoForTypeLabelInstance::partnerType() const noexcept
  {
    return partnerType_;
  }

  std::vector<IndexProductIDPair>&
  InfoForTypeLabelInstance::processIndexToProductID() const noexcept
  {
    return processIndexToProductID_;
  }

  std::vector<art::ProductID>&
  InfoForTypeLabelInstance::productIDsOrderedByHistory() const noexcept
  {
    return productIDsOrderedByHistory_;
  }
}
