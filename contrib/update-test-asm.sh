#!/bin/sh

cd "${MESON_SOURCE_ROOT}"
exec node src/update-test-asm.mjs "${@}"
