#ifndef gallery_ValidHandle_h
#define gallery_ValidHandle_h

// Should be identical to art::ValidHandle
// except it is missing functions, data members,
// typedefs, and dependences not needed in gallery.

#include "canvas/Utilities/Exception.h"

#include <utility>

namespace gallery {

  template <typename T>
  class ValidHandle
  {
  public:

    ValidHandle() : prod_(nullptr) { }
    ValidHandle(T const* prod);
    ValidHandle(ValidHandle const&) = default;
    ValidHandle& operator=(ValidHandle const&) = default;

    // pointer behaviors
    T const & operator*() const;
    T const * operator->() const; // alias for product()
    T const * product() const;

  private:
    T const*   prod_;
  };

  template <class T>
  ValidHandle<T>::ValidHandle(T const* prod) :
    prod_(prod)
  {
    if (prod == nullptr)
      throw art::Exception(art::errors::NullPointerError)
        << "Attempt to create ValidHandle with null pointer";
  }

  template <class T>
  inline
  T const &
  ValidHandle<T>::operator*() const
  {
    return *prod_;
  }

  template <class T>
  inline
  T const *
  ValidHandle<T>::operator->() const
  {
    return prod_;
  }

  template <class T>
  inline
  T const*
  ValidHandle<T>::product() const
  {
    return prod_;
  }
}
#endif /* gallery_ValidHandle_h */

// Local Variables:
// mode: c++
// End:
