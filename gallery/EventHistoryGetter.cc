#include "gallery/EventHistoryGetter.h"

#include "gallery/EventNavigator.h"

#include <type_traits>

namespace gallery {

  EventHistoryGetter::EventHistoryGetter(EventNavigator const* eventNavigator) :
    eventNavigator_(eventNavigator) {
    static_assert(std::is_nothrow_destructible<EventHistoryGetter>::value,
		  "EventHistoryGetter is not nothrow destructible");
  }

  art::ProcessHistoryID const& EventHistoryGetter::processHistoryID() const {
    return eventNavigator_->processHistoryID();
  }

  art::ProcessHistory const& EventHistoryGetter::processHistory() const {
    return eventNavigator_->processHistory();
  }

  art::History const& EventHistoryGetter::history() const {
    return eventNavigator_->history();
  }
}
