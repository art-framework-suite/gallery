#include "gallery/BranchMapReader.h"

#include "gallery/EventHistoryGetter.h"
#include "gallery/throwFunctions.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/rootNames.h"

#include "TFile.h"
#include "TTree.h"

#include <memory>

class TBranch;

namespace gallery {

  void BranchMapReader::updateFile(TFile* tFile) {

    TTree* metaDataTree = dynamic_cast<TTree*>(tFile->Get(art::rootNames::metaDataTreeName().c_str()));
    if (!metaDataTree) {
      throwTreeNotFound(art::rootNames::metaDataTreeName());
    }

    std::unique_ptr<art::ProductRegistry> productRegistry(new art::ProductRegistry);
    art::ProductRegistry* productRegistryPtr = productRegistry.get();
    TBranch* productRegistryBranch =
      metaDataTree->GetBranch(art::rootNames::metaBranchRootName<art::ProductRegistry>());
    if (productRegistryBranch == nullptr) {
      throwBranchNotFound(art::rootNames::metaBranchRootName<art::ProductRegistry>());
    }
    metaDataTree->SetBranchAddress(art::rootNames::metaBranchRootName<art::ProductRegistry>(),
                                   &productRegistryPtr);

    art::BranchIDLists* branchIDListsPtr = &branchIDLists_;
    TBranch* branchIDListsBranch =
      metaDataTree->GetBranch(art::rootNames::metaBranchRootName<art::BranchIDLists>());
    if (branchIDListsBranch == nullptr) {
      throwBranchNotFound(art::rootNames::metaBranchRootName<art::BranchIDLists>());
    }
    metaDataTree->SetBranchAddress(art::rootNames::metaBranchRootName<art::BranchIDLists>(),
                                   &branchIDListsPtr);

    metaDataTree->GetEntry(0);

    branchIDToDescriptionMap_.clear();
    for (auto const& product : productRegistry->productList_) {
      art::BranchDescription const& branchDescription = product.second;
      if (branchDescription.branchType() == art::InEvent && branchDescription.branchID().isValid()) {
        branchIDToDescriptionMap_.emplace(std::make_pair(branchDescription.branchID(), branchDescription));
        allSeenBranchIDs_.insert(branchDescription.branchID());
      }
    }
  }

  void BranchMapReader::updateEvent(EventHistoryGetter* historyGetter) {
    branchListIndexes_ = historyGetter->history().branchListIndexes();
  }

  art::BranchID BranchMapReader::productToBranchID(art::ProductID const& pid) const {
    if (pid.isValid()) {
      size_t procIndex = pid.processIndex() - 1;
      if (procIndex < branchListIndexes_.size()) {
        art::BranchListIndex branchListIndex = branchListIndexes_[procIndex];
        if (branchListIndex < branchIDLists_.size()) {
          art::BranchIDList const& branchIDList = branchIDLists_[branchListIndex];
          size_t prodIndex = pid.productIndex() - 1;
          if (prodIndex < branchIDList.size()) {
            art::BranchID::value_type bid = branchIDList[prodIndex];
            return art::BranchID(bid);
          }
        }
      }
    }
    return art::BranchID();
  }

  art::BranchDescription const* BranchMapReader::productToBranch(art::ProductID const& pid) const {
    art::BranchID bid = productToBranchID(pid);
    auto bdi = branchIDToDescriptionMap_.find(bid);
    if (branchIDToDescriptionMap_.end() == bdi) {
      return nullptr;
    }
    return &bdi->second;
  }

  art::BranchDescription const* BranchMapReader::branchIDToBranch(art::BranchID const& bid) const {
    auto bdi = branchIDToDescriptionMap_.find(bid);
    if (branchIDToDescriptionMap_.end() == bdi) {
      return nullptr;
    }
    return &bdi->second;
  }

  bool BranchMapReader::branchInRegistryOfAnyOpenedFile(art::BranchID const& branchID) const {
    return allSeenBranchIDs_.find(branchID) != allSeenBranchIDs_.end();
  }
}
