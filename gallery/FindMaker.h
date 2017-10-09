#ifndef gallery_FindMaker_h
#define gallery_FindMaker_h

// The primary reason for this class's existence is that I failed
// when I tried to call the FindOne contructor directly from pyROOT
// because of all the template complexities. (It might be possible
// and I just failed to find the correct insanely complex incantation ...)
// In pyROOT, it is much easier to call the functions defined below
// to construct a FindOne object and that is what makes them useful.

// This class is also needed for ROOT scripts using FindOne. Even if
// the ROOT scripts do not directly use it, they rely on the fact that
// FindMaker has a dictionary and its existence pulls in the template
// definitions from FindOne.h, FindOneP.h, FindMany.h and FindManyP.h.
// If this class did not exist and one attempted to directly include
// the FindOne.h header in a ROOT script, the ROOT script would fail
// complaining about the complex templates that are used in FindOne.h
// and the other headers it includes.

#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindManyP.h"
#include "canvas/Persistency/Common/FindOne.h"
#include "canvas/Persistency/Common/FindOneP.h"

namespace art {
  class InputTag;
}

namespace gallery {

  class Event;

  class FindMaker {
  public:
    FindMaker();

    template <typename PRODB, typename DATA, typename HANDLE>
    art::FindOne<PRODB, DATA>
    makeFindOne(HANDLE const& handle,
                Event const& event,
                art::InputTag const& inputTag)
    {
      return art::FindOne<PRODB, DATA>(handle, event, inputTag);
    }

    template <typename PRODB, typename DATA, typename HANDLE>
    art::FindOneP<PRODB, DATA>
    makeFindOneP(HANDLE const& handle,
                 Event const& event,
                 art::InputTag const& inputTag)
    {
      return art::FindOneP<PRODB, DATA>(handle, event, inputTag);
    }

    template <typename PRODB, typename DATA, typename HANDLE>
    art::FindMany<PRODB, DATA>
    makeFindMany(HANDLE const& handle,
                 Event const& event,
                 art::InputTag const& inputTag)
    {
      return art::FindMany<PRODB, DATA>(handle, event, inputTag);
    }

    template <typename PRODB, typename DATA, typename HANDLE>
    art::FindManyP<PRODB, DATA>
    makeFindManyP(HANDLE const& handle,
                  Event const& event,
                  art::InputTag const& inputTag)
    {
      return art::FindManyP<PRODB, DATA>(handle, event, inputTag);
    }
  };
} // namespace gallery
#endif /* gallery_FindMaker_h */

// Local Variables:
// mode: c++
// End:
