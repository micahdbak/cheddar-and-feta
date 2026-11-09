#!/usr/bin/env bash
set -euo pipefail

CLANG_TIDY=${CLANG_TIDY:-clang-tidy}

if [ ! -f build/compile_commands.json ]; then
  echo "build/compile_commands.json not found." >&2
  echo "Run cmake -S . -B build first." >&2
  exit 1
fi

status=0

# tidy <clang-tidy args...> -- <git pathspec...>
#
# The argument split keeps empty file lists from running clang-tidy with no
# input, which it treats as an error.
tidy() {
  local args=()
  while [ "$#" -gt 0 ] && [ "$1" != "--" ]; do
    args+=("$1")
    shift
  done
  shift

  local files=()
  mapfile -d '' files < <(git ls-files -z "$@")
  if [ "${#files[@]}" -eq 0 ]; then
    return 0
  fi

  "${CLANG_TIDY}" -p build --quiet --fix "${args[@]}" "${files[@]}" ||
    status=1
}

tidy -- 'src/*.cc' 'src/*.c'

# clang-tidy reports only on the main file, so headers need their own pass.
# Headers have no compile command, so supply the language and include paths.
# Game headers are C++.
GAME_HEADER_ARGS=(
  --extra-arg=-xc++
  --extra-arg=-std=c++26
  --extra-arg=-I"${PWD}/src"
  --extra-arg=-I"${PWD}/thoom/include"
  --extra-arg=-I"${PWD}/thoom/src"
  --extra-arg=-I"${PWD}/thoom/deps/libdatachannel/include"
  --extra-arg=-I"${PWD}/build/thoom/deps/SDL/include-revision"
  --extra-arg=-I"${PWD}/thoom/deps/SDL/include"
)
tidy "${GAME_HEADER_ARGS[@]}" -- 'src/*.h'

exit "${status}"
