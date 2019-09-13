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

#ifndef BT_INHERITANCE_HPP
#define BT_INHERITANCE_HPP

#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>
#include "status.hpp"

namespace bt::inheritance {

class BehaviorNode {
 public:
  virtual bt::Status operator()() = 0;
  virtual ~BehaviorNode() = default;
};

using BehaviorTree = std::unique_ptr<BehaviorNode>;

template <typename Node, typename... Args>
BehaviorTree MakeNode(Args&&... args) {
  return std::make_unique<Node>(std::forward<Args>(args)...);
}

using ActionNode = BehaviorNode;

class ConditionNode : public virtual BehaviorNode {
 public:
  bt::Status operator()() final { return evaluate() ? Success : Failure; }
  virtual bool evaluate() = 0;
  virtual ~ConditionNode() = default;
};

class Fallback : public virtual BehaviorNode {
 public:
  template <typename... Behaviors>
  Fallback(std::unique_ptr<Behaviors>... ptrs) : children_{} {
    (children_.emplace_back(ptrs.release()), ...);
  }

  Fallback(Fallback&&) = default;
  ~Fallback() = default;

  bt::Status operator()() override {
    for (auto& child : children_) {
      switch ((*child)()) {
        case Success: return Success;
        case Running: return Running;
        default: continue;
      }
    }
    return Failure;
  }

 private:
  std::vector<BehaviorTree> children_;
};

class Sequence : public virtual BehaviorNode {
 public:
  template <typename... Behaviors>
  Sequence(std::unique_ptr<Behaviors>... ptrs) : children_{} {
    (children_.emplace_back(ptrs.release()), ...);
  }

  Sequence(Sequence&&) = default;
  ~Sequence() = default;

  bt::Status operator()() override {
    for (auto& child : children_) {
      switch ((*child)()) {
        case Failure: return Failure;
        case Running: return Running;
        default: continue;
      }
    }
    return Success;
  }

 private:
  std::vector<BehaviorTree> children_;
};

class Parallel : public virtual BehaviorNode {
 public:
  template <typename... Behaviors>
  Parallel(unsigned threshold, std::unique_ptr<Behaviors>... ptrs) : threshold_{threshold}, children_{} {
    (children_.emplace_back(ptrs.release()), ...);
  }

  Parallel(Parallel&&) = default;
  ~Parallel() = default;

  bt::Status operator()() override {
    unsigned successes = 0;
    for (auto& child : children_) {
      switch ((*child)()) {
        case Success: successes++; break;
        case Running: return Running;
        default: continue;
      }
    }
    return (successes >= threshold_) ? Success : Failure;
  }

 private:
  unsigned threshold_;
  std::vector<BehaviorTree> children_;
};

}  // namespace bt::inheritance

#endif