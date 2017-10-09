#include "gallery/ValidHandle.h"

#include "canvas/Utilities/Exception.h"

namespace gallery {

  void
  throwValidHandleNullPointer()
  {
    throw art::Exception(art::errors::NullPointerError)
      << "Attempt to create ValidHandle with null pointer";
  }
} // namespace gallery
