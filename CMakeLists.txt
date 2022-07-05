cmake_minimum_required(VERSION 3.15)
project(ArchDispatch)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

function(add_flag flag)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endfunction()

set(TargetArch "sse4_1" CACHE STRING "Target archiecture")
set_property(CACHE TargetArch PROPERTY STRINGS sse2 sse4_1 avx avx2)
string(TOUPPER ${TargetArch} TargetArchUpper)

function(set_arch linux_arch windows_arch)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_flag(linux_arch)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_flag(linux_arch)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        add_flag(windows_arch)
    else()
        message(FATAL_ERROR "Not written for compiler ${CMAKE_CXX_COMPILER_ID}")
    endif()
endfunction()

if (${TargetArch} STREQUAL "sse2")
    set_arch("-msse2" "/arch:SSE2")
elseif(${TargetArch} STREQUAL "sse4_1")
    set_arch("-msse4.1" "/arch:SSE2") # note -- no sse4.2
elseif(${TargetArch} STREQUAL "avx")
    set_arch("-mavx" "/arch:AVX")
elseif(${TargetArch} STREQUAL "avx2")
    set_arch("-mavx2" "/arch:AVX2")
endif()

# libsimdpp
find_path(LIBSIMDPP_PATH NAMES simdpp/simd.h HINTS "${CMAKE_CURRENT_SOURCE_DIR}/submodules/libsimdpp")
add_library(libsimdpp INTERFACE)
target_include_directories(libsimdpp INTERFACE ${LIBSIMDPP_PATH})
add_library(libsimdpp::libsimdpp  ALIAS libsimdpp )

add_library(ArchDispatch src/ArchDispatch/ArchDispatch.cpp src/ArchDispatch/ArchDispatch.h)
target_include_directories(ArchDispatch PUBLIC src)
target_link_libraries(ArchDispatch PRIVATE libsimdpp)
add_library(ArchDispatch::ArchDispatch ALIAS ArchDispatch)

option(ARCH_DISPATCH_BUILD_EXAMPLES "Whether to build Arch Dispatch examples" OFF)
if (ARCH_DISPATCH_BUILD_EXAMPLES)
    add_library(for_loop${TargetArchUpper} SHARED examples/for_loop.cpp)

    add_executable(example_simple examples/simple.cpp)
    target_link_libraries(example_simple PRIVATE ArchDispatch::ArchDispatch)
endif()
