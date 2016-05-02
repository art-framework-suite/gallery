#include "gallery/Handle.h"

#include "canvas/Utilities/Exception.h"

namespace gallery {

  void throwHandleWhyFailed(std::shared_ptr<art::Exception const> whyFailed) {
    if (whyFailed) {
      throw *whyFailed;
    }
    throw art::Exception(art::errors::LogicError)
      << "Attempt to dereference invalid Handle with no stored exception\n"
      << "Maybe you forgot to call getByLabel before dereferencing\n";
  }
}
