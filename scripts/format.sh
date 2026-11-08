#!/usr/bin/env bash
set -euo pipefail

CLANG_FORMAT=${CLANG_FORMAT:-clang-format}

find src -type f \
  \( -name '*.h' -o -name '*.cc' \) -print0 |
  xargs -0 "${CLANG_FORMAT}" -i --style=Google

echo "Formatted src/."
