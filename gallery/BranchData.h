#ifndef gallery_BranchData_h
#define gallery_BranchData_h

// BranchData holds a pointer to a single TBranch and
// the buffer that data is written to from that TBranch.

#include "canvas/Persistency/Common/EDProductGetter.h"

#include "cetlib/exempt_ptr.h"

#include <string>

class TBranch;
class TClass;

namespace art {
  class EDProduct;
  class EDProductGetterFinder;
  class TypeID;
}

namespace gallery {

  class EventNavigator;

  class BranchData : public art::EDProductGetter {
  public:

    BranchData();

    BranchData(art::TypeID const& type,
               TClass* iTClass,
               TBranch* branch,
               EventNavigator const* eventNavigator,
               art::EDProductGetterFinder const* finder,
               std::string&& iBranchName);

    BranchData(BranchData const&) = delete;
    BranchData const& operator=(BranchData const&) = delete;

    virtual ~BranchData();

    virtual void updateFile(TBranch* iBranch);

    TClass* tClass() const { return tClass_; }
    void* address() const { return address_; }
    TBranch* branch() const { return branch_; }
    std::string const& branchName() const { return branchName_; }
    long long lastProduct() const { return lastProduct_; }

    bool isReady() const override { return true; }
    art::EDProduct const *getIt() const override;

    art::EDProduct const *anyProduct() const override;
    art::EDProduct const *uniqueProduct() const override;
    art::EDProduct const *uniqueProduct(art::TypeID const&) const override;
    bool resolveProduct(art::TypeID const&) const override;
    bool resolveProductIfAvailable(art::TypeID const&) const override;

  private:

    TClass* tClass_;
    void* address_;
    art::EDProduct const * edProduct_;
    TBranch* branch_;
    EventNavigator const* eventNavigator_;
    cet::exempt_ptr<art::EDProductGetterFinder const> finder_;
    mutable long long lastProduct_;
    std::string branchName_;
  };
}
#endif /* gallery_BranchData_h */

// Local Variables:
// mode: c++
// End:
