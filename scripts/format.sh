#!/usr/bin/env bash
set -euo pipefail

CLANG_FORMAT=${CLANG_FORMAT:-clang-format}

# clang-format picks the language from the extension, so C and C++ share a pass.
find src -type f \( -name '*.h' -o -name '*.c' -o -name '*.cc' \) -print0 |
  xargs -0 "${CLANG_FORMAT}" -i --style=Google

echo "Formatted src/."
