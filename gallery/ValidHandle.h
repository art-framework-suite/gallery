#ifndef gallery_ValidHandle_h
#define gallery_ValidHandle_h

// Should be identical to art::ValidHandle except it is missing
// functions, data members, typedefs, and dependences not needed in
// gallery.

#include "canvas/Persistency/Provenance/ProductID.h"

namespace gallery {

  void throwValidHandleNullPointer();

  template <typename T>
  class ValidHandle {
  public:
    using element_type = T;
    class HandleTag {};

    ValidHandle() = delete;
    explicit ValidHandle(T const* prod, art::ProductID) noexcept(false);
    ValidHandle(ValidHandle const&) = default;
    ValidHandle& operator=(ValidHandle const&) = default;

    // pointer behaviors
    T const& operator*() const noexcept;
    T const* operator->() const noexcept; // alias for product()
    T const* product() const noexcept;

    art::ProductID id() const noexcept;

  private:
    T const* prod_;
    art::ProductID productID_;
  };

  template <class T>
  ValidHandle<T>::ValidHandle(T const* prod,
                              art::ProductID const productID) noexcept(false)
    : prod_{prod}, productID_{productID}
  {
    if (prod == nullptr) {
      throwValidHandleNullPointer();
    }
  }

  template <class T>
  inline T const& ValidHandle<T>::operator*() const noexcept
  {
    return *prod_;
  }

  template <class T>
  inline T const* ValidHandle<T>::operator->() const noexcept
  {
    return prod_;
  }

  template <class T>
  inline T const*
  ValidHandle<T>::product() const noexcept
  {
    return prod_;
  }

  template <class T>
  inline art::ProductID
  ValidHandle<T>::id() const noexcept
  {
    return productID_;
  }
} // namespace gallery

#endif /* gallery_ValidHandle_h */

// Local Variables:
// mode: c++
// End:
