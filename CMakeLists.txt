cmake_minimum_required(VERSION 3.16)
project(soro)

option(RAPID_LINT "Run clang-tidy with the compiler." OFF)

include(cmake/buildcache.cmake)
include(cmake/pkg.cmake)
if(RAPID_LINT)
  include(cmake/clang-tidy.cmake)
endif()

file(GLOB_RECURSE soro-lib-files src/*.cc)
add_library(soro-lib ${soro-lib-files})
target_compile_features(soro-lib PUBLIC cxx_std_20)
target_link_libraries(soro-lib utl cista date pugixml)
target_include_directories(soro-lib PUBLIC include)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(soro-lib PRIVATE
    -fno-strict-aliasing
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-newline-eof
    -Wno-missing-prototypes
    -Wno-padded
    -Wno-double-promotion
    -Wno-undef
    -Wno-undefined-reinterpret-cast
    -Wno-float-conversion
    -Wno-global-constructors
    -Wno-exit-time-destructors
    -Wno-switch-enum
    -Wno-c++20-designator
    -Werror
  )
elseif(MSVC)
  target_compile_options(soro-lib PRIVATE
    /WX
  )
else()
  target_compile_options(soro-lib PRIVATE
    -Wall
    -Wextra
    -Werror
  )
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
  file(GLOB_RECURSE soro-web-files web/*.cc)
  add_executable(soro-web ${soro-web-files})
  set_target_properties(soro-web PROPERTIES SUFFIX ".wasm.js")
  set_target_properties(soro-web PROPERTIES LINK_FLAGS "-s TOTAL_MEMORY=1024MB -s WASM=1 -s --bind")
  set_target_properties(soro-web PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/web)
  target_link_libraries(soro-web soro-lib)
  configure_file(web/index.html ${CMAKE_CURRENT_BINARY_DIR}/web/index.html COPYONLY)
  configure_file(web/worker.js ${CMAKE_CURRENT_BINARY_DIR}/web/worker.js COPYONLY)
  configure_file(web/ace.js ${CMAKE_CURRENT_BINARY_DIR}/web/ace.js COPYONLY)
  configure_file(web/viz.js ${CMAKE_CURRENT_BINARY_DIR}/web/viz.js COPYONLY)
  configure_file(web/fabric.min.js ${CMAKE_CURRENT_BINARY_DIR}/web/fabric.min.js COPYONLY)
  configure_file(web/split.min.js ${CMAKE_CURRENT_BINARY_DIR}/web/split.min.js COPYONLY)
  configure_file(web/svg-pan-zoom.min.js ${CMAKE_CURRENT_BINARY_DIR}/web/svg-pan-zoom.min.js COPYONLY)
  configure_file(web/plotly.min.js ${CMAKE_CURRENT_BINARY_DIR}/web/plotly.min.js COPYONLY)
  configure_file(web/running_time_calc.html ${CMAKE_CURRENT_BINARY_DIR}/web/running_time_calc.html COPYONLY)
endif()

file(GLOB_RECURSE soro-test-files test/*.cc)
add_executable(soro-test ${soro-test-files})
target_link_libraries(soro-test doctest soro-lib)

add_executable(soro-acceleration-benchmark benchmark/acceleration.cc)
target_link_libraries(soro-acceleration-benchmark soro-lib benchmark)