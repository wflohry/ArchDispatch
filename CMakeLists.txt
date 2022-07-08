cmake_minimum_required(VERSION 3.15)
project(ArchDispatch)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

set(TargetArch "sse4_1" CACHE STRING "Target archiecture")
set_property(CACHE TargetArch PROPERTY STRINGS sse2 sse4_1 avx avx2)
string(TOUPPER ${TargetArch} TargetArchUpper)

include("cmake/SetArch.cmake")
set_arch("${TargetArch}")


# libsimdpp
find_path(LIBSIMDPP_PATH NAMES simdpp/simd.h HINTS "${CMAKE_CURRENT_SOURCE_DIR}/submodules/libsimdpp")
add_library(libsimdpp INTERFACE)
target_include_directories(libsimdpp INTERFACE ${LIBSIMDPP_PATH})
add_library(libsimdpp::libsimdpp  ALIAS libsimdpp )

set(ARCH_SRC_DIR "src/ArchDispatch/")
add_library(ArchDispatch ${ARCH_SRC_DIR}/ArchDispatch.cpp
                    ${ARCH_SRC_DIR}/ArchDispatch.h
                )
target_include_directories(ArchDispatch PUBLIC src)
target_link_libraries(ArchDispatch PRIVATE libsimdpp)
add_library(ArchDispatch::ArchDispatch ALIAS ArchDispatch)

option(ARCH_DISPATCH_BUILD_EXAMPLES "Whether to build Arch Dispatch examples" OFF)
if (ARCH_DISPATCH_BUILD_EXAMPLES)
    add_library(for_loop${TargetArchUpper} SHARED examples/for_loop.cpp)
    if (unix)
        target_compile_options(for_loop${TargetArchUpper} PRIVATE -fvisibility=hidden)
    endif()

    add_executable(example_simple examples/simple.cpp)
    target_link_libraries(example_simple PRIVATE ArchDispatch::ArchDispatch)
endif()
