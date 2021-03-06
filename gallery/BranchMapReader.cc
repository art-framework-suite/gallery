#include "gallery/BranchMapReader.h"

#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas_root_io/Streamers/BranchDescriptionStreamer.h"
#include "gallery/InfoForTypeLabelInstance.h"
#include "gallery/throwFunctions.h"

#include "TFile.h"
#include "TTree.h"

#include <memory>

class TBranch;

namespace gallery {

  void
  BranchMapReader::updateFile(TFile* tFile)
  {

    std::unique_ptr<TTree> metaDataTree{
      tFile->Get<TTree>(art::rootNames::metaDataTreeName().c_str())};
    if (!metaDataTree) {
      throwTreeNotFound(art::rootNames::metaDataTreeName());
    }

    auto productRegistry = std::make_unique<art::ProductRegistry>();
    auto* productRegistryPtr = productRegistry.get();
    TBranch* productRegistryBranch = metaDataTree->GetBranch(
      art::rootNames::metaBranchRootName<art::ProductRegistry>());
    if (productRegistryBranch == nullptr) {
      throwBranchNotFound(
        art::rootNames::metaBranchRootName<art::ProductRegistry>());
    }
    metaDataTree->SetBranchAddress(
      art::rootNames::metaBranchRootName<art::ProductRegistry>(),
      &productRegistryPtr);

    // To support files that contain BranchIDLists
    branchIDLists_.reset(nullptr);
    art::BranchIDLists branchIDLists;
    bool hasBranchIDLists{false};
    if (metaDataTree->GetBranch(
          art::rootNames::metaBranchRootName<art::BranchIDLists>())) {
      hasBranchIDLists = true;
      auto branchIDListsPtr = &branchIDLists;
      metaDataTree->SetBranchAddress(
        art::rootNames::metaBranchRootName<art::BranchIDLists>(),
        &branchIDListsPtr);
    }

    metaDataTree->GetEntry(0);

    // Necessary only for supporting conversion of an old Product ID
    // schema to the current one
    if (hasBranchIDLists) {
      branchIDLists_ =
        std::make_unique<art::BranchIDLists>(std::move(branchIDLists));
      metaDataTree->SetBranchAddress(
        art::rootNames::metaBranchRootName<art::BranchIDLists>(), nullptr);
    }

    productIDToDescriptionMap_.clear();
    for (auto const& product : productRegistry->productList_) {
      art::BranchDescription const& branchDescription = product.second;
      if (branchDescription.branchType() != art::InEvent)
        continue;
      if (!branchDescription.productID().isValid())
        continue;

      art::detail::BranchDescriptionStreamer::fluffRootTransients(
        branchDescription);
      productIDToDescriptionMap_.emplace(branchDescription.productID(),
                                         branchDescription);
      allSeenProductIDs_.insert(branchDescription.productID());
    }
  }

  art::BranchDescription const*
  BranchMapReader::productDescription(InfoForTypeLabelInstance const& info,
                                      std::string const& process) const
  {
    return productDescription(
      info.type(), info.label(), info.instance(), process);
  }

  art::BranchDescription const*
  BranchMapReader::productDescription(art::TypeID const& type,
                                      std::string const& label,
                                      std::string const& instance,
                                      std::string const& process) const
  {
    auto const fcn = type.friendlyClassName();
    auto match = [&fcn, &label, &instance, &process](auto const& pr) {
      auto const& pd = pr.second;
      return pd.friendlyClassName() == fcn && pd.moduleLabel() == label &&
             pd.productInstanceName() == instance &&
             pd.processName() == process;
    };
    auto const end = cend(productIDToDescriptionMap_);
    auto it = std::find_if(cbegin(productIDToDescriptionMap_), end, match);
    return it == end ? nullptr : &it->second;
  }

  art::BranchDescription const*
  BranchMapReader::productDescription(art::ProductID const pid) const
  {
    auto bdi = productIDToDescriptionMap_.find(pid);
    if (bdi == cend(productIDToDescriptionMap_)) {
      return nullptr;
    }
    return &bdi->second;
  }

  std::map<art::ProductID, art::BranchDescription> const&
  BranchMapReader::productDescriptions() const
  {
    return productIDToDescriptionMap_;
  }

  cet::exempt_ptr<art::BranchIDLists const>
  BranchMapReader::branchIDLists() const
  {
    return branchIDLists_.get();
  }

  bool
  BranchMapReader::branchInRegistryOfAnyOpenedFile(
    art::ProductID const& productID) const
  {
    return allSeenProductIDs_.find(productID) != allSeenProductIDs_.end();
  }
} // namespace gallery
