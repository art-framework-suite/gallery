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

  // The gallery::Event provides read-only access to event data
  // products. It also provides the means to iterate through one or
  // more art/ROOT files, reading the events in those files; in this
  // sense, the gallery::Event is a kind of iterator. Only a valid
  // Event (as defined by Event::isValid()) can be used to access
  // event data products.
  class Event {
  public:

    // gallery::Event objects can be neither copied nor assigned. They
    // can be move-copied and move-assigned.

    // Construct an Event that will read events from each of the
    // named art/ROOT files, in order.
    explicit
    Event(std::vector<std::string> const& fileNames,
          bool useTTreeCache = true,
          unsigned int eventsToLearnUsedBranches = 7);

    // Get access to a data product of type PROD, using a ValidHandle.
    template <typename PROD>
    gallery::ValidHandle<PROD>
    getValidHandle(art::InputTag const&) const;

    // Get access to a data product of type PROD, using a Handle.
    // Only if the return value is 'true' is the Handle valid.
    template <typename PROD>
    bool getByLabel(art::InputTag const&, Handle<PROD>& result) const;

    art::EventAuxiliary const& eventAuxiliary() const;
    art::History const& history() const;
    art::ProcessHistoryID const& processHistoryID() const;
    art::ProcessHistory const& processHistory() const;

    // Return the number of events in the currently-open file.
    long long numberOfEventsInFile() const;

    // Return the current entry number (the entry number of the Events
    // tree in the current art/ROOT file).
    long long eventEntry() const;

    // Return the index of the current file.
    long long fileEntry() const;

    // Go to the entry with the given index. If the index is
    // out-of-bounds, or negative, or if the Event is not suitable for
    // random access, an exception will be thrown.
    void goToEntry(long long entry);

    // Return true if the Event can be used to access data products.
    bool isValid() const;

    // Return true if we are at the end of the sequence of events
    // through which we are iterating.
    bool atEnd() const;

    // Go to the first event of the sequence we are to traverse.
    void toBegin();
    void first();

    // Go to the next event in the sequence.
    Event& operator++();
    void next();

    // Throws an exception if the Event was constructed with more than
    // one filename, or if we are already at the beginning of the
    // sequence. Otherwise, go to the previous event in the sequence.
    Event& operator--();
    void previous();

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

    bool randomAccessOK_;
    std::unique_ptr<EventNavigator> eventNavigator_;
    std::unique_ptr<DataGetterHelper> dataGetterHelper_;

    bool useTTreeCache_;
    unsigned int eventsToLearnUsedBranches_;
    unsigned int eventsProcessed_;
  };
} // namespace gallery

template <typename PROD>
inline
gallery::ValidHandle<PROD>
gallery::Event::getValidHandle(art::InputTag const& inputTag) const {
  checkForEnd();
  std::type_info const& typeInfoOfWrapper = typeid(art::Wrapper<PROD>);

  art::EDProduct const* edProduct = nullptr;

  getByLabel(typeInfoOfWrapper,
	     inputTag,
	     edProduct);

  art::Wrapper<PROD> const* ptrToWrapper = dynamic_cast< art::Wrapper<PROD> const *>(edProduct);

  if (ptrToWrapper == nullptr) {
    throwProductNotFoundException(typeid(PROD), inputTag);
  }
  PROD const* product = ptrToWrapper->product();
  return ValidHandle<PROD>(product);
}

template <typename PROD>
inline
bool
gallery::Event::getByLabel(art::InputTag const& inputTag, Handle<PROD>& result) const {
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

inline
void
gallery::Event::first() {
  return toBegin();
}

#endif /* gallery_Event_h */

// Local Variables:
// mode: c++
// End:
