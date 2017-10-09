#ifndef gallery_BranchData_h
#define gallery_BranchData_h
// vim: set sw=2 expandtab :

// BranchData holds a pointer to a single TBranch and
// the buffer that data is written to from that TBranch.

#include "canvas/Persistency/Common/EDProductGetter.h"
#include "cetlib/exempt_ptr.h"

#include <string>

class TBranch;
class TClass;

namespace art {

  class EDProduct;
  class PrincipalBase;
  class TypeID;

  typedef PrincipalBase EDProductGetterFinder;

} // namespace art

namespace gallery {

  class EventNavigator;

  class BranchData : public art::EDProductGetter {

  public:
    virtual ~BranchData();

    BranchData() = default;

    BranchData(art::TypeID const& type,
               TClass* iTClass,
               TBranch* branch,
               EventNavigator const* eventNavigator,
               art::EDProductGetterFinder const* finder,
               std::string&& iBranchName);

    BranchData(BranchData const&) = delete;

    BranchData(BranchData&&) = delete;

    BranchData& operator=(BranchData const&) = delete;

    BranchData& operator=(BranchData&&) = delete;

  public:
    virtual void updateFile(TBranch* iBranch);

    TClass*
    tClass() const
    {
      return tClass_;
    }

    void*
    address() const
    {
      return address_;
    }

    TBranch*
    branch() const
    {
      return branch_;
    }

    std::string const&
    branchName() const
    {
      return branchName_;
    }

    long long
    lastProduct() const
    {
      return lastProduct_;
    }

    // bool isReady() const override { return true; }

    virtual art::EDProduct const* getIt_() const override;

    virtual art::EDProduct const*
    // anyProduct_() const override;
    anyProduct_() const;

    virtual art::EDProduct const*
    // uniqueProduct_() const override;
    uniqueProduct_() const;

    virtual art::EDProduct const*
    // uniqueProduct_(art::TypeID const&) const override;
    uniqueProduct_(art::TypeID const&) const;

    // bool resolveProduct_(art::TypeID const&) const override;

    virtual bool
    // resolveProductIfAvailable_(art::TypeID const&) const override;
    resolveProductIfAvailable_(art::TypeID const&) const;

  private:
    TClass* tClass_{nullptr};
    void* address_{nullptr};
    art::EDProduct const* edProduct_{nullptr};
    TBranch* branch_{nullptr};
    EventNavigator const* eventNavigator_{nullptr};
    cet::exempt_ptr<art::EDProductGetterFinder const> finder_{nullptr};
    mutable long long lastProduct_{-1};
    std::string branchName_{};
  };

} // namespace gallery

#endif /* gallery_BranchData_h */

// Local Variables:
// mode: c++
// End:
