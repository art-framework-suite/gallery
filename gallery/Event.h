#ifndef gallery_Event_h
#define gallery_Event_h

// ====================================================================
// Main interface to users. It uses the DataGetterHelper and
// EventNavigator to iterate over events in a set of input files and
// find products in them.
// ====================================================================

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/container_algorithms.h"
#include "gallery/DataGetterHelper.h"
#include "gallery/EventNavigator.h"
#include "gallery/Handle.h"
#include "gallery/ValidHandle.h"

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
    explicit Event(std::vector<std::string> const& fileNames,
                   bool useTTreeCache = true,
                   unsigned int eventsToLearnUsedBranches = 7);

    // Get access to a data product of type PROD, using a ValidHandle.
    template <typename PROD>
    gallery::ValidHandle<PROD> getValidHandle(art::InputTag const&) const;

    // Get access to a data product of type PROD, using a Handle.
    // Only if the return value is 'true' is the Handle valid.
    template <typename PROD>
    bool getByLabel(art::InputTag const&, Handle<PROD>& result) const;

    // Get access to all data products of type PROD, using a
    // std::vector<Handle>.
    template <typename PROD>
    void getManyByType(std::vector<Handle<PROD>>& result) const;

    // Return all input tags corresponding to products of type PROD.
    // This function call *does not* read or provide access to any
    // products.
    template <typename PROD>
    std::vector<art::InputTag> getInputTags() const;

    template <typename PROD>
    std::vector<art::ProductToken<PROD>> getProductTokens() const;

    art::EventAuxiliary const& eventAuxiliary() const;
    art::History const& history() const;
    art::ProcessHistoryID const& processHistoryID() const;
    art::ProcessHistory const& processHistory() const;

    // Return the product description if it is present.
    art::BranchDescription const& getProductDescription(art::ProductID) const;

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
    ProductWithID getByLabel(std::type_info const& typeInfoOfWrapper,
                             art::InputTag const& inputTag) const;

    std::vector<ProductWithID> getManyByType(
      std::type_info const& typeInfoOfWrapper) const;

    [[noreturn]] void throwProductNotFoundException(
      std::type_info const& typeInfo,
      art::InputTag const& tag) const;

    std::shared_ptr<art::Exception const> makeProductNotFoundException(
      std::type_info const& typeInfo,
      art::InputTag const& tag) const;

    void checkForEnd() const;
    void updateAfterEventChange(long long oldFileEntry);

    bool randomAccessOK_;
    std::unique_ptr<EventNavigator> eventNavigator_;
    std::unique_ptr<DataGetterHelper> dataGetterHelper_;

    bool useTTreeCache_;
    unsigned int eventsToLearnUsedBranches_;
    unsigned int eventsProcessed_{};
  };
} // namespace gallery

template <typename PROD>
inline gallery::ValidHandle<PROD>
gallery::Event::getValidHandle(art::InputTag const& inputTag) const
{
  checkForEnd();
  std::type_info const& typeInfoOfWrapper{typeid(art::Wrapper<PROD>)};
  auto res = getByLabel(typeInfoOfWrapper, inputTag);
  auto edProduct = res.first;

  auto ptrToWrapper = dynamic_cast<art::Wrapper<PROD> const*>(edProduct);
  if (ptrToWrapper == nullptr) {
    throwProductNotFoundException(typeid(PROD), inputTag);
  }

  auto product = ptrToWrapper->product();
  return ValidHandle<PROD>{product, res.second};
}

template <typename PROD>
inline bool
gallery::Event::getByLabel(art::InputTag const& inputTag,
                           Handle<PROD>& result) const
{
  checkForEnd();
  std::type_info const& typeInfoOfWrapper{typeid(art::Wrapper<PROD>)};
  auto res = getByLabel(typeInfoOfWrapper, inputTag);
  auto edProduct = res.first;

  auto ptrToWrapper = dynamic_cast<art::Wrapper<PROD> const*>(edProduct);

  if (ptrToWrapper == nullptr) {
    result = Handle<PROD>{makeProductNotFoundException(typeid(PROD), inputTag)};
    return false;
  }
  auto product = ptrToWrapper->product();
  result = Handle<PROD>{product, res.second};
  return true;
}

template <typename PROD>
inline void
gallery::Event::getManyByType(std::vector<Handle<PROD>>& result) const
{
  std::type_info const& typeInfoOfWrapper{typeid(art::Wrapper<PROD>)};
  auto products = getManyByType(typeInfoOfWrapper);
  std::vector<Handle<PROD>> tmp;
  cet::transform_all(products, back_inserter(tmp), [](auto const& pr) {
    auto product = pr.first;
    auto wrapped_product = dynamic_cast<art::Wrapper<PROD> const*>(product);
    assert(wrapped_product != nullptr);
    auto user_product = wrapped_product->product();
    assert(user_product != nullptr);
    return Handle<PROD>{user_product, pr.second};
  });
  swap(tmp, result);
}

template <typename PROD>
inline std::vector<art::InputTag>
gallery::Event::getInputTags() const
{
  std::type_info const& typeInfoOfWrapper{typeid(art::Wrapper<PROD>)};
  return dataGetterHelper_->getInputTags(typeInfoOfWrapper);
}

template <typename PROD>
inline std::vector<art::ProductToken<PROD>>
gallery::Event::getProductTokens() const
{
  std::vector<art::ProductToken<PROD>> result;
  auto const tags = getInputTags<PROD>();
  cet::transform_all(tags, back_inserter(result), [](auto const& tag) {
    return art::ProductToken<PROD>{tag};
  });
  return result;
}

inline void
gallery::Event::first()
{
  return toBegin();
}

#endif /* gallery_Event_h */

// Local Variables:
// mode: c++
// End:
