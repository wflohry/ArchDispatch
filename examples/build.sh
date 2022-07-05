#!/bin/bash

root=$(readlink -f ./)
for arch in sse2 sse4_1 avx avx2; do
    cd "${root}" || exit
    dir="${root}/${arch}"
    mkdir -p "${dir}" && cd "${dir}" || exit 1
    cmake "${root}/../ArchDispatch" -DTargetArch="${arch}" -DARCH_DISPATCH_BUILD_EXAMPLES=1
    cmake --build ./
    libname=$(readlink -f "libfor_loop*.so")
    cp "${libname}" "${root}"
done

cd "${root}" || exit
cp sse2/example_simple ./
