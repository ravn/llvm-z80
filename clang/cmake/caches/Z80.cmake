# Z80.cmake
# Configure llvm-z80 for building
# Usage:
#   cmake -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON -DLLVM_CCACHE_BUILD=ON \
#         -C clang/cmake/caches/Z80.cmake -G Ninja -S llvm -B build-macos-asserts
#   ninja -C build-macos-asserts
#
# -DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON  prevents ccache from seeing ~45% of
#   compile calls as uncacheable (PCH cannot be cached via RULE_LAUNCH_COMPILE).
# -DLLVM_CCACHE_BUILD=ON  activates ccache; set CCACHE_PROGRAM to your binary.

set(LLVM_TARGETS_TO_BUILD "" CACHE STRING "")
set(LLVM_EXPERIMENTAL_TARGETS_TO_BUILD "Z80" CACHE STRING "")
set(LLVM_ENABLE_PROJECTS "clang;lld" CACHE STRING "")

# Disable optional dependencies (not needed for Z80 cross-compiler)
set(LLVM_ENABLE_LIBXML2 OFF CACHE BOOL "")
set(LLVM_ENABLE_ZLIB OFF CACHE BOOL "")
set(LLVM_ENABLE_ZSTD OFF CACHE BOOL "")
set(LLVM_ENABLE_BINDINGS OFF CACHE BOOL "")

set(CMAKE_BUILD_TYPE Release CACHE STRING "")
