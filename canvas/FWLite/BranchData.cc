#include "canvas/FWLite/BranchData.h"

#include "canvas/FWLite/EventNavigator.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"

#include "TBranch.h"
#include "TClass.h"

namespace canvas {

  BranchData::BranchData() :
    // This is an invalid BranchData object initialized
    // such that getIt always returns a nullptr.
    tClass_(nullptr),
    address_(nullptr),
    edProduct_(nullptr),
    branch_(nullptr),
    eventNavigator_(nullptr),
    finder_(nullptr),
    lastProduct_(-1) {
  }

  BranchData::BranchData(art::TypeID const& type,
                         TBranch* iBranch,
                         TClass* edProductTClass,
                         EventNavigator const* eventNavigator,
                         art::EDProductGetterFinder const* finder,
                         std::string&& iBranchName) :
    tClass_(TClass::GetClass(type.typeInfo())),
    address_(tClass_ != nullptr ? tClass_->New() : nullptr),
    edProduct_(nullptr),
    branch_(iBranch),
    eventNavigator_(eventNavigator),
    finder_(finder),
    lastProduct_(-1) {

    if (tClass_ == nullptr) {
      throw art::Exception(art::errors::LogicError)
        << "In BranchData constructor, no dictionary exists for type " << type.className();
    }
    if (address_ == nullptr) {
      throw art::Exception(art::errors::LogicError)
        << "In BranchData constructor, failed to construct type " << type.className();
    }

    branchName_.swap(iBranchName);
    union {
      void* vp;
      unsigned char* ucp;
      art::EDProduct* edProduct;
    } pointerUnion;
    pointerUnion.vp = address_;
    pointerUnion.ucp += tClass_->GetBaseClassOffset(edProductTClass);
    edProduct_ =  pointerUnion.edProduct;

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
    throw art::Exception(art::errors::LogicError)
      << "BranchData::uniqueProduct not implemented. Should not be called.";
    return nullptr;
  }

  art::EDProduct const* BranchData::uniqueProduct(art::TypeID const&) const {
    throw art::Exception(art::errors::LogicError)
      << "BranchData::uniqueProduct not implemented. Should not be called.";
    return nullptr;
  }

  bool BranchData::resolveProduct(bool, art::TypeID const&) const {
    throw art::Exception(art::errors::LogicError)
      << "BranchData::resolveProduct not implemented. Should not be called.";
    return false;
  }

  bool BranchData::resolveProductIfAvailable(bool, art::TypeID const&) const {
    throw art::Exception(art::errors::LogicError)
      << "BranchData::resolveProductIfAvailable not implemented. Should not be called.";
    return false;
  }
}
