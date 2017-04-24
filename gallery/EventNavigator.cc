#include "gallery/EventNavigator.h"

#include "gallery/throwFunctions.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"

#include "TBranch.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>

namespace gallery {

  EventNavigator::EventNavigator(std::vector<std::string> const& iFileNames) :
    fileNames_(iFileNames),
    numberOfFiles_(fileNames_.size()),
    fileEntry_(-1),
    firstFileWithEvent_(0),
    entriesInCurrentFile_(0),
    eventEntry_(0),
    file_(),
    eventsTree_(nullptr),
    eventAuxiliaryBranch_(nullptr),
    eventAuxiliary_(),
    pEventAuxiliary_(&eventAuxiliary_),
    previousEventAuxiliaryEntry_(-1),
    eventHistoryTree_(nullptr),
    eventHistoryBranch_(nullptr),
    eventHistory_(),
    pEventHistory_(&eventHistory_),
    previousEventHistoryEntry_(-1),
    historyMap_() {

    if (fileNames_.empty()) {
      fileEntry_ = 0;
    } else {
      while (eventEntry_ == entriesInCurrentFile_ && fileEntry_ != numberOfFiles_) {
        nextFile();
      }
      firstFileWithEvent_ = fileEntry_;
    }
    if (isValid()) {
      eventsTree_->LoadTree(eventEntry_);
    }
  }

  void EventNavigator::toBegin() {
    if (fileEntry_ == firstFileWithEvent_) {
      eventEntry_ = 0;
      return;
    }
    fileEntry_ = firstFileWithEvent_ - 1;
    nextFile();
    if (isValid()) {
      eventsTree_->LoadTree(eventEntry_);
    }
  }

  void EventNavigator::next() {
    if (eventEntry_ != entriesInCurrentFile_) {
      // normal case, go to next event in same file
      ++eventEntry_;
    } else {
      // handle odd cases, calling next when already at the end
      // or after a call to nextFile() left us on a file with
      // no events. (Neither of these cases should occur in a 
      // normal loop over events)
      nextFile();
    }
    // if we hit the end of the file go to the next file
    // also skip empty files
    while (eventEntry_ == entriesInCurrentFile_ && fileEntry_ != numberOfFiles_) {
      nextFile();
    }
    if (isValid()) {
      eventsTree_->LoadTree(eventEntry_);
    }
  }

  void EventNavigator::previous() {
    // We will never get here is we're dealing with more than one file.
    --eventEntry_;
    if (isValid()) {
      eventsTree_->LoadTree(eventEntry_);
    }
  }

  void EventNavigator::goToEntry(long long entry) {
    eventEntry_ = entry;
    if (isValid()) {
      eventsTree_->LoadTree(eventEntry_);
    }
  }

  void EventNavigator::nextFile() {

    // Be careful with this function. If the next file
    // is empty this will leave it not pointing at a
    // valid event.

    if (atEnd()) {
      throw art::Exception(art::errors::LogicError) <<
	"Illegal call to EventNavigator::nextFile() when atEnd() is true";
    }

    if (file_) {
      std::cout << "Closing file, read "
		<< file_->GetBytesRead()
		<< " bytes in "
		<< file_->GetReadCalls()
		<<  " transactions\n";
    }

    ++fileEntry_;
    if (atEnd()) {
      entriesInCurrentFile_ = 0;
      eventEntry_ = 0;
      file_ = nullptr;
      eventsTree_ = nullptr;
      eventAuxiliaryBranch_ = nullptr;
      eventHistoryTree_ = nullptr;
      eventHistoryBranch_ = nullptr;
      previousEventAuxiliaryEntry_ = -1;
      previousEventHistoryEntry_ = -1;
      historyMap_.clear();
      return;
    }
    file_.reset(TFile::Open(fileNames_[fileEntry_].c_str()));
    if (!file_ || file_->IsZombie()) {
      throw art::Exception(art::errors::FileOpenError)
        << "Failed opening file \'" << fileNames_[fileEntry_] << "\'";
    }
    std::cout << "Successfully opened file " << fileNames_[fileEntry_] << std::endl;
    initializeTTreePointers();
    initializeTBranchPointers();
    entriesInCurrentFile_ = eventsTree_->GetEntries();
    eventEntry_ = 0;
    previousEventAuxiliaryEntry_ = -1;
    previousEventHistoryEntry_ = -1;
    historyMap_.clear();
    return;
  }

