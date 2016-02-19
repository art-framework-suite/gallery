#ifndef gallery_HistoryGetterBase_h
#define gallery_HistoryGetterBase_h

#include "canvas/Persistency/Provenance/ProcessHistoryID.h"

namespace art {
  class History;
  class ProcessHistory;
}

namespace gallery {

  class HistoryGetterBase {
  public:

    HistoryGetterBase() = default;
    virtual ~HistoryGetterBase();

    HistoryGetterBase(HistoryGetterBase const&) = delete;
    HistoryGetterBase const& operator=(HistoryGetterBase const&) = delete;

    virtual art::ProcessHistoryID const& processHistoryID() const = 0;
    virtual art::ProcessHistory const& processHistory() const = 0;
    virtual art::History const& history() const = 0;
  };
}
#endif

// Local Variables:
// mode: c++
// End:
