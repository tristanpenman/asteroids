#!/bin/bash
set -euo pipefail

build_dir=build

if ! cmake -S . -B "$build_dir"; then
    echo "CMake configuration failed; removing '$build_dir' and retrying." >&2
    rm -rf -- "$build_dir"
    cmake -S . -B "$build_dir"
fi

cmake --build "$build_dir" -- "$@"