  art::EventAuxiliary const& EventNavigator::eventAuxiliary() const {
    if (previousEventAuxiliaryEntry_ != eventEntry_) {
      eventAuxiliaryBranch_->GetEntry(eventEntry_);
      previousEventAuxiliaryEntry_ = eventEntry_;
    }
    return eventAuxiliary_;
  }

  art::History const& EventNavigator::history() const {
    if(previousEventHistoryEntry_ != eventEntry_) {
      eventHistoryBranch_->GetEntry(eventEntry_);
      previousEventHistoryEntry_ = eventEntry_;
    }
    return eventHistory_;
  }

  art::ProcessHistoryID const& EventNavigator::processHistoryID() const {
    if(previousEventHistoryEntry_ != eventEntry_) {
      eventHistoryBranch_->GetEntry(eventEntry_);
      previousEventHistoryEntry_ = eventEntry_;
    }
    return eventHistory_.processHistoryID();
  }

  art::ProcessHistory const& EventNavigator::processHistory() const {

    if (historyMap_.empty()) {

      auto metaDataTree =
        dynamic_cast<TTree*>(file_->Get(art::rootNames::metaDataTreeName().c_str()));

      if (!metaDataTree) {
        throwTreeNotFound(art::rootNames::metaDataTreeName());
      }

      auto pHistMapPtr = &historyMap_;
      TBranch* processHistoryBranch =
        metaDataTree->GetBranch(art::rootNames::metaBranchRootName<art::ProcessHistoryMap>());

      if (!processHistoryBranch) {
        throwBranchNotFound(art::rootNames::metaBranchRootName<art::ProcessHistoryMap>());
      }
      processHistoryBranch->SetAddress(&pHistMapPtr);
      processHistoryBranch->GetEntry(0);
    }
    return historyMap_[processHistoryID()];
  }

  TFile* EventNavigator::getTFile() const {
    return file_.get();
  }

  TTree* EventNavigator::getTTree() const {
    return eventsTree_;
  }

  void EventNavigator::initializeTTreePointers() {
    eventsTree_ = dynamic_cast<TTree*>(file_->Get(art::rootNames::eventTreeName().c_str()));
    if (eventsTree_ == nullptr) {
      throwTreeNotFound(art::rootNames::eventTreeName());
    }
    if (eventsTree_->GetEntries() < 0) {
      throw art::Exception(art::errors::LogicError)
        << "Unable to get the number of entries in events TTree.\n"
           "This might be a corrupted file.\n";
    }
    eventHistoryTree_ = dynamic_cast<TTree*>(file_->Get(art::rootNames::eventHistoryTreeName().c_str()));
    if (eventHistoryTree_ == nullptr) {
      throwTreeNotFound(art::rootNames::eventHistoryTreeName());
    }
  }

  void EventNavigator::initializeTBranchPointers() {
    eventAuxiliaryBranch_ =
      eventsTree_->GetBranch(art::BranchTypeToAuxiliaryBranchName(art::InEvent).c_str());

    if (eventAuxiliaryBranch_ == nullptr) {
      throwBranchNotFound(art::BranchTypeToAuxiliaryBranchName(art::InEvent));
    }
    eventAuxiliaryBranch_->SetAddress(&pEventAuxiliary_);

    eventHistoryBranch_ =
      eventHistoryTree_->GetBranch(art::rootNames::eventHistoryBranchName().c_str());

    if(eventHistoryBranch_ == nullptr) {
      throwBranchNotFound(art::rootNames::eventHistoryBranchName());
    }
    eventHistoryBranch_->SetAddress(&pEventHistory_);
  }
}
