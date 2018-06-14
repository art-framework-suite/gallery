#include "gallery/DataGetterHelper.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchListIndex.h"
#include "canvas/Persistency/Provenance/Compatibility/type_aliases.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"

#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "canvas/Utilities/uniform_type_name.h"
#include "gallery/AssnsBranchData.h"
#include "gallery/EventNavigator.h"

#include "canvas_root_io/Streamers/AssnsStreamer.h"
#include "canvas_root_io/Streamers/BranchDescriptionStreamer.h"
#include "canvas_root_io/Streamers/CacheStreamers.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"
#include "canvas_root_io/Streamers/RefCoreStreamer.h"
#include "canvas_root_io/Streamers/TransientStreamer.h"
#include "canvas_root_io/Streamers/setPtrVectorBaseStreamer.h"
#include "canvas_root_io/Utilities/TypeTools.h"

#include "TClass.h"
#include "TTree.h"

#include <algorithm>
#include <cstring>

using namespace std;

namespace {

  constexpr char underscore{'_'};
  constexpr char period{'.'};

  void
  initializeStreamers()
  {
    art::setCacheStreamers();
    art::setProvenanceTransientStreamers();
    art::detail::setBranchDescriptionStreamer();
    art::detail::setPtrVectorBaseStreamer();
    art::configureProductIDStreamer();
    art::configureRefCoreStreamer();
  }

  string
  buildBranchName(gallery::InfoForTypeLabelInstance const& info,
                  string const& processName)
  {
    string branchName{info.type().friendlyClassName()};
    unsigned int const branchNameSize =
      branchName.size() + info.label().size() + info.instance().size() +
      processName.size() + 4;
    branchName.reserve(branchNameSize);
    branchName += underscore;
    branchName += info.label();
    branchName += underscore;
    branchName += info.instance();
    branchName += underscore;
    branchName += processName;
    branchName += period;
    return branchName;
  }

} // unnamed namespace

namespace gallery {

  DataGetterHelper::DataGetterHelper(
    EventNavigator const* eventNavigator,
    shared_ptr<EventHistoryGetter> historyGetter)
    : eventNavigator_{eventNavigator}, historyGetter_{historyGetter}
  {
    initializeStreamers();
  }

  ProductIDWithProduct
  DataGetterHelper::getByLabel(type_info const& typeInfoOfWrapper,
                               art::InputTag const& inputTag) const
  {
    if (!initializedForProcessHistory_) {
      initializeForProcessHistory();
    }
    art::TypeID const type{typeInfoOfWrapper};
    auto const& info =
      getInfoForTypeLabelInstance(type, inputTag.label(), inputTag.instance());

    if (inputTag.process().empty()) {
      // search in reverse order of the ProcessHistory
      for (auto reverseIter = info.productIDsOrderedByHistory().rbegin(),
                iEnd = info.productIDsOrderedByHistory().rend();
           reverseIter != iEnd;
           ++reverseIter) {
        if (auto edProduct = readProduct(*reverseIter, type)) {
          // If the product was present in the input file and we
          // successfully read it then we are done
          return std::make_pair(*reverseIter, edProduct);
        }
      }
    } else { // process is not empty
      auto itProcess = processNameToProcessIndex_.find(inputTag.process());
      if (itProcess != processNameToProcessIndex_.end()) {
        unsigned int const processIndex = itProcess->second;
        auto branchData =
          getBranchData(info.processIndexToBranchDataIndex(), processIndex);
        if (branchData) {
          auto pd = branchMapReader_.productDescription(info, itProcess->first);
          assert(pd);
          auto product = branchData->uniqueProduct_(type);
          auto productID =
            (product == nullptr) ? art::ProductID::invalid() : pd->productID();
          return std::make_pair(productID, product);
        }
      }
    }
    return std::make_pair(art::ProductID::invalid(), nullptr);
  }

  std::vector<ProductIDWithProduct>
  DataGetterHelper::getManyByType(std::type_info const& typeInfoOfWrapper) const
  {
    std::vector<ProductIDWithProduct> products;
    if (!initializedForProcessHistory_) {
      initializeForProcessHistory();
    }
    art::TypeID const type{typeInfoOfWrapper};
    for (auto const& pr : branchMapReader_.productDescriptions()) {
      auto const& pd = pr.second;
      if (pd.friendlyClassName() != type.friendlyClassName()) {
        continue;
      }

      auto itProcess = processNameToProcessIndex_.find(pd.processName());
      if (itProcess == cend(processNameToProcessIndex_)) {
        continue;
      }

      auto const& info = getInfoForTypeLabelInstance(
        type, pd.moduleLabel(), pd.productInstanceName());

      unsigned int const processIndex = itProcess->second;
      auto branchData =
        getBranchData(info.processIndexToBranchDataIndex(), processIndex);
      if (!branchData) {
        continue;
      }

      if (auto product = branchData->uniqueProduct_(type)) {
        products.emplace_back(pr.first, product);
      }
    }
    return products;
  }

  void
  DataGetterHelper::updateFile(TFile* iFile,
                               TTree* iTree,
                               bool const initializeTheCache)
  {
    tree_ = iTree;
    art::configureProductIDStreamer(); // Null out the ProductID streamer
    branchMapReader_.updateFile(iFile);
    art::configureProductIDStreamer(branchMapReader_.branchIDLists());
    if (initializeTheCache) {
      tree_->SetCacheSize();
      tree_->AddBranchToCache(eventNavigator_->eventAuxiliaryBranch(), kTRUE);
    }
    for (auto& info : infoVector_) {
      vector<uupair> old;
      old.swap(info.processIndexToBranchDataIndex());
      info.processIndexToBranchDataIndex().reserve(processNames_.size());
      for (unsigned int processIndex{}; processIndex < processNames_.size();
           ++processIndex) {
        auto const& processName = processNames_[processIndex];
        auto bd = branchMapReader_.productDescription(info, processName);
        if (bd == nullptr) {
          // Product not available.
          continue;
        }

        auto const result = getBranchDataIndex(old, processIndex);
        if (result.second) {
          auto const branchDataIndex = result.first;
          BranchData& branchData = *branchDataVector_[branchDataIndex];
          auto branch = tree_->GetBranch(branchData.branchName().c_str());
          // This will update the pointer to the TBranch in
          // BranchData.  This loop is sufficient to update all
          // BranchData objects in the vector.
          branchData.updateFile(branch);

          // A little paranoia here. What if in one input file
          // Assns<A,B,C> is written into the file and Assns<B,A,C> is
          // written into another? The following deals properly with
          // that case by constructing a new AssnsBranchData object,
          // although I imagine it might never happen in practice.
          if (info.isAssns() && branch != nullptr) {
            TClass* tClass = getTClass(info, processName);

            art::TypeID const typeIDInDescription{tClass->GetTypeInfo()};
            art::TypeID const typeIDInBranchData{
              branchData.tClass()->GetTypeInfo()};
            if (typeIDInDescription != typeIDInBranchData) {
              std::string branchName = branchData.branchName();
              auto branchData = new AssnsBranchData{typeIDInDescription,
                                                    tClass,
                                                    branch,
                                                    eventNavigator_,
                                                    this,
                                                    move(branchName),
                                                    info.type(),
                                                    info.partnerType()};
              branchDataVector_[branchDataIndex].reset(branchData);
              branchDataMap_[bd->productID()] = branchData;
            }
          }
          info.processIndexToBranchDataIndex().emplace_back(processIndex,
                                                            branchDataIndex);
          if (initializeTheCache && branch) {
            tree_->AddBranchToCache(branch, kTRUE);
          }
        } else if (branchMapReader_.branchInRegistryOfAnyOpenedFile(
                     info.productIDs()[processIndex])) {
          auto pd = branchMapReader_.productDescription(
            info.productIDs()[processIndex]);
          assert(pd != nullptr);
          addBranchData(
            pd->branchName(), processIndex, info, initializeTheCache);
        }
      }
      updateBranchDataIndexOrderedByHistory(info);
    }
    if (initializeTheCache) {
      tree_->StopCacheLearningPhase();
    }
    // This must be cleared because the BranchIDLists may be different
    // in different input files.
    branchDataMissingSet_.clear();
    updateEvent();
  }

  void
  DataGetterHelper::initializeTTreeCache()
  {
    tree_->SetCacheSize();
    tree_->AddBranchToCache(eventNavigator_->eventAuxiliaryBranch(), kTRUE);
    for (auto const& info : infoVector_) {
      for (auto const& i : info.processIndexToBranchDataIndex()) {
        unsigned int const branchDataIndex = i.second;
        auto& branchData = *branchDataVector_[branchDataIndex];
        if (auto branch = branchData.branch()) {
          tree_->AddBranchToCache(branch, kTRUE);
        }
      }
    }
    tree_->StopCacheLearningPhase();
  }

  void
  DataGetterHelper::updateEvent()
  {
    initializedForProcessHistory_ = false;
  }

  void
  DataGetterHelper::initializeForProcessHistory() const
  {
    initializedForProcessHistory_ = true;
    // Do nothing if the process names in the process history are the same
    auto const processHistoryID = historyGetter_->processHistoryID();
    if (processHistoryID == previousProcessHistoryID_) {
      return;
    }
    previousProcessHistoryID_ = processHistoryID;
    art::ProcessHistory const& processHistory =
      historyGetter_->processHistory();
    if (previousProcessHistoryNames_.size() == processHistory.size()) {
      bool same = true;
      auto iPrevious = previousProcessHistoryNames_.begin();
      for (auto i = processHistory.begin(), iEnd = processHistory.end();
           i != iEnd;
           ++i, ++iPrevious) {
        if (i->processName() != *iPrevious) {
          same = false;
          break;
        }
      }
      if (same) {
        return;
      }
    }
    previousProcessHistoryNames_.clear();
    // update for the new process history
    orderedProcessIndexes_.clear();
    for (auto const& processConfig : processHistory) {
      string const& processName = processConfig.processName();
      previousProcessHistoryNames_.push_back(processName);
      auto itFind = processNameToProcessIndex_.find(processName);
      if (itFind == processNameToProcessIndex_.end()) {
        addProcess(processName);
        itFind = processNameToProcessIndex_.find(processName);
      }
      orderedProcessIndexes_.push_back(itFind->second);
    }
    for (auto const& info : infoVector_) {
      updateBranchDataIndexOrderedByHistory(info);
    }
  }

  void
  DataGetterHelper::addProcess(string const& processName) const
  {
    unsigned int const processIndex = processNames_.size();
    processNames_.push_back(processName);
    processNameToProcessIndex_[processName] = processIndex;
    for (auto& info : infoVector_) {
      auto pd = branchMapReader_.productDescription(info, processName);
      if (pd == nullptr) {
        continue;
      }
      assert(branchMapReader_.branchInRegistryOfAnyOpenedFile(pd->productID()));
      info.productIDs().push_back(pd->productID());
      addBranchData(pd->branchName(), processIndex, info);
    }
  }

  void
  DataGetterHelper::addBranchData(string branchName,
                                  unsigned int const processIndex,
                                  InfoForTypeLabelInstance const& info,
                                  bool const initializeTheCache) const
  {
    auto branch = tree_->GetBranch(branchName.c_str());
    if (branch == nullptr) {
      return;
    }
    unsigned int const branchDataIndex = branchDataVector_.size();
    if (info.isAssns()) {
      auto const& processName = processNames_[processIndex];
      auto tClass = getTClass(info, processName);

      art::TypeID const typeIDInDescription{tClass->GetTypeInfo()};
      branchDataVector_.emplace_back(new AssnsBranchData{typeIDInDescription,
                                                         tClass,
                                                         branch,
                                                         eventNavigator_,
                                                         this,
                                                         move(branchName),
                                                         info.type(),
                                                         info.partnerType()});
    } else {
      branchDataVector_.emplace_back(new BranchData{info.type(),
                                                    info.tClass(),
                                                    branch,
                                                    eventNavigator_,
                                                    this,
                                                    move(branchName)});
    }
    auto pd =
      branchMapReader_.productDescription(info, processNames_[processIndex]);
    assert(pd);
    branchDataMap_[pd->productID()] = branchDataVector_.back().get();
    info.processIndexToBranchDataIndex().emplace_back(processIndex,
                                                      branchDataIndex);
    if (initializeTheCache && branch) {
      tree_->AddBranchToCache(branch, kTRUE);
    }
  }

  TClass*
  DataGetterHelper::getTClass(InfoForTypeLabelInstance const& info,
                              std::string const& processName) const
  {
    auto bd = branchMapReader_.productDescription(info, processName);
    if (bd == nullptr) {
      throw art::Exception{art::errors::LogicError,
                           "DataGetterHelper::getTClass: "}
        << "TBranch exists but no BranchDescription in ProductRegistry.\n"
        << "This shouldn't be possible. For type " << info.type().className()
        << "\n";
    }

    art::detail::AssnsStreamer::init_streamer(bd->producedClassName());
    auto tClass = TClass::GetClass(bd->wrappedName().c_str());
    if (tClass == nullptr) {
      throw art::Exception{art::errors::DictionaryNotFound,
                           "DataGetterHelper::getTClass: "}
        << "Missing dictionary for wrapped Assns class.\n"
        << bd->wrappedName() << "\n";
    }
    return tClass;
  }

  InfoForTypeLabelInstance&
  DataGetterHelper::getInfoForTypeLabelInstance(art::TypeID const& type,
                                                string const& label,
                                                string const& instance) const
  {
    if (label.empty()) {
      throw art::Exception(art::errors::LogicError)
        << "getValidHandle was passed an empty module label. Not allowed.\n";
    }
    TypeLabelInstanceKey const key{type, label, instance};
    auto itFind = infoMap_.find(key);
    if (itFind == cend(infoMap_)) {
      addTypeLabelInstance(type, label, instance);
      itFind = infoMap_.find(key);
    }
    return infoVector_[itFind->second];
  }

  void
  DataGetterHelper::addTypeLabelInstance(art::TypeID const& type,
                                         std::string const& label,
                                         std::string const& instance) const
  {
    dictChecker_.checkDictionaries(art::uniform_type_name(type.typeInfo()),
                                   true);
    dictChecker_.reportMissingDictionaries();
    unsigned int infoIndex = infoVector_.size();
    infoVector_.emplace_back(type, label, instance);
    insertIntoInfoMap(type, label, instance, infoIndex);
    InfoForTypeLabelInstance const& info = infoVector_[infoIndex];
    if (info.isAssns()) {
      insertIntoInfoMap(info.partnerType(), label, instance, infoIndex);
    }
    unsigned int processIndex{};
    info.productIDs().reserve(processNames_.size());
    for (auto const& processName : processNames_) {
      string branchName{buildBranchName(info, processName)};
      auto bd[[gnu::unused]] =
        branchMapReader_.productDescription(info, processName);
      // if (bd == nullptr) {
      //   continue;
      //   assert(bd);
      // }
      art::ProductID const productID{branchName};
      info.productIDs().push_back(productID);
      if (branchMapReader_.branchInRegistryOfAnyOpenedFile(productID)) {
        addBranchData(move(branchName), processIndex, info);
      }
      ++processIndex;
    }
    updateBranchDataIndexOrderedByHistory(info);
  }

  void
  DataGetterHelper::insertIntoInfoMap(art::TypeID const& type,
                                      string const& label,
                                      string const& instance,
                                      unsigned int const infoIndex) const
  {
    TypeLabelInstanceKey const newKey{type, label, instance};
    infoMap_[newKey] = infoIndex;
  }

  art::EDProduct const*
  DataGetterHelper::readProduct(art::ProductID const productID,
                                art::TypeID const& type) const
  {
    auto const& branchData = branchDataMap_.at(productID);
    return branchData->uniqueProduct_(type);
  }

  std::pair<unsigned int, bool>
  DataGetterHelper::getBranchDataIndex(
    vector<uupair> const& processIndexToBranchDataIndex,
    unsigned int const processIndex) const
  {
    auto itBranchDataIndex = lower_bound(
      processIndexToBranchDataIndex.cbegin(),
      processIndexToBranchDataIndex.cend(),
      uupair(processIndex, 0),
      [](uupair const& l, uupair const& r) { return l.first < r.first; });
    if (itBranchDataIndex != processIndexToBranchDataIndex.cend() &&
        itBranchDataIndex->first == processIndex) {
      return make_pair(itBranchDataIndex->second, true);
    }
    return make_pair(-1u, false);
  }

  BranchData const*
  DataGetterHelper::getBranchData(
    vector<uupair> const& processIndexToBranchDataIndex,
    unsigned int const processIndex) const
  {
    auto res = getBranchDataIndex(processIndexToBranchDataIndex, processIndex);
    return res.second ? branchDataVector_[res.first].get() : nullptr;
  }

  void
  DataGetterHelper::updateBranchDataIndexOrderedByHistory(
    InfoForTypeLabelInstance const& info) const
  {
    info.productIDsOrderedByHistory().clear();
    if (info.productIDsOrderedByHistory().capacity() <
        orderedProcessIndexes_.size()) {
      info.productIDsOrderedByHistory().reserve(orderedProcessIndexes_.size());
    }
    for (auto processIndex : orderedProcessIndexes_) {
      auto const& orderedProcessName = processNames_[processIndex];
      auto pd = branchMapReader_.productDescription(info, orderedProcessName);
      if (pd == nullptr) {
        continue;
      }
      auto branchData =
        getBranchData(info.processIndexToBranchDataIndex(), processIndex);
      if (branchData && branchData->branch() != nullptr) {
        info.productIDsOrderedByHistory().emplace_back(pd->productID());
      }
    }
  }

  BranchData const*
  DataGetterHelper::getBranchData(art::BranchDescription const& desc) const
  {
    if (!initializedForProcessHistory_) {
      initializeForProcessHistory();
    }
    TClass* tClass = TClass::GetClass(desc.wrappedName().c_str());
    if (tClass == nullptr) {
      throw art::Exception(art::errors::DictionaryNotFound,
                           "DataGetterHelper::getBranchData: ")
        << "Missing dictionary for wrapped class.\n"
        << desc.wrappedName() << "\n";
    }

    type_info const& typeInfoOfWrapper = *tClass->GetTypeInfo();
    art::TypeID const type{typeInfoOfWrapper};
    auto const& info = getInfoForTypeLabelInstance(
      type, desc.moduleLabel(), desc.productInstanceName());
    auto itProcess = processNameToProcessIndex_.find(desc.processName());
    if (itProcess != cend(processNameToProcessIndex_)) {
      unsigned int const processIndex = itProcess->second;
      auto branchData =
        getBranchData(info.processIndexToBranchDataIndex(), processIndex);
      if (branchData && branchData->branch() != nullptr) {
        return branchData;
      }
    }
    return nullptr;
  }

  art::BranchDescription const&
  DataGetterHelper::getProductDescription(art::ProductID const productID) const
  {
    auto pd = branchMapReader_.productDescription(productID);
    if (pd == nullptr) {
      throw art::Exception{art::errors::ProductNotFound,
                           "DataGetterHelper::getProductDescription: "}
        << "No product description could be found for ProductID " << productID
        << ".\n";
    }
    return *pd;
  }

  art::EDProductGetter const*
  DataGetterHelper::getEDProductGetter_(art::ProductID const& productID) const
  {
    auto itFind = branchDataMap_.find(productID);
    if (itFind != cend(branchDataMap_)) {
      return itFind->second;
    }

    if (branchDataMissingSet_.find(productID) != cend(branchDataMissingSet_)) {
      return &invalidBranchData_;
    }

    auto const pd = branchMapReader_.productDescription(productID);
    if (pd == nullptr) {
      return &invalidBranchData_;
    }

    auto branchData = getBranchData(*pd);
    if (branchData == nullptr) {
      branchDataMissingSet_.insert(productID);
      return &invalidBranchData_;
    }
    return branchData;
  }

} // namespace gallery
