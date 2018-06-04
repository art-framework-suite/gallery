#ifndef gallery_Handle_h
#define gallery_Handle_h

// Similar to art::Handle, constructors are different and unneeded
// things have been removed but otherwise the interface is identical.

#include "canvas/Utilities/Exception.h"

#include <memory>

namespace gallery {

  template <typename T>
  class Handle {
  public:
    using element_type = T;
    class HandleTag {
    };

    Handle() = default;
    Handle(T const*);
    Handle(std::shared_ptr<art::Exception const>);
    Handle(Handle const&) = default;
    Handle& operator=(Handle const&) = default;

    // pointer behaviors
    T const& operator*() const;
    T const* operator->() const; // alias for product()
    T const* product() const;

    // inspectors:
    bool isValid() const;
    std::shared_ptr<art::Exception const> whyFailed() const;

  private:
    void throwHandleWhyFailed_() const;

    T const* prod_{nullptr};
    std::shared_ptr<art::Exception const> whyFailed_;
  };

  template <class T>
  Handle<T>::Handle(T const* prod) : prod_{prod}
  {}

  template <class T>
  Handle<T>::Handle(std::shared_ptr<art::Exception const> iWhyFailed)
    : whyFailed_{iWhyFailed}
  {}

  template <class T>
  inline T const& Handle<T>::operator*() const
  {
    return *product();
  }

  template <class T>
  inline T const* Handle<T>::operator->() const
  {
    return product();
  }

  template <class T>
  inline T const*
  Handle<T>::product() const
  {
    if (!prod_)
      throwHandleWhyFailed_();
    return prod_;
  }

  template <class T>
  bool
  Handle<T>::isValid() const
  {
    return prod_ != nullptr;
  }

  template <class T>
  inline std::shared_ptr<art::Exception const>
  Handle<T>::whyFailed() const
  {
    return whyFailed_;
  }

  template <class T>
  void
  Handle<T>::throwHandleWhyFailed_() const
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
