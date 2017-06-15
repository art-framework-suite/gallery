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

    auto productRegistry = std::make_unique<art::ProductRegistry>();
    auto* productRegistryPtr = productRegistry.get();
    TBranch* productRegistryBranch = metaDataTree->GetBranch(art::rootNames::metaBranchRootName<art::ProductRegistry>());
    if (productRegistryBranch == nullptr) {
      throwBranchNotFound(art::rootNames::metaBranchRootName<art::ProductRegistry>());
    }
    metaDataTree->SetBranchAddress(art::rootNames::metaBranchRootName<art::ProductRegistry>(), &productRegistryPtr);

    // To support files that contain BranchIDLists
    branchIDLists_.reset(nullptr);
    art::BranchIDLists branchIDLists;
    bool hasBranchIDLists{false};
    if (metaDataTree->GetBranch(art::rootNames::metaBranchRootName<art::BranchIDLists>())) {
      hasBranchIDLists = true;
      auto branchIDListsPtr = &branchIDLists;
      metaDataTree->SetBranchAddress(art::rootNames::metaBranchRootName<art::BranchIDLists>(), &branchIDListsPtr);
    }

    metaDataTree->GetEntry(0);

    // Necessary only for supporting conversion of an old Product ID
    // schema to the current one
    if (hasBranchIDLists) {
      branchIDLists_ = std::make_unique<art::BranchIDLists>(std::move(branchIDLists));
      metaDataTree->SetBranchAddress(art::rootNames::metaBranchRootName<art::BranchIDLists>(), nullptr);
    }

    productIDToDescriptionMap_.clear();
    for (auto const& product : productRegistry->productList_) {
      art::BranchDescription const& branchDescription = product.second;
      if (branchDescription.branchType() == art::InEvent && branchDescription.productID().isValid()) {
        productIDToDescriptionMap_.emplace(std::make_pair(branchDescription.productID(), branchDescription));
        allSeenProductIDs_.insert(branchDescription.productID());
      }
    }
  }

  art::BranchDescription const* BranchMapReader::productToBranch(art::ProductID const& pid) const
  {
    auto bdi = productIDToDescriptionMap_.find(pid);
    if (productIDToDescriptionMap_.end() == bdi) {
      return nullptr;
    }
    return &bdi->second;
  }

  bool BranchMapReader::branchInRegistryOfAnyOpenedFile(art::ProductID const& productID) const
  {
    return allSeenProductIDs_.find(productID) != allSeenProductIDs_.end();
  }
}
