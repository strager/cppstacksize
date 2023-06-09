project(
  'cppstacksize',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

cmake = import('cmake')
qt6 = import('qt6')

capstone_cmake_options = cmake.subproject_options()
capstone_cmake_options.add_cmake_defines({
  'CAPSTONE_BUILD_CSTOOL': false,
  'CAPSTONE_BUILD_SHARED': false,
  'CAPSTONE_BUILD_STATIC': true,
  'CAPSTONE_BUILD_TESTS': false,
})
capstone_cmake_options.set_install(false)
capstone_proj = cmake.subproject('capstone', options: capstone_cmake_options)

fmt_proj = subproject('fmt', default_options: 'default_library=static')
gtest_proj = subproject('gtest')

qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])

cppstacksize_includes = include_directories('src/')
cppstacksize_lib = static_library(
  'cppstacksize-lib',
  [
    'src/cppstacksize/asm-stack-map-debug.cpp',
    'src/cppstacksize/asm-stack-map.cpp',
    'src/cppstacksize/asm-stack-map.h',
    'src/cppstacksize/base.h',
    'src/cppstacksize/codeview-constants.cpp',
    'src/cppstacksize/codeview-constants.h',
    'src/cppstacksize/codeview.h',
    'src/cppstacksize/file.cpp',
    'src/cppstacksize/file.h',
    'src/cppstacksize/guid.cpp',
    'src/cppstacksize/guid.h',
    'src/cppstacksize/line-tables-debug.cpp',
    'src/cppstacksize/line-tables.h',
    'src/cppstacksize/logger.cpp',
    'src/cppstacksize/logger.h',
    'src/cppstacksize/pdb-reader.h',
    'src/cppstacksize/pdb.h',
    'src/cppstacksize/pe.h',
    'src/cppstacksize/project.h',
    'src/cppstacksize/reader.cpp',
    'src/cppstacksize/reader.h',
    'src/cppstacksize/register-debug.cpp',
    'src/cppstacksize/register.cpp',
    'src/cppstacksize/register.h',
    'src/cppstacksize/sparse-bit-set.h',
    'src/cppstacksize/stack-map-touch-group.cpp',
    'src/cppstacksize/stack-map-touch-group.h',
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

gui_moc_files = qt6.compile_moc(
  headers: [
    'src/cppstacksize/gui/function-table.h',
    'src/cppstacksize/gui/locals-table.h',
    'src/cppstacksize/gui/log-table.h',
    'src/cppstacksize/gui/main-window.h',
    'src/cppstacksize/gui/stack-map-table.h',
  ],
  dependencies: [cppstacksize_lib_dep, qt6_dep],
)
executable(
  'cppstacksize-gui',
  [
    'src/cppstacksize/gui/function-table.cpp',
    'src/cppstacksize/gui/function-table.h',
    'src/cppstacksize/gui/locals-table.cpp',
    'src/cppstacksize/gui/locals-table.h',
    'src/cppstacksize/gui/log-table.cpp',
    'src/cppstacksize/gui/log-table.h',
    'src/cppstacksize/gui/main-window.cpp',
    'src/cppstacksize/gui/main-window.h',
    'src/cppstacksize/gui/main.cpp',
    'src/cppstacksize/gui/stack-map-table.cpp',
    'src/cppstacksize/gui/stack-map-table.h',
    'src/cppstacksize/gui/style.h',
    gui_moc_files,
  ],
  cpp_pch: ['src/cppstacksize/gui/pch.h'],
  dependencies: [cppstacksize_lib_dep, qt6_dep],
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
  'test/test-line-tables.cpp',
  'test/test-pdb.cpp',
  'test/test-pe.cpp',
  'test/test-project.cpp',
  'test/test-reader-location.cpp',
  'test/test-reader.cpp',
  'test/test-sparse-bit-set.cpp',
  'test/test-stack-map-touch-group.cpp',

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
