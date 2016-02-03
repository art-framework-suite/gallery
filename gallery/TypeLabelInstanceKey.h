#ifndef art_FWLite_TypeLabelInstanceKey_h
#define art_FWLite_TypeLabelInstanceKey_h

#include "canvas/Utilities/TypeID.h"

#include <cstring>

namespace canvas {

  class TypeLabelInstanceKey {
  public:
    //NOTE: Do not take ownership of strings.  This is done to avoid
    // doing 'new's and string copies when we just want to lookup the data
    // This means something else is responsible for the pointers remaining
    // valid for the time for which the pointers are still in use
    TypeLabelInstanceKey(art::TypeID const& iType,
                         char const* iLabel,
                         char const* iInstance) :
      type_(iType),
      label_(iLabel),
      instance_(iInstance) {
    }

    ~TypeLabelInstanceKey() {
    }

    bool operator<(TypeLabelInstanceKey const& iRHS) const {
      if (type_ < iRHS.type_) {
        return true;
      }
      if (iRHS.type_ < type_) {
        return false;
      }
      int comp = std::strcmp(label_, iRHS.label_);
      if (0 != comp) {
        return comp < 0;
      }
      comp = std::strcmp(instance_, iRHS.instance_);
      return comp < 0;
    }

    art::TypeID const& typeID() const { return type_; }
    char const* label() const { return label_; }
    char const* instance() const { return instance_; }

  private:
    art::TypeID type_;
    char const* label_;
    char const* instance_;
  };
}
#endif /* art_FWLite_TypeLabelInstanceKey_h */

// Local Variables:
// mode: c++
// End:
