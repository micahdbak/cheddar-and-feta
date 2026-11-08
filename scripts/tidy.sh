#!/usr/bin/env bash
set -euo pipefail

CLANG_TIDY=${CLANG_TIDY:-clang-tidy}

if [ ! -f build/compile_commands.json ]; then
  echo "build/compile_commands.json not found." >&2
  echo "Run cmake -S . -B build first." >&2
  exit 1
fi

status=0

git ls-files -z 'cheddar/*.cc' 'feta/*.cc' 'shared/*.cc' |
  xargs -0 "${CLANG_TIDY}" -p build --quiet --fix || status=1

# clang-tidy reports only on the main file, so headers need their own pass.
# Headers have no compile command, so supply C++ mode and include paths.
git ls-files -z 'cheddar/*.h' 'feta/*.h' 'shared/*.h' |
  xargs -0 "${CLANG_TIDY}" -p build --quiet --fix \
    --extra-arg=-xc++ \
    --extra-arg=-std=c++26 \
    --extra-arg=-I"${PWD}/cheddar" \
    --extra-arg=-I"${PWD}/feta" \
    --extra-arg=-I"${PWD}/shared" \
    --extra-arg=-I"${PWD}/thoom/include" \
    --extra-arg=-I"${PWD}/thoom/src" \
    --extra-arg=-I"${PWD}/thoom/deps/libdatachannel/include" \
    --extra-arg=-I"${PWD}/build/thoom/deps/SDL/include-revision" \
    --extra-arg=-I"${PWD}/thoom/deps/SDL/include" || status=1

exit "${status}"
