#include "gallery/EventHistoryGetter.h"

#include "gallery/EventNavigator.h"

namespace gallery {

  EventHistoryGetter::EventHistoryGetter(EventNavigator const* eventNavigator) :
    eventNavigator_(eventNavigator) {
  }

  EventHistoryGetter::~EventHistoryGetter() { }

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
