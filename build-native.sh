#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
if ! command -v cmake >/dev/null; then
 echo 'CMake is required. On macOS: brew install cmake'; exit 1
fi
if [[ "$(uname -s)" == Darwin ]] && ! xcode-select -p >/dev/null 2>&1; then
 echo 'Install Apple command line tools first: xcode-select --install'; exit 1
fi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
ctest --test-dir build --output-on-failure
echo 'Built plugins and standalone app are in build/Bread_artefacts/Release/'
