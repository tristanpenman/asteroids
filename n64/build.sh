#!/bin/bash
set -eu
rm -rf build
mkdir -p build/src
wine cmd /c compile.bat
