#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$(readlink -f "$0")")"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"${JOBS:-2}"
echo "Built: $(pwd)/build/officialcraft"
