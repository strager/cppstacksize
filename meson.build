project(
  'cppstacksize',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++20'],
)

gtest_proj = subproject('gtest')

cppstacksize_includes = include_directories('src/')
cppstacksize_lib = library(
  'cppstacksize-lib',
  [
    'src/cppstacksize/hello.cpp',
  ],
  include_directories: [cppstacksize_includes],
)
cppstacksize_lib_dep = declare_dependency(
  link_with: cppstacksize_lib,
  include_directories : [cppstacksize_includes],
)

tests = executable(
  'cppstacksize-test',
  [
    'test/test-hello.cpp',
  ],
  dependencies: [
    cppstacksize_lib_dep,
    gtest_proj.get_variable('gtest_dep'),
    gtest_proj.get_variable('gtest_main_dep'),
  ],
)
test('tests', tests)
