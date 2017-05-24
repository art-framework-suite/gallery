#include "gallery/Event.h"

#include "gallery/DataGetterHelper.h"
#include "gallery/EventHistoryGetter.h"
#include "gallery/EventNavigator.h"
#include "gallery/throwFunctions.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"


#include "TFile.h"

namespace gallery {

  Event::Event(std::vector<std::string> const& fileNames,
               bool useTTreeCache,
               unsigned int eventsToLearnUsedBranches) :
    randomAccessOK_(fileNames.size()==1),
    eventNavigator_(std::make_unique<EventNavigator>(fileNames)),
    dataGetterHelper_(std::make_unique<DataGetterHelper>(eventNavigator_.get(),
                                                         std::make_shared<EventHistoryGetter>(eventNavigator_.get()))),
    useTTreeCache_(useTTreeCache),
    eventsToLearnUsedBranches_(eventsToLearnUsedBranches),
    eventsProcessed_(0) {

    if (eventsToLearnUsedBranches_ < 1) eventsToLearnUsedBranches_ = 1;
    if (!atEnd()) {
      bool initializeTheCache = false;
      dataGetterHelper_->updateFile(eventNavigator_->getTFile(), eventNavigator_->getTTree(), initializeTheCache);
    }
  }

  art::EventAuxiliary const& Event::eventAuxiliary() const {
    return eventNavigator_->eventAuxiliary();
  }

  art::History const& Event::history() const {
    return eventNavigator_->history();
  }

  art::ProcessHistoryID const& Event::processHistoryID() const {
    return eventNavigator_->processHistoryID();
  }

  art::ProcessHistory const& Event::processHistory() const {
    return eventNavigator_->processHistory();
  }

  long long Event::numberOfEventsInFile() const {
    return eventNavigator_->entriesInCurrentFile();
  }

  long long Event::eventEntry() const {
    return eventNavigator_->eventEntry();
  }

  long long Event::fileEntry() const {
    return eventNavigator_->fileEntry();
  }

  void Event::goToEntry(long long entry) {
    if (!randomAccessOK_) throwIllegalRandomAccess();
    if (entry < 0) throwIllegalRandomAccess();
    if (entry >= numberOfEventsInFile()) throwIllegalRandomAccess();
    eventNavigator_->goToEntry(entry);
  }

  bool Event::isValid() const {
    return eventNavigator_->isValid();
  }

  bool Event::atEnd() const {
    return eventNavigator_->atEnd();
  }

  void Event::toBegin() {
    long long oldEventEntry = eventEntry();
    long long oldFileEntry = fileEntry();
    eventNavigator_->toBegin();
    if (oldEventEntry == eventEntry() && oldFileEntry == fileEntry()) {
      return;
    }
    updateAfterEventChange(oldFileEntry);
  }

  Event& Event::operator++() {
    auto const oldFileEntry = fileEntry();
    eventNavigator_->next();
    updateAfterEventChange(oldFileEntry);
    return *this;
  }

  Event& Event::operator--() {
    if (!randomAccessOK_) throwIllegalRandomAccess();
    if (atEnd()) throwIllegalDecrement();
    auto const oldFileEntry = fileEntry();
    eventNavigator_->previous();
    updateAfterEventChange(oldFileEntry);
    return *this;
  }

  void Event::updateAfterEventChange(long long oldFileEntry) {
    ++eventsProcessed_;
    if (atEnd()) return;
    if (oldFileEntry != fileEntry()) {
      bool initializeTheCache = useTTreeCache_ && eventsProcessed_ >= eventsToLearnUsedBranches_;
      dataGetterHelper_->updateFile(eventNavigator_->getTFile(), eventNavigator_->getTTree(), initializeTheCache);
    } else {
      dataGetterHelper_->updateEvent();
      if (useTTreeCache_ && eventsProcessed_ == eventsToLearnUsedBranches_) {
        dataGetterHelper_->initializeTTreeCache();
      }
    }
  }

  void Event::next() {
    operator++();
  }

  void Event::previous() {
    operator--();
  }

  TFile* Event::getTFile() const {
    return eventNavigator_->getTFile();
  }

  TTree* Event::getTTree() const {
    return eventNavigator_->getTTree();
  }

  void Event::getByLabel(std::type_info const& typeInfoOfWrapper,
                         art::InputTag const& inputTag,
                         art::EDProduct const*& edProduct) const {

    dataGetterHelper_->getByLabel(typeInfoOfWrapper,
                                  inputTag,
                                  edProduct);
  }

  void Event::throwProductNotFoundException(std::type_info const& typeInfo,
                                            art::InputTag const& tag) const {
    auto e = makeProductNotFoundException(typeInfo, tag);
    throw *e;
  }

  std::shared_ptr<art::Exception const>
  Event::makeProductNotFoundException(std::type_info const& typeInfo,
                                      art::InputTag const& tag) const {
    art::TypeID type(typeInfo);
    std::shared_ptr<art::Exception> e = std::make_shared<art::Exception>(art::errors::ProductNotFound);
    *e << "Failed to find product for \n  type = '"
       << type.className()
       << "'\n  module = '" << tag.label()
       << "'\n  productInstance = '"
       << ((!tag.instance().empty()) ? tag.instance().c_str() : "")
       << "'\n  process='"
       << ((!tag.process().empty()) ? tag.process().c_str() : "")
       << "'\n";
    return e;
  }

  void Event::checkForEnd() const {
    if (atEnd()) {
      throw art::Exception(art::errors::LogicError)
        << "You have requested data past the last event\n";
    }
  }
}
