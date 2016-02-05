#ifndef gallery_EventHistoryGetter_h
#define gallery_EventHistoryGetter_h

#include "gallery/HistoryGetterBase.h"

namespace gallery {

  class EventNavigator;

  class EventHistoryGetter : public HistoryGetterBase {
  public:

    EventHistoryGetter(EventNavigator const*);
    virtual ~EventHistoryGetter();

    EventHistoryGetter(EventHistoryGetter const&) = delete;
    EventHistoryGetter const& operator=(EventHistoryGetter const&) = delete;

    virtual art::ProcessHistoryID const& processHistoryID() const override;
    virtual art::ProcessHistory const& processHistory() const override;
    virtual art::History const& history() const override;

  private:

    EventNavigator const* eventNavigator_;
  };
}
#endif

// Local Variables:
// mode: c++
// End:
