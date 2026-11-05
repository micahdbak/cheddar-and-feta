#!/usr/bin/env bash
set -euo pipefail

CLANG_FORMAT=${CLANG_FORMAT:-clang-format}

find cheddar feta map_editor shared -type f \
  \( -name '*.h' -o -name '*.cc' \) -print0 |
  xargs -0 "${CLANG_FORMAT}" -i --style=Google

echo "Formatted cheddar/, feta/, map_editor/, and shared/."
