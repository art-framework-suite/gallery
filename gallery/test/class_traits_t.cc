#include "gallery/Event.h"

#include <type_traits>

template <typename T>
constexpr bool
nothrow_destructible()
{
  return std::is_nothrow_destructible_v<T>;
}

template <typename T>
constexpr bool
noncopyable()
{
  return not std::is_copy_constructible_v<T>;
}

template <typename T>
constexpr bool
nothrow_movable()
{
  return std::is_nothrow_move_constructible_v<T> and
         std::is_nothrow_move_assignable_v<T>;
}

int
main()
{
  static_assert(not std::is_default_constructible_v<gallery::Event>);
  static_assert(nothrow_destructible<gallery::Event>());
  static_assert(noncopyable<gallery::Event>());
  static_assert(nothrow_movable<gallery::Event>());
}
