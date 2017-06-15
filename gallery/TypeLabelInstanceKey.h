#ifndef gallery_TypeLabelInstanceKey_h
#define gallery_TypeLabelInstanceKey_h

#include "canvas/Utilities/TypeID.h"

#include <string>
#include <tuple>

namespace gallery {

  class TypeLabelInstanceKey {
  public:

    TypeLabelInstanceKey(art::TypeID const& iType,
                         std::string const& iLabel,
                         std::string const& iInstance) :
      type_{iType},
      label_{iLabel},
      instance_{iInstance}
    {}

    bool operator<(TypeLabelInstanceKey const& iRHS) const
    {
      return std::tie(type_, label_, instance_) <
             std::tie(iRHS.type_, iRHS.label_, iRHS.instance_);
    }

    art::TypeID const& typeID() const { return type_; }
    std::string const& label() const { return label_; }
    std::string const& instance() const { return instance_; }

  private:
    art::TypeID type_;
    std::string label_;
    std::string instance_;
  };
}
#endif /* gallery_TypeLabelInstanceKey_h */

// Local Variables:
// mode: c++
// End:
