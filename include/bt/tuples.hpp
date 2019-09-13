// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#ifndef BT_TUPLES_HPP
#define BT_TUPLES_HPP

#include <tuple>
#include "status.hpp"

namespace bt::tuples {

namespace detail {
/// Recursive implementation of std::tuple iteration
/// Stop condition is defined on structured binding [stop, result] return by the function f
template <typename Tuple, class F, size_t I = 0>
auto iterate_tuple(Tuple& t, const F& f) {
  auto [stop, result] = f(std::get<I>(t));
  if constexpr (I < std::tuple_size_v<Tuple> - 1) {
    if (!stop)
      return iterate_tuple<Tuple, F, I + 1>(t, f);
  }
  return result;
}
}  // namespace detail

template <typename F>
class ConditionNode {
 public:
  ConditionNode(const F& evaluator) : evaluator_{evaluator} {}
  bt::Status operator()() { return evaluator_() ? Success : Failure; }

 private:
  F evaluator_;
};

template <typename... Behaviors>
class Fallback {
 public:
  Fallback(const Behaviors&... children) : children_{children...} {}

  bt::Status operator()() {
    return detail::iterate_tuple(children_, [](auto& behavior) {
      auto result = behavior();
      return std::tuple(result == Success || result == Running, result);
    });
  }

 private:
  std::tuple<Behaviors...> children_;
};

template <typename... Behaviors>
class Sequence {
 public:
  Sequence(const Behaviors&... children) : children_{children...} {}

  bt::Status operator()() {
    return detail::iterate_tuple(children_, [](auto& behavior) {
      auto result = behavior();
      return std::tuple(result == Failure || result == Running, result);
    });
  }

 private:
  std::tuple<Behaviors...> children_;
};

template <typename... Behaviors>
class Parallel {
 public:
  Parallel(size_t threshold, const Behaviors&... children) : threshold_{threshold}, children_{children...} {}

  bt::Status operator()() {
    size_t successes = 0;
    auto result = detail::iterate_tuple(children_, [&successes](auto& behavior) {
      auto result = behavior();
      if (result == Success)
        successes++;
      return std::tuple(result == Running, result);
    });
    return (result == Running) ? Running : (successes >= threshold_) ? Success : Failure;
  }

 private:
  size_t threshold_;
  std::tuple<Behaviors...> children_;
};

}  // namespace bt::tuples

#endif