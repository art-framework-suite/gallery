#ifndef gallery_AssnsBranchData_h
#define gallery_AssnsBranchData_h

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Utilities/TypeID.h"
#include "gallery/BranchData.h"

#include <memory>
#include <string>

class TBranch;
class TClass;

namespace art {
  class EDProduct;
  class EDProductGetterFinder;
} // namespace art

namespace gallery {

  class EventNavigator;

  class AssnsBranchData : public BranchData {
  public:
    AssnsBranchData(art::TypeID const& type,
                    TClass* iTClass,
                    TBranch* branch,
                    EventNavigator const* eventNavigator,
                    art::EDProductGetterFinder const* finder,
                    std::string&& iBranchName,
                    art::TypeID const& infoType,
                    art::TypeID const& infoPartnerType);

  private:
    void updateFile(TBranch* iBranch) override;

    art::EDProduct const* getIt() const override;

    art::EDProduct const* uniqueProduct() const override;

    art::EDProduct const* uniqueProduct(
      art::TypeID const& wanted_wrapper_type) const override;

    art::TypeID secondary_wrapper_type_{};
    mutable std::unique_ptr<art::EDProduct> secondaryProduct_{nullptr};
    mutable long long secondaryLastProduct_{-1};
  };
} // namespace gallery
#endif /* gallery_AssnsBranchData_h */

// Local Variables:
// mode: c++
// End:
