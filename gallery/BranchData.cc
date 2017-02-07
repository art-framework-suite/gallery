#include "gallery/BranchData.h"

#include "gallery/EventNavigator.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"

#include "TBranch.h"
#include "TClass.h"

namespace {
  art::EDProduct *
  calculateEDProductAddress(TClass * const tClass, void * address)
  {
    static TClass * const edProductTClass_s =
      TClass::GetClass("art::EDProduct");

    union {
      void* vp;
      unsigned char* ucp;
      art::EDProduct* edProduct;
    } pointerUnion;
    pointerUnion.vp = address;
    pointerUnion.ucp += tClass->GetBaseClassOffset(edProductTClass_s);
    return pointerUnion.edProduct;
  }
}

namespace gallery {

  BranchData::BranchData() :
    // This is an invalid BranchData object initialized
    // such that getIt always returns a nullptr.
    tClass_(nullptr),
    address_(nullptr),
    edProduct_(nullptr),
    branch_(nullptr),
    eventNavigator_(nullptr),
    finder_(nullptr),
    lastProduct_(-1),
    branchName_()
  {
  }

  BranchData::BranchData(art::TypeID const& type,
                         TClass* iTClass,
                         TBranch* iBranch,
                         EventNavigator const* eventNavigator,
                         art::EDProductGetterFinder const* finder,
                         std::string&& iBranchName) :
    tClass_(iTClass),
    address_(tClass_ != nullptr ? tClass_->New() : nullptr),
    edProduct_(calculateEDProductAddress(tClass_, address_)),
    branch_(iBranch),
    eventNavigator_(eventNavigator),
    finder_(finder),
    lastProduct_(-1),
    branchName_(std::move(iBranchName))
  {
    if (tClass_ == nullptr) {
      throw art::Exception(art::errors::LogicError)
        << "In BranchData constructor, no dictionary exists for type " << type.className();
    }
    if (address_ == nullptr) {
      throw art::Exception(art::errors::LogicError)
        << "In BranchData constructor, failed to construct type " << type.className();
    }

    if (branch_) {
      branch_->SetAddress(&address_);
    }
  }

  BranchData::~BranchData() {
    if (tClass_) {
      tClass_->Destructor(address_);
    }
  }

  void BranchData::updateFile(TBranch* iBranch) {
    branch_ = iBranch;
    if (branch_) {
      branch_->SetAddress(&address_);
    }
    lastProduct_ = -1;
  }

  art::EDProduct const* BranchData::getIt() const {
    if (branch_) {
      long long entry = eventNavigator_->eventEntry();
      if (entry != lastProduct_) {
        //haven't gotten the data for this event
        art::configureRefCoreStreamer(finder_);
        branch_->GetEntry(entry);
        art::configureRefCoreStreamer();
        lastProduct_ = entry;
      }
      if (edProduct_->isPresent()) {
        return edProduct_;
      }
    }
    return nullptr;
  }

  art::EDProduct const* BranchData::anyProduct() const {
    throw art::Exception(art::errors::LogicError)
      << "BranchData::anyProduct not implemented. Should not be called.";
    return nullptr;
  }

  art::EDProduct const* BranchData::uniqueProduct() const {
    return getIt();
  }

  art::EDProduct const* BranchData::uniqueProduct(art::TypeID const&) const {
    return getIt();
  }

  bool BranchData::resolveProduct(art::TypeID const&) const {
    throw art::Exception(art::errors::LogicError)
      << "BranchData::resolveProduct not implemented. Should not be called.";
    return false;
  }

  bool BranchData::resolveProductIfAvailable(art::TypeID const&) const {
    throw art::Exception(art::errors::LogicError)
      << "BranchData::resolveProductIfAvailable not implemented. Should not be called.";
    return false;
  }
}
