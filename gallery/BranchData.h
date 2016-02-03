#ifndef art_FWLite_BranchData_h
#define art_FWLite_BranchData_h

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

namespace canvas {

  class EventNavigator;

  class BranchData : public art::EDProductGetter {
  public:

    BranchData();

    BranchData(art::TypeID const& type,
               TBranch* branch,
               TClass* edProductTClass,
               EventNavigator const* eventNavigator,
               art::EDProductGetterFinder const* finder,
               std::string&& iBranchName);
    ~BranchData();

    void updateFile(TBranch* iBranch);

    void* address() const { return address_; }
    TBranch* branch() const { return branch_; }
    std::string const& branchName() const { return branchName_; }

    virtual bool isReady() const override { return true; }
    virtual art::EDProduct const *getIt() const override;

    virtual art::EDProduct const *anyProduct() const override;
    virtual art::EDProduct const *uniqueProduct() const override;
    virtual art::EDProduct const *uniqueProduct(art::TypeID const&) const override;
    virtual bool resolveProduct(bool, art::TypeID const&) const override;
    virtual bool resolveProductIfAvailable(bool, art::TypeID const&) const override;

  private:

    TClass* tClass_;
    void* address_;
    art::EDProduct const* edProduct_;
    TBranch* branch_;
    EventNavigator const* eventNavigator_;
    cet::exempt_ptr<art::EDProductGetterFinder const> finder_;
    mutable long long lastProduct_;
    std::string branchName_;
  };
}
#endif /* art_FWLite_BranchData_h */

// Local Variables:
// mode: c++
// End:
