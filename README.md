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

## Related projects

* [puncover][] - ELF cross-function stack analyzer

[puncover]: https://github.com/HBehrens/puncover
