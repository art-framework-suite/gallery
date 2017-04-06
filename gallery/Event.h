#ifndef gallery_Event_h
#define gallery_Event_h

// Main interface to users. It uses the DataGetterHelper
// and EventNavigator to iterate over events in a set
// of input files and find products in them.

#include "gallery/Handle.h"
#include "gallery/ValidHandle.h"
#include "gallery/DataGetterHelper.h"
#include "gallery/EventNavigator.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"

#include "TFile.h"
#include "TTree.h"

#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace cet {
  class exception;
}

namespace gallery {

  class DataGetterHelper;
  class EventNavigator;

  class Event {
  public:

    // Can not copy or assign.
    Event(Event const&) = delete;
    Event& operator=(Event const&) = delete;

    // Can move and move assign. We need to state this, because we
    // have deleted the copy and assignment.
    Event(Event &&) = default;
    Event& operator=(Event &&) = default;

    explicit
    Event(std::vector<std::string> const& fileNames,
          bool useTTreeCache = true,
          unsigned int eventsToLearnUsedBranches = 7);

    template <typename PROD>
    gallery::ValidHandle<PROD>
    getValidHandle(art::InputTag const&) const;

    template <typename PROD>
    bool getByLabel(art::InputTag const&, Handle<PROD>& result) const;

    art::EventAuxiliary const& eventAuxiliary() const;
    art::History const& history() const;
    art::ProcessHistoryID const& processHistoryID() const;
    art::ProcessHistory const& processHistory() const;

    long long numberOfEventsInFile() const;
    long long eventEntry() const;
    long long fileEntry() const;

    bool isValid() const;
    bool atEnd() const;

    void toBegin();
    Event& operator++();
    void next();

    TFile* getTFile() const;
    TTree* getTTree() const;

    template <typename T>
    using HandleT = Handle<T>;

  private:

    void getByLabel(std::type_info const& typeInfoOfWrapper,
                    art::InputTag const& inputTag,
                    art::EDProduct const*& edProduct) const;

    void throwProductNotFoundException(std::type_info const& typeInfo,
                                       art::InputTag const& tag) const;

    std::shared_ptr<art::Exception const>
    makeProductNotFoundException(std::type_info const& typeInfo,
                                 art::InputTag const& tag) const;

    void checkForEnd() const;
    void updateAfterEventChange(long long oldFileEntry);

    std::unique_ptr<EventNavigator> eventNavigator_;
    std::unique_ptr<DataGetterHelper> dataGetterHelper_;

    bool useTTreeCache_;
    unsigned int eventsToLearnUsedBranches_;
    unsigned int eventsProcessed_;
  };

  template <typename PROD>
  ValidHandle<PROD>
  Event::getValidHandle(art::InputTag const& inputTag) const {
    checkForEnd();
    std::type_info const& typeInfoOfWrapper = typeid(art::Wrapper<PROD>);

    art::EDProduct const* edProduct = nullptr;

    getByLabel(typeInfoOfWrapper,
               inputTag,
               edProduct);

    art::Wrapper<PROD> const* ptrToWrapper = dynamic_cast< art::Wrapper<PROD> const *>(edProduct);

    if(ptrToWrapper == nullptr) {
      throwProductNotFoundException(typeid(PROD), inputTag);
    }
    PROD const* product = ptrToWrapper->product();
    return ValidHandle<PROD>(product);
  }

  template <typename PROD>
  bool
  Event::getByLabel(art::InputTag const& inputTag, Handle<PROD>& result) const {
    checkForEnd();
    std::type_info const& typeInfoOfWrapper = typeid(art::Wrapper<PROD>);

    art::EDProduct const* edProduct = nullptr;

    getByLabel(typeInfoOfWrapper,
               inputTag,
               edProduct);

    art::Wrapper<PROD> const* ptrToWrapper = dynamic_cast< art::Wrapper<PROD> const *>(edProduct);

    if(ptrToWrapper == nullptr) {
      result = Handle<PROD>(makeProductNotFoundException(typeid(PROD), inputTag));
      return false;
    }
    PROD const* product = ptrToWrapper->product();
    result = Handle<PROD>(product);
    return true;
  }
}
#endif /* gallery_Event_h */

// Local Variables:
// mode: c++
// End:
