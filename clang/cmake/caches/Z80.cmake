# Z80.cmake
# Configure llvm-z80 for building
# Usage:
#   cmake -C clang/cmake/caches/Z80.cmake -G Ninja -S llvm -B build
#   ninja -C build

set(LLVM_TARGETS_TO_BUILD "" CACHE STRING "")
set(LLVM_EXPERIMENTAL_TARGETS_TO_BUILD "Z80" CACHE STRING "")
set(LLVM_ENABLE_PROJECTS "clang;lld" CACHE STRING "")

# Disable optional dependencies (not needed for Z80 cross-compiler)
set(LLVM_ENABLE_LIBXML2 OFF CACHE BOOL "")
set(LLVM_ENABLE_ZLIB OFF CACHE BOOL "")
set(LLVM_ENABLE_ZSTD OFF CACHE BOOL "")
set(LLVM_ENABLE_BINDINGS OFF CACHE BOOL "")

# Skip the third-party/benchmark subdir (ravn/llvm-z80 #124).  Its
# HAVE_PTHREAD_AFFINITY probe fails on macOS (no cpu_set_t/CPU_SETSIZE), and
# cmake 4.2 (bundled with CLion) treats that failure as fatal, breaking any
# fresh configure / bisect-walk reconfigure.  We never run LLVM's microbenchmarks.
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "")

set(CMAKE_BUILD_TYPE Release CACHE STRING "")
