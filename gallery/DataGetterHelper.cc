
#include "gallery/DataGetterHelper.h"

#include "gallery/EventNavigator.h"

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchListIndex.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/Exception.h"

#include "canvas/Persistency/Common/CacheStreamers.h"
#include "canvas/Persistency/Provenance/TransientStreamer.h"
#include "canvas/Persistency/Common/detail/setPtrVectorBaseStreamer.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"

#include "TClass.h"
#include "TTree.h"

#include <algorithm>
#include <cstring>

namespace {
  std::string const underscore("_");
  std::string const period(".");
  std::string const emptyString;
}

namespace canvas {

  bool DataGetterHelper::streamersInitialized_ = false;

  DataGetterHelper::DataGetterHelper(EventNavigator const* eventNavigator,
                                     std::shared_ptr<HistoryGetterBase> historyGetter) :
    eventNavigator_(eventNavigator),
    tree_(nullptr),
    edProductTClass_(TClass::GetClass("art::EDProduct")),
    historyGetter_(historyGetter),
    initializedForProcessHistory_(false) {

    initializeStreamers();
  }

  DataGetterHelper::~DataGetterHelper() {
    for (auto label : labels_) {
      delete [] label;
    }
  }

  void DataGetterHelper::getByLabel(std::type_info const& typeInfoOfWrapper,
                                    InputTag const& inputTag,
                                    void* ptrToPtrToWrapper) const {

    void** ptrToPtr = reinterpret_cast<void**>(ptrToPtrToWrapper);
    *ptrToPtr = nullptr; // this nullptr indicates product not found yet

    if (!initializedForProcessHistory_) initializeForProcessHistory();

    InfoForTypeLabelInstance const& info = getInfoForTypeLabelInstance(typeInfoOfWrapper,
                                                                       inputTag.label(),
                                                                       inputTag.instance());
    if (inputTag.process().empty()) {
      // search in reverse order of the ProcessHistory
      for (auto reverseIter = info.branchDataIndexOrderedByHistory().rbegin(),
                       iEnd = info.branchDataIndexOrderedByHistory().rend();
           reverseIter != iEnd; ++reverseIter) {
        readBranchData(*reverseIter, ptrToPtr);
        // If the product was present in the input file and we successfully read it then we are done
        if (*ptrToPtr) return;
      }
    } else {  // process is not empty

      auto itProcess = processNameToProcessIndex_.find(inputTag.process());
      if (itProcess != processNameToProcessIndex_.end()) {
        unsigned int processIndex = itProcess->second;

        unsigned int branchDataIndex = 0;
        if (getBranchDataIndex(info.processIndexToBranchDataIndex(), processIndex, branchDataIndex)) {
          readBranchData(branchDataIndex, ptrToPtr);
        }
      }
    }
  }

  void DataGetterHelper::updateFile(TFile* iFile, TTree* iTree, bool initializeTheCache) {
    tree_ = iTree;
    branchMapReader_.updateFile(iFile);

    if (initializeTheCache) {
      tree_->SetCacheSize();
      tree_->AddBranchToCache(eventNavigator_->eventAuxiliaryBranch(), kTRUE);
    }

    for (auto& info : infoVector_) {
      std::vector<std::pair<unsigned int, unsigned int> > old;
      old.swap(info.processIndexToBranchDataIndex());
      info.processIndexToBranchDataIndex().reserve(processNames_.size());
      for (unsigned int processIndex = 0; processIndex < processNames_.size(); ++processIndex) {
        unsigned int branchDataIndex = 0;
        if (getBranchDataIndex(old, processIndex, branchDataIndex)) {
          BranchData& branchData = *branchDataVector_[branchDataIndex];
          TBranch* branch = tree_->GetBranch(branchData.branchName().c_str());
          // This will update the pointer to the TBranch in BranchData.
          // This loop is sufficient to update all BranchData objects in the vector.
          branchData.updateFile(branch);
          info.processIndexToBranchDataIndex().push_back(uupair(processIndex, branchDataIndex));
          if (initializeTheCache && branch) {
            tree_->AddBranchToCache(branch, kTRUE);
          }
        } else if (branchMapReader_.branchInRegistryOfAnyOpenedFile(info.branchIDs()[processIndex])) {
          std::string branchName(buildBranchName(info, processNames_[processIndex]));
          addBranchData(std::move(branchName), processIndex, info, initializeTheCache);
        }
      }
      updateBranchDataIndexOrderedByHistory(info);
    }

    if (initializeTheCache) {
      tree_->StopCacheLearningPhase();
    }

    // These must be cleared because the BranchIDLists may be different in
    // different input files.
    branchDataIndexMap_.clear();
    branchDataMissingSet_.clear();

    updateEvent();
  }

  void DataGetterHelper::initializeTTreeCache() {
    tree_->SetCacheSize();
    tree_->AddBranchToCache(eventNavigator_->eventAuxiliaryBranch(), kTRUE);

    for (auto& info : infoVector_) {
      for (auto const& i : info.processIndexToBranchDataIndex()) {
        unsigned int branchDataIndex = i.second;
        BranchData& branchData = *branchDataVector_[branchDataIndex];
        TBranch* branch = branchData.branch();
        if (branch) {
          tree_->AddBranchToCache(branch,kTRUE);
        }
      }
    }
    tree_->StopCacheLearningPhase();
  }

  void DataGetterHelper::updateEvent() {
    branchMapReader_.updateEvent(historyGetter_.get());
    initializedForProcessHistory_ = false;
  }

  void DataGetterHelper::initializeStreamers() {
    if (streamersInitialized_) return;
    streamersInitialized_ = true;

    art::setCacheStreamers();
    art::setProvenanceTransientStreamers();
    art::detail::setBranchDescriptionStreamer();
    art::detail::setPtrVectorBaseStreamer();
    art::configureRefCoreStreamer();
  }

  void DataGetterHelper::initializeForProcessHistory() const {
    initializedForProcessHistory_ = true;

    // Do nothing if the process names in the process history are the same
    art::ProcessHistoryID processHistoryID = historyGetter_->processHistoryID();
    if (processHistoryID == previousProcessHistoryID_) return;
    previousProcessHistoryID_ = processHistoryID;

    art::ProcessHistory const& processHistory = historyGetter_->processHistory();
    if (previousProcessHistoryNames_.size() == processHistory.size()) {
      bool same = true;
      auto iPrevious = previousProcessHistoryNames_.begin();
      for (auto i = processHistory.begin(), iEnd = processHistory.end();
           i != iEnd; ++i, ++iPrevious) {
        if (i->processName() != *iPrevious) {
          same = false;
          break;
        }
      }
      if (same) return;
    }
    previousProcessHistoryNames_.clear();

    // update for the new process history
    orderedProcessIndexes_.clear();
    for (auto i = processHistory.begin(), iEnd = processHistory.end(); i != iEnd; ++i) {
      std::string const& processName = i->processName();
      previousProcessHistoryNames_.push_back(processName);
      auto itFind = processNameToProcessIndex_.find(processName);
      if (itFind == processNameToProcessIndex_.end()) {
        addProcess(processName);
        itFind = processNameToProcessIndex_.find(processName);
      }
      orderedProcessIndexes_.push_back(itFind->second);
    }
    for (auto& info : infoVector_) {
      updateBranchDataIndexOrderedByHistory(info);
    }
  }

  void DataGetterHelper::addProcess(std::string const& processName) const {
    unsigned int processIndex = processNames_.size();
    processNames_.push_back(processName);
    processNameToProcessIndex_[processName] = processIndex;
    for (auto& info : infoVector_) {
      std::string branchName(buildBranchName(info, processName));
      art::BranchID branchID(branchName);
      info.branchIDs().push_back(branchID);
      if (branchMapReader_.branchInRegistryOfAnyOpenedFile(branchID)) {
        addBranchData(std::move(branchName), processIndex, info);
      }
    }
  }

  std::string DataGetterHelper::buildBranchName(InfoForTypeLabelInstance const& info,
                                                std::string const& processName) {
    std::string branchName(info.type().friendlyClassName());
    unsigned int branchNameSize = branchName.size() +
                                  info.label().size() +
                                  info.instance().size() +
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

  void DataGetterHelper::addBranchData(std::string&& branchName,
                                       unsigned int processIndex,
                                       InfoForTypeLabelInstance const& info,
                                       bool initializeTheCache) const {
    TBranch* branch = tree_->GetBranch(branchName.c_str());

    unsigned int branchDataIndex = branchDataVector_.size();
    branchDataVector_.emplace_back(new BranchData(info.type(), branch, edProductTClass_,
                                                  eventNavigator_, this, std::move(branchName)));
    info.processIndexToBranchDataIndex().push_back(uupair(processIndex, branchDataIndex));
    if (initializeTheCache && branch) {
      tree_->AddBranchToCache(branch,kTRUE);
    }
  }

  DataGetterHelper::InfoForTypeLabelInstance&
  DataGetterHelper::getInfoForTypeLabelInstance(std::type_info const& typeInfoOfWrapper,
                                                std::string const& label,
                                                std::string const& instance) const {
    if (label.empty()) {
      throw art::Exception(art::errors::LogicError)
        << "getValidHandle was passed an empty module label. Not allowed.\n";
    }

    art::TypeID type(typeInfoOfWrapper);
    TypeLabelInstanceKey key(type, label.c_str(), instance.c_str());

    auto itFind = infoMap_.find(key);
    if (itFind == infoMap_.end()) {
      addTypeLabelInstance(type, label, instance);
      itFind = infoMap_.find(key);
    }
    return infoVector_[itFind->second];
  }

  void DataGetterHelper::addTypeLabelInstance(art::TypeID const& type,
                                              std::string const& label,
                                              std::string const& instance) const {
    unsigned int infoIndex = infoVector_.size();
    infoVector_.emplace_back(type, label, instance);
    InfoForTypeLabelInstance const& info = infoVector_[infoIndex];

    unsigned int processIndex = 0;
    info.branchIDs().reserve(processNames_.size());
    for(auto const& processName : processNames_) {
      std::string branchName(buildBranchName(info, processName));
      art::BranchID branchID(branchName);
      info.branchIDs().push_back(branchID);
      if (branchMapReader_.branchInRegistryOfAnyOpenedFile(branchID)) {
        addBranchData(std::move(branchName), processIndex, info);
      }
      ++processIndex;
    }
    updateBranchDataIndexOrderedByHistory(info);

    insertIntoInfoMap(type, label, instance, infoIndex);
  }

  void DataGetterHelper::insertIntoInfoMap(art::TypeID const& type,
                                           std::string const& label,
                                           std::string const& instance,
                                           unsigned int infoIndex) const {
    char const* pLabel = label.c_str();
    char const* pInstance = instance.c_str();

    size_t labelLen = strlen(pLabel) + 1;
    char* newLabel = new char[labelLen];
    std::strncpy(newLabel, pLabel, labelLen);
    // The only purpose of the labels_ vector is to save
    // the pointer so we can delete the allocated memory
    labels_.push_back(newLabel);

    char const* newInstance = emptyString.c_str();
    size_t newInstanceLen = strlen(pInstance) + 1;
    if (newInstanceLen > 1) {
      char* temp = new char[newInstanceLen];
      std::strncpy(temp, pInstance, newInstanceLen);
      labels_.push_back(temp);
      newInstance = temp;
    }

    TypeLabelInstanceKey newKey(type, newLabel, newInstance);

    infoMap_[newKey] = infoIndex;
  }

  void DataGetterHelper::readBranchData(unsigned int branchDataIndex,
                                        void **ptrToPtr) const {

    BranchData const* branchData =  branchDataVector_[branchDataIndex].get();
    if (branchData->getIt()) {
      *ptrToPtr = branchData->address();
    }
  }

  bool DataGetterHelper::getBranchDataIndex(std::vector<std::pair<unsigned int, unsigned int> > const& processIndexToBranchDataIndex,
                                            unsigned int processIndex,
                                            unsigned int & branchDataIndex) const {
    auto itBranchDataIndex = std::lower_bound(processIndexToBranchDataIndex.cbegin(),
                                              processIndexToBranchDataIndex.cend(),
                                              uupair(processIndex, 0),
                                              [](uupair const& l, uupair const& r) { return l.first < r.first; });
    if (itBranchDataIndex != processIndexToBranchDataIndex.cend() &&
        itBranchDataIndex->first == processIndex) {
      branchDataIndex = itBranchDataIndex->second;
      return true;
    }
    return false;
  }

  void DataGetterHelper::updateBranchDataIndexOrderedByHistory(InfoForTypeLabelInstance const& info) const {
    info.branchDataIndexOrderedByHistory().clear();
    if (info.branchDataIndexOrderedByHistory().capacity() < orderedProcessIndexes_.size()) {
      info.branchDataIndexOrderedByHistory().reserve(orderedProcessIndexes_.size());
    }
    for (auto processIndex : orderedProcessIndexes_) {
      unsigned int branchDataIndex = 0;
      if (getBranchDataIndex(info.processIndexToBranchDataIndex(), processIndex, branchDataIndex) &&
          branchDataVector_[branchDataIndex]->branch() != nullptr) {
        info.branchDataIndexOrderedByHistory().push_back(branchDataIndex);
      }
    }
  }

  bool
  DataGetterHelper::getByBranchDescription(art::BranchDescription const& desc,
                                           unsigned int & branchDataIndex) const {

    if (!initializedForProcessHistory_) initializeForProcessHistory();

    TClass* tClass = TClass::GetClass(desc.wrappedName().c_str());
    std::type_info const& typeInfoOfWrapper = *tClass->GetTypeInfo();

    InfoForTypeLabelInstance const& info = getInfoForTypeLabelInstance(typeInfoOfWrapper,
                                                                       desc.moduleLabel(),
                                                                       desc.productInstanceName());
    auto itProcess = processNameToProcessIndex_.find(desc.processName());
    if (itProcess != processNameToProcessIndex_.end()) {
      unsigned int processIndex = itProcess->second;
      if (getBranchDataIndex(info.processIndexToBranchDataIndex(), processIndex, branchDataIndex)) {
        return branchDataVector_[branchDataIndex]->branch() != nullptr;
      }
    }
    return false;
  }

  art::EDProductGetter const* DataGetterHelper::getEDProductGetterImpl(art::ProductID const& productID) const {
    art::ProcessIndex processIndex = productID.processIndex();
    art::BranchListIndexes const& branchListIndexes = historyGetter_->history().branchListIndexes();
    if (processIndex == 0 || processIndex > branchListIndexes.size()) {
      throw art::Exception(art::errors::LogicError)
        << "DataGetterHelper::getEDProductGetterImpl out of range processIndex in ProductID.\n";
    }

    std::pair<unsigned short, unsigned short> key(productID.productIndex(),
                                                  branchListIndexes[processIndex - 1]);
    unsigned int branchDataIndex = 0;
    auto itFind =  branchDataIndexMap_.find(key);
    if (itFind == branchDataIndexMap_.end()) {
      if (branchDataMissingSet_.find(key) != branchDataMissingSet_.end()) {
        return &invalidBranchData_;
      }
      art::BranchDescription const* branchDescription = branchMapReader_.productToBranch(productID);
      if (branchDescription) {
        if (!getByBranchDescription(*branchDescription, branchDataIndex)) {
          branchDataMissingSet_.insert(key);
          return &invalidBranchData_;
        }
      } else {
        return &invalidBranchData_;
      }
      branchDataIndexMap_.emplace(key, branchDataIndex);
    } else {
      branchDataIndex = itFind->second;
    }
    return  branchDataVector_[branchDataIndex].get();
  }

  DataGetterHelper::InfoForTypeLabelInstance::
  InfoForTypeLabelInstance(art::TypeID const& iType,
                           std::string const& iLabel,
                           std::string const& iInstance) :
    type_(iType),
    label_(iLabel),
    instance_(iInstance) {
  }
}
