# C++ Behavior Tree benchmarks

A Behavior Tree is an hierarchical abstraction utilized in robotics and AI (See the [Wikipedia
page](https://en.wikipedia.org/wiki/Behavior_tree_%28artificial_intelligence%2C_robotics_and_control%29)
for more information).

This repository aims to benchmark three behavior tree implementations:

* Inheritance-based (OOP-style), using `std::unique_ptr` to derived classes
* `std::function`-based
* `std::tuple`-based

The first two use heap allocations, the second one can be entirely allocated on the stack.

## The design

All tree implementations followed similar design:

* The control flow is synchronous and the tree traversal is recursive
* Each tree implementation contains the Conditional, Action, Sequence, Fallback and Parallel nodes
  and provides support for custom decorators
* The operator `()` was utilized for the tick and the operator returns `bt::Status`
* The heap allocated trees utilize `std::vector` for storage
  * Although this means the trees can be modified at runtime, support for modification was not
    implemented

## The benchmark

The benchmark utilized a simple tree:

    - Fallback
      - Conditional: returns a random boolean
      - Sequence
        - Action: always returns Success
        - Action: increments a counter and returns Success

A "default implementation" was also provided, where the behavior tree is reduced to
`if(!random_boolean()) count++;`, in order to compare the overhead of each implementation.

The [bench.cpp](bench.cpp) file contains the benchmark code and can be compiled with Google
Benchmark, using the provided CMake file.

## Results

### GCC 8, Ubuntu 18.04 (WSL1)

    -----------------------------------------------------------------
    Benchmark                       Time             CPU   Iterations
    -----------------------------------------------------------------
    oop_style                    9.05 ns         8.79 ns     74666667
    functions                    13.4 ns         13.5 ns     49777778
    tuples                       1.44 ns         1.41 ns    497777778
    default_implementation       1.41 ns         1.41 ns    497777778

### Clang 8, Ubuntu 18.04 (WSL1)

    -----------------------------------------------------------------
    Benchmark                       Time             CPU   Iterations
    -----------------------------------------------------------------
    oop_style                    13.1 ns         12.8 ns     56000000
    functions                    11.6 ns         11.5 ns     64000000
    tuples                       1.64 ns         1.61 ns    407272727
    default_implementation       1.60 ns         1.60 ns    448000000

### Clang 8, Windows 10, x64

    -----------------------------------------------------------------
    Benchmark                       Time             CPU   Iterations
    -----------------------------------------------------------------
    oop_style                    12.7 ns         12.2 ns     49777778
    functions                    13.1 ns         13.1 ns     56000000
    tuples                       1.71 ns         1.69 ns    407272727
    default_implementation       1.70 ns         1.69 ns    407272727

### MSVC 2017, Windows 10, x64

    -----------------------------------------------------------------
    Benchmark                       Time             CPU   Iterations
    -----------------------------------------------------------------
    oop_style                    15.3 ns         15.0 ns     44800000
    functions                    14.2 ns         14.2 ns     56000000
    tuples                       3.38 ns         3.30 ns    213333333
    default_implementation       3.47 ns         3.38 ns    203636364

## Observations

* The timing differences were too small between certain implementations to guarantee perfect
  results, e.g. the difference between tuple and default implementations varied between runs, both
  negatively and positively. 
    * **A better test tree must be created for the next iteration of the benchmark**.
* GCC (Linux) had the best results for the inheritance-based tests and differed from Clang on the
  same platform. 
* Clang's results on Linux for the `std::function` were really interesting. Faster than
  `libstdc++`'s implementation, even though the vtable performance from the inheritance test was
  worse than GCC's.
* MSVC's results were the slowest all-around, although I haven't tested using the most recent
  version of the compiler and the standard library to be certain. 
* WSL is awesome! If you develop cross-platform applications on Windows, it's definitely worth a
  try.
* Using zero-overhead abstractions in C++ still reigns, regardless of compiler or Standard Library
  implementation. **If you don't need something, e.g. reconfigurability in this application, don't
  pay for it**!

## (un) License

The code in this repository is **public domain**, released under [The Unlicense](UNLICENSE.md). 

Feel free to use any standalone Behavior Tree implementation in your applications or as a base for
your own implementations!