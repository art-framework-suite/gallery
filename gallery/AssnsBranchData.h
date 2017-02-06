#ifndef gallery_AssnsBranchData_h
#define gallery_AssnsBranchData_h

#include "gallery/BranchData.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <string>
#include <vector>

class TBranch;
class TClass;

namespace art {
  class EDProduct;
  class EDProductGetterFinder;
}

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
                    std::vector<art::TypeID> const & infoPartnerTypes);

    virtual ~AssnsBranchData();

    art::EDProduct const*
    getIt() const override;

    art::EDProduct const*
    uniqueProduct() const override;

    art::EDProduct const*
    uniqueProduct(art::TypeID const& wanted_wrapper_type) const override;

  private:
    virtual void doResetProducts() const override;

    std::vector<art::TypeID> secondary_wrapper_types_;
    mutable std::vector<std::unique_ptr<art::EDProduct> > secondaryProducts_;
  };
}
#endif /* gallery_AssnsBranchData_h */

// Local Variables:
// mode: c++
// End:
