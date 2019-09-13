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

#ifndef BT_FUNCTION_HPP
#define BT_FUNCTION_HPP

#include <functional>
#include <utility>
#include <vector>
#include "status.hpp"

namespace bt::function {

using BehaviorNode = std::function<bt::Status()>;
using BehaviorTree = BehaviorNode;
using ActionNode = BehaviorNode;

BehaviorNode ConditionNode(std::function<bool()> evaluator) {
  return [evaluator_ = std::move(evaluator)]() { return evaluator_() ? Success : Failure; };
}

template <typename... Nodes>
BehaviorNode Fallback(Nodes&&... nodes) {
  return [children_ = std::vector<BehaviorNode>{BehaviorNode(std::forward<Nodes>(nodes))...}]() {
    for (auto& child : children_) {
      switch (child()) {
        case Success: return Success;
        case Running: return Running;
        default: continue;
      }
    }
    return Failure;
  };
}

template <typename... Nodes>
BehaviorNode Sequence(Nodes&&... nodes) {
  return [children_ = std::vector<BehaviorNode>{BehaviorNode(std::forward<Nodes>(nodes))...}]() {
    for (auto& child : children_) {
      switch (child()) {
        case Failure: return Failure;
        case Running: return Running;
        default: continue;
      }
    }
    return Success;
  };
}

template <typename... Nodes>
BehaviorNode Parallel(unsigned threshold, Nodes&&... nodes) {
  return [threshold_ = threshold, children_ = std::vector<BehaviorNode>{BehaviorNode(std::forward<Nodes>(nodes))...}]() {
    unsigned successes = 0;
    for (auto& child : children_) {
      switch (child()) {
        case Success: successes++; break;
        case Running: return Running;
        default: continue;
      }
    }
    return (successes >= threshold_) ? Success : Failure;
  };
}

}  // namespace bt::function

#endif