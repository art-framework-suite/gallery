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
} // namespace art

namespace gallery {

  class EventNavigator;

  class BranchData : public art::EDProductGetter {
  public:
    virtual ~BranchData();

    BranchData() = default;

    explicit BranchData(art::TypeID const& type,
                        TClass* iTClass,
                        TBranch* branch,
                        EventNavigator const* eventNavigator,
                        art::PrincipalBase const* finder,
                        std::string&& iBranchName);

    BranchData(BranchData const&) = delete;
    BranchData(BranchData&&) = delete;
    BranchData& operator=(BranchData const&) = delete;
    BranchData& operator=(BranchData&&) = delete;

  public:
    virtual void updateFile(TBranch* iBranch);

    TClass*
    tClass() const noexcept
    {
      return tClass_;
    }

    void*
    address() const noexcept
    {
      return address_;
    }

    TBranch*
    branch() const noexcept
    {
      return branch_;
    }

    std::string const&
    branchName() const noexcept
    {
      return branchName_;
    }

    long long
    lastProduct() const noexcept
    {
      return lastProduct_;
    }

    virtual art::EDProduct const* getIt_() const;
    virtual art::EDProduct const* uniqueProduct_() const;
    virtual art::EDProduct const* uniqueProduct_(art::TypeID const&) const;

  private:
    TClass* tClass_{nullptr};
    void* address_{nullptr};
    art::EDProduct const* edProduct_{nullptr};
    TBranch* branch_{nullptr};
    EventNavigator const* eventNavigator_{nullptr};
    cet::exempt_ptr<art::PrincipalBase const> finder_{nullptr};
    mutable long long lastProduct_{-1};
    std::string branchName_{};
  };

} // namespace gallery

#endif /* gallery_BranchData_h */

// Local Variables:
// mode: c++
// End:
