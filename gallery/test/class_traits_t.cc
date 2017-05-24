#include "gallery/Event.h"
#include "gallery/EventHistoryGetter.h"
#include "gallery/EventNavigator.h"
#include "gallery/DataGetterHelper.h"

#include <type_traits>

template <typename T> 
constexpr bool nothrow_destructible() {
  return std::is_nothrow_destructible<T>::value;
}

template <typename T>
constexpr bool noncopyable() {
  return not std::is_copy_constructible<T>::value;
}

template <typename T>
constexpr bool nothrow_movable() {
  return std::is_nothrow_move_constructible<T>::value 
    and  std::is_nothrow_move_assignable<T>::value;
}

int main() {

  assert(std::is_default_constructible<gallery::Event>::value == false);
  assert(nothrow_destructible<gallery::Event>());
  assert(noncopyable<gallery::Event>());
  assert(nothrow_movable<gallery::Event>());
}
