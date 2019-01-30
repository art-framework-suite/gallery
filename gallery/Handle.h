#ifndef gallery_Handle_h
#define gallery_Handle_h

// ==================================================================
// Similar to art::Handle, constructors are different and unneeded
// things have been removed but otherwise the interface is identical.
// ==================================================================

#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/Exception.h"

#include <memory>

namespace gallery {

  template <typename T>
  class Handle {
  public:
    using element_type = T;
    class HandleTag {};

    Handle() = default;
    explicit Handle(T const*, art::ProductID);
    explicit Handle(std::shared_ptr<art::Exception const>);
    Handle(Handle const&) = default;
    Handle& operator=(Handle const&) = default;

    // pointer behaviors
    T const& operator*() const noexcept(false);
    T const* operator->() const noexcept(false); // alias for product()
    T const* product() const noexcept(false);

    // inspectors
    bool isValid() const noexcept;
    art::ProductID id() const noexcept;
    std::shared_ptr<art::Exception const> whyFailed() const noexcept;

  private:
    [[noreturn]] void throwHandleWhyFailed_() const noexcept(false);

    T const* prod_{nullptr};
    art::ProductID productID_{art::ProductID::invalid()};
    std::shared_ptr<art::Exception const> whyFailed_;
  };

  template <class T>
  Handle<T>::Handle(T const* prod, art::ProductID const productID)
    : prod_{prod}, productID_{productID}
  {}

  template <class T>
  Handle<T>::Handle(std::shared_ptr<art::Exception const> iWhyFailed)
    : whyFailed_{iWhyFailed}
  {}

  template <class T>
  inline T const& Handle<T>::operator*() const noexcept(false)
  {
    return *product();
  }

  template <class T>
  inline T const* Handle<T>::operator->() const noexcept(false)
  {
    return product();
  }

  template <class T>
  inline T const*
  Handle<T>::product() const noexcept(false)
  {
    if (!prod_)
      throwHandleWhyFailed_();
    return prod_;
  }

  template <class T>
  bool
  Handle<T>::isValid() const noexcept
  {
    return prod_ != nullptr;
  }

  template <class T>
  art::ProductID
  Handle<T>::id() const noexcept
  {
    return productID_;
  }

  template <class T>
  inline std::shared_ptr<art::Exception const>
  Handle<T>::whyFailed() const noexcept
  {
    return whyFailed_;
  }

  template <class T>
  void
  Handle<T>::throwHandleWhyFailed_() const noexcept(false)
  {
    if (whyFailed_) {
      throw *whyFailed_;
    }
    throw art::Exception(art::errors::LogicError)
      << "Attempt to dereference invalid Handle with no stored exception\n"
      << "Maybe you forgot to call getByLabel before dereferencing\n";
  }
} // namespace gallery
#endif /* gallery_Handle_h */

// Local Variables:
// mode: c++
// End:
