project(
  'cppstacksize',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

cmake = import('cmake')

capstone_cmake_options = cmake.subproject_options()
capstone_cmake_options.add_cmake_defines({
  'CAPSTONE_BUILD_CSTOOL': false,
  'CAPSTONE_BUILD_SHARED': false,
  'CAPSTONE_BUILD_STATIC': true,
  'CAPSTONE_BUILD_TESTS': false,
})
capstone_cmake_options.set_install(false)
capstone_proj = cmake.subproject('capstone', options: capstone_cmake_options)

fmt_proj = subproject('fmt')
gtest_proj = subproject('gtest')

cppstacksize_includes = include_directories('src/')
cppstacksize_lib = static_library(
  'cppstacksize-lib',
  [
    'src/cppstacksize/asm-stack-map-debug.cpp',
    'src/cppstacksize/asm-stack-map.cpp',
    'src/cppstacksize/asm-stack-map.h',
    'src/cppstacksize/base.h',
    'src/cppstacksize/c-api.cpp',
    'src/cppstacksize/c-api.h',
    'src/cppstacksize/codeview-constants.cpp',
    'src/cppstacksize/codeview-constants.h',
    'src/cppstacksize/codeview.h',
    'src/cppstacksize/file.cpp',
    'src/cppstacksize/file.h',
    'src/cppstacksize/guid.cpp',
    'src/cppstacksize/guid.h',
    'src/cppstacksize/logger.cpp',
    'src/cppstacksize/logger.h',
    'src/cppstacksize/pdb-reader.h',
    'src/cppstacksize/pdb.h',
    'src/cppstacksize/pe.h',
    'src/cppstacksize/reader.cpp',
    'src/cppstacksize/reader.h',
    'src/cppstacksize/register-debug.cpp',
    'src/cppstacksize/register.cpp',
    'src/cppstacksize/register.h',
    'src/cppstacksize/util.h',
  ],
  include_directories: [cppstacksize_includes],
  dependencies: [
    capstone_proj.dependency('capstone-static'),
    fmt_proj.get_variable('fmt_dep'),
  ],
)
cppstacksize_lib_dep = declare_dependency(
  link_with: cppstacksize_lib,
  include_directories : [cppstacksize_includes],
  # TODO(strager): Remove uses of fmt from our headers and make the dependency
  # private.
  dependencies: [fmt_proj.get_variable('fmt_dep')],
)

test_sources = [
  # Tests with ASM_X86_64 directives:
  'test/test-asm-stack-map.cpp',
  'test/test-register.cpp',

  'test/cppstacksize/asm.h',
  'test/cppstacksize/example-file.h',
  'test/test-codeview.cpp',
  'test/test-coff.cpp',
  'test/test-guid.cpp',
  'test/test-pdb.cpp',
  'test/test-pe.cpp',
  'test/test-reader-location.cpp',
  'test/test-reader.cpp',

  # HACK[example-file-path]: example-file.cpp uses __FILE__ which needs to expand to an
  # absolute path. Make the source path absolute to make __FILE__ absolute.
  meson.current_source_dir() / 'test/cppstacksize/example-file.cpp',
]
# TODO(strager): Commit the generated file into source control and make this
# step optional.
test_asm_generated_src = custom_target(
  'test-asm-generated',
  input: ['src/update-test-asm.mjs', test_sources],
  output: ['test-asm-generated.cpp'],
  command: [
    'node', '@SOURCE_ROOT@/src/update-test-asm.mjs',
    '@OUTPUT@',
    # TODO(strager): Auto-fill this based on test_sources.
    '@INPUT1@', '@INPUT2@',
  ],
)

executable(
  'dump-stack-map',
  ['src/cppstacksize/dump-stack-map.cpp'],
  dependencies: [cppstacksize_lib_dep],
)

tests = executable(
  'cppstacksize-test',
  [test_sources, test_asm_generated_src],
  include_directories: include_directories('test/'),
  dependencies: [
    cppstacksize_lib_dep,
    gtest_proj.get_variable('gmock_dep'),
    gtest_proj.get_variable('gmock_main_dep'),
  ],
  override_options: ['cpp_std=c++20'],
  cpp_pch: ['test/pch.h'],
)
test('tests', tests)

# TODO(strager): Only build when configured with Emscripten.
cppstacksize_wasm_exe = executable(
  'cppstacksize-wasm',
  dependencies: [cppstacksize_lib_dep],
  link_args: [
    '--no-entry',
    '-sENVIRONMENT=node,web',
    '-sEXPORTED_FUNCTIONS=["_css_create_analyzer_x86_64","_css_destroy_analyzer","_css_set_machine_code","_css_get_stack_map","_malloc","_free"]',
    '-sEXPORTED_RUNTIME_METHODS=cwrap',
    '-sEXPORT_ES6=1',
    '-sMODULARIZE=1',
  ],
  install: false,
  name_suffix: 'mjs',
)
meson.add_install_script(['contrib/install-wasm.sh', cppstacksize_wasm_exe])
