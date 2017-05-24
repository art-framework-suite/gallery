#ifndef gallery_EventHistoryGetter_h
#define gallery_EventHistoryGetter_h

#include "canvas/Persistency/Provenance/ProcessHistoryID.h"

namespace art {
  class History;
  class ProcessHistory;
}

namespace gallery {

  class EventNavigator;

  class EventHistoryGetter {
  public:

    EventHistoryGetter(EventNavigator const*);
    virtual ~EventHistoryGetter() = default;

    EventHistoryGetter(EventHistoryGetter const&) = delete;
    EventHistoryGetter const& operator=(EventHistoryGetter const&) = delete;

    virtual art::ProcessHistoryID const& processHistoryID() const;
    virtual art::ProcessHistory const& processHistory() const;
    virtual art::History const& history() const;

  private:

    EventNavigator const* eventNavigator_;
  };
}
#endif /* gallery_EventHistoryGetter_h */

// Local Variables:
// mode: c++
// End:
