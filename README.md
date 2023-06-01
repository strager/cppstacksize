## Building with Emscripten

Create a toolchain file:

```
[binaries]
exe_wrapper = 'node'
```

Configure:

```
emconfigure meson setup --cross-file .../emscripten.meson build-emscripten/
```

Build and install:

```
emmake meson install -C build-emscripten/ --destdir "${PWD}/src/build/"
```

## License

cppstacksize finds C++ stack usage bugs.

Copyright (C) 2023 Matthew "strager" Glazar

SPDX-License-Identifier: GPL-3.0-or-later

## Related projects

* [puncover][] - ELF cross-function stack analyzer

[puncover]: https://github.com/HBehrens/puncover
