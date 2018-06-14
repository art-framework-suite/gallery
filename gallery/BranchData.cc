#include "gallery/BranchData.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"
#include "canvas_root_io/Streamers/RefCoreStreamer.h"
#include "gallery/EventNavigator.h"

#include "TBranch.h"
#include "TClass.h"

namespace {
  art::EDProduct*
  calculateEDProductAddress(TClass* const tClass, void* address)
  {
    static TClass* const edProductTClass_s = TClass::GetClass("art::EDProduct");
    union {
      void* vp;
      unsigned char* ucp;
      art::EDProduct* edProduct;
    } pointerUnion;
    pointerUnion.vp = address;
    pointerUnion.ucp += tClass->GetBaseClassOffset(edProductTClass_s);
    return pointerUnion.edProduct;
  }
} // namespace

namespace gallery {

  BranchData::~BranchData()
  {
    if (tClass_) {
      tClass_->Destructor(address_);
    }
  }

  BranchData::BranchData(art::TypeID const& type,
                         TClass* iTClass,
                         TBranch* iBranch,
                         EventNavigator const* eventNavigator,
                         art::PrincipalBase const* finder,
                         std::string&& iBranchName)
    : tClass_{iTClass}
    , address_{(tClass_ != nullptr) ? tClass_->New() : nullptr}
    , edProduct_{calculateEDProductAddress(tClass_, address_)}
    , branch_{iBranch}
    , eventNavigator_{eventNavigator}
    , finder_{finder}
    , lastProduct_{-1}
    , branchName_{move(iBranchName)}
  {
    if (tClass_ == nullptr) {
      throw art::Exception(art::errors::LogicError)
        << "In BranchData constructor, no dictionary exists for type "
        << type.className();
    }
    if (address_ == nullptr) {
      throw art::Exception(art::errors::LogicError)
        << "In BranchData constructor, failed to construct type "
        << type.className();
    }
    if (branch_) {
      branch_->SetAddress(&address_);
    }
  }

  void
  BranchData::updateFile(TBranch* iBranch)
  {
    branch_ = iBranch;
    if (branch_) {
      branch_->SetAddress(&address_);
    }
    lastProduct_ = -1;
  }

  art::EDProduct const*
  BranchData::getIt_() const
  {
    if (branch_ == nullptr) {
      return nullptr;
    }
    long long entry = eventNavigator_->eventEntry();
    if (entry != lastProduct_) {
      // haven't gotten the data for this event
      art::configureRefCoreStreamer(finder_.get());
      branch_->GetEntry(entry);
      art::configureRefCoreStreamer();
      lastProduct_ = entry;
    }
    if (edProduct_->isPresent()) {
      return edProduct_;
    }
    return nullptr;
  }

  art::EDProduct const*
  BranchData::uniqueProduct_() const
  {
    return getIt_();
  }

  art::EDProduct const*
  BranchData::uniqueProduct_(art::TypeID const&) const
  {
    return getIt_();
  }

} // namespace gallery
