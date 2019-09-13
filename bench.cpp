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

#include <benchmark/benchmark.h>

#include "bt/function.hpp"
#include "bt/inheritance.hpp"
#include "bt/tuples.hpp"

// Forward declaration of random_boolean,
// a helper for eliminating the full benchmark optimization
bool random_boolean();

// ----------------------------------------------------------------------------
// Benchmark for OOP-style behavior trees, using inheritance
// ----------------------------------------------------------------------------
namespace inheritance_bench {
using namespace bt::inheritance;
inline int count = 0;  // Global because it doesn't matter for this.

// A bunch of OOP boilerplate

class OkNode : public ConditionNode {
 public:
  bool evaluate() final { return random_boolean(); }
};

class Step1Node : public ActionNode {
 public:
  Step1Node() = default;
  bt::Status operator()() final { return bt::Success; }
};

class Step2Node : public ActionNode {
 public:
  Step2Node() = default;
  bt::Status operator()() final {
    count++;
    return bt::Success;
  }
};

// The benchmark

static void oop_style(benchmark::State& state) {
  auto ok = MakeNode<OkNode>();
  auto seq = MakeNode<Sequence>(MakeNode<Step1Node>(), MakeNode<Step2Node>());
  auto inheritance_tree = MakeNode<Fallback>(std::move(ok), std::move(seq));

  for (auto _ : state) {
    (*inheritance_tree)();  // That's ugly. A real implementation would probably use a better wrapper than just std::unique_ptr
    benchmark::DoNotOptimize(count);
  }
}

BENCHMARK(oop_style);

};  // namespace inheritance_bench

// ----------------------------------------------------------------------------
// Benchmark for std::function-based behavior tree implementation
// ----------------------------------------------------------------------------
namespace functional_bench {
static void functions(benchmark::State& state) {
  using namespace bt::function;
  int count = 0;
  // clang-format off
  auto ok = ConditionNode([]{ return random_boolean(); });
  auto seq = Sequence([]{ return bt::Success; }, 
                      [&]{ count++; return bt::Success; });
  // clang-format on

  BehaviorTree functional_tree = Fallback(ok, seq);

  for (auto _ : state) {
    functional_tree();
    benchmark::DoNotOptimize(count);
  }
}

BENCHMARK(functions);

}  // namespace functional_bench

// ----------------------------------------------------------------------------
// Benchmark for the std::tuple-based implementation
// When optimized, should compile to "++count" or similar.
// ----------------------------------------------------------------------------
namespace tuple_bench {
static void tuples(benchmark::State& state) {
  using namespace bt::tuples;
  int count = 0;

  // Pretty cool that it works like the std::function version, huh? CTAD is awesome.
  // clang-format off
  auto ok = ConditionNode([] { return random_boolean(); });
  auto seq = Sequence([] { return bt::Success; }, 
                      [&] { count++; return bt::Success; });
  // clang-format on

  auto tuple_tree = Fallback(ok, seq);

  for (auto _ : state) {
    tuple_tree();
    benchmark::DoNotOptimize(count);
  }
}

BENCHMARK(tuples);
}  // namespace tuple_bench

// ----------------------------------------------------------------------------
// Main function
// ----------------------------------------------------------------------------
BENCHMARK_MAIN();

// ----------------------------------------------------------------------------
// random_boolean implementation
// Not actually random, because the random functions make it harder to bechmark
// So an array with fixed size and random initial values is used
// ----------------------------------------------------------------------------
#include <algorithm>
#include <array>
#include <cstdlib>
bool random_boolean() {
  static auto const bools = [] {
    std::array<bool, 256> arr{};
    std::generate(begin(arr), end(arr), [] { return bool(rand() & 1); });
    return arr;
  }();
  static std::size_t i;
  return bools[i++ % bools.size()];
}

static void default_implementation(benchmark::State& state) {
  int count = 0;
  for (auto _ : state) {
    bool b = !random_boolean();
    if (b)
      count++;
    benchmark::DoNotOptimize(count);
  }
}

BENCHMARK(default_implementation);