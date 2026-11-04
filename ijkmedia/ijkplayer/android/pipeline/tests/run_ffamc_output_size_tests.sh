#!/usr/bin/env bash
set -euo pipefail
test_dir=$(cd "$(dirname "$0")" && pwd)
out=${1:-/tmp/ijk-output-size-test}
mkdir -p "$out"
flags=()
if [[ ${SANITIZE:-0} == 1 ]]; then flags=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
read -r -a extra_flags <<< "${CFLAGS:-}"
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror "${flags[@]}" "${extra_flags[@]}" \
    "$test_dir/ffamc_output_size_test.c" -o "$out/ffamc_output_size_test"
"$out/ffamc_output_size_test"
