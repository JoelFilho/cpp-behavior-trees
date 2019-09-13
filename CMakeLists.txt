cmake_minimum_required(VERSION 3.0.0)
project(bt_benchmark VERSION 0.0.1)

find_package(benchmark CONFIG REQUIRED)

add_executable(bt_benchmark bench.cpp)
target_compile_features(bt_benchmark PRIVATE cxx_std_17)
target_include_directories(bt_benchmark PRIVATE include)
target_link_libraries(bt_benchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)