#!/bin/sh

set -e
set -u

mjs_path="${1}"
base_path="${mjs_path%.mjs}"
wasm_path="${base_path}.wasm"

if [ "${DESTDIR:-}" = "" ]; then
  printf 'error: $DESTDIR must be set (try meson install --destdir foo)\n' >&2
  exit 1
fi

# NOTE(strager): We intentionally avoid the prefix
# (MESON_INSTALL_DESTDIR_PREFIX). We want to make 'meson install --destdir foo'
# to copy directly into 'foo'.
mkdir -p "${DESTDIR}/"
cp -- "${mjs_path}" "${wasm_path}" "${DESTDIR}/"
