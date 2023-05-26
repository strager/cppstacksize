project(
  'cppstacksize',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

capstone_proj = import('cmake').subproject('capstone')
gtest_proj = subproject('gtest')

cppstacksize_includes = include_directories('src/')
cppstacksize_lib = library(
  'cppstacksize-lib',
  [
    'src/cppstacksize/hello.cpp',
  ],
  include_directories: [cppstacksize_includes],
  dependencies: [
    capstone_proj.dependency('capstone-shared'),
  ],
)
cppstacksize_lib_dep = declare_dependency(
  link_with: cppstacksize_lib,
  include_directories : [cppstacksize_includes],
)

test_sources = [
  'test/test-hello.cpp',
]
tests = executable(
  'cppstacksize-test',
  test_sources,
  dependencies: [
    cppstacksize_lib_dep,
    gtest_proj.get_variable('gtest_dep'),
    gtest_proj.get_variable('gtest_main_dep'),
  ],
)
test('tests', tests)

run_target(
  'update-test-asm',
  command: ['contrib/update-test-asm.sh'] + test_sources,
)
