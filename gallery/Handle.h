#ifndef gallery_Handle_h
#define gallery_Handle_h

// Similar to art::Handle, constructors are
// different and unneeded things have been removed
// but otherwise the interface is identical.

#include "canvas/Utilities/Exception.h"

#include <memory>

namespace gallery {

  void throwHandleWhyFailed(std::shared_ptr<art::Exception const>);

  template <typename T>
  class Handle
  {
  public:

    typedef T element_type;
    class HandleTag { };

    Handle() : prod_(nullptr) { }
    Handle(T const*);
    Handle(std::shared_ptr<art::Exception const>);
    Handle(Handle const&) = default;
    Handle& operator=(Handle const&) = default;

    // pointer behaviors
    T const & operator*() const;
    T const * operator->() const; // alias for product()
    T const * product() const;

    // inspectors:
    bool isValid() const;
    std::shared_ptr<art::Exception const> whyFailed() const;

  private:
    T const*   prod_;
    std::shared_ptr<art::Exception const>  whyFailed_;
  };

  template <class T>
  Handle<T>::Handle(T const* prod) :
    prod_(prod)
  {
  }

  template <class T>
  Handle<T>::Handle(std::shared_ptr<art::Exception const> iWhyFailed) :
    prod_(nullptr),
    whyFailed_(iWhyFailed)
  {
  }

  template <class T>
  inline
  T const &
  Handle<T>::operator*() const
  {
    return *product();
  }

  template <class T>
  inline
  T const *
  Handle<T>::operator->() const
  {
    return product();
  }

  template <class T>
  inline
  T const*
  Handle<T>::product() const
  {
    if (!prod_) throwHandleWhyFailed(whyFailed_);
    return prod_;
  }

  template <class T>
  bool
  Handle<T>::isValid() const
  {
    return prod_ != nullptr;
  }

  template <class T>
  inline
  std::shared_ptr<art::Exception const>
  Handle<T>::whyFailed() const
  {
    return whyFailed_;
  }
}
#endif /* gallery_Handle_h */

// Local Variables:
// mode: c++
// End: