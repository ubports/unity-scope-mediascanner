if (NOT DEFINED GMOCK_ROOT)
    set(GMOCK_ROOT /usr/src/gmock)
endif()
add_subdirectory(${GMOCK_ROOT} gmock)
include_directories(${GMOCK_ROOT}/gtest/include)

set(gtest_libs gtest gmock gmock_main)
foreach(target ${gtest_libs})
  set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-missing-field-initializers -Wno-missing-declarations")
endforeach()

include_directories(${UNITY_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_executable(test-music-scope
  test-music-scope.cpp
  ../src/mymusic/music-scope.cpp
)

add_executable(test-music-aggregator
  test-music-aggregator.cpp
  ../src/musicaggregator/musicaggregatorquery.cpp
  ../src/musicaggregator/onlinemusicresultforwarder.cpp
  ../src/musicaggregator/musicaggregatorscope.cpp
)

add_definitions(
  -DTEST_SCOPE_DIRECTORY="${CMAKE_BINARY_DIR}/src"
)

target_link_libraries(test-music-scope
  scope-utils ${UNITY_LDFLAGS} ${gtest_libs} ${GIO_DEPS_LDFLAGS})
add_test(test-music-scope test-music-scope)

target_link_libraries(test-music-aggregator
  scope-utils ${UNITY_LDFLAGS} ${gtest_libs} ${GIO_DEPS_LDFLAGS})
add_test(test-music-aggregator test-music-aggregator)

add_executable(test-video-scope
  test-video-scope.cpp
  ../src/myvideos/video-scope.cpp
)
target_link_libraries(test-video-scope
  scope-utils ${UNITY_LDFLAGS} ${gtest_libs} ${Boost_LIBRARIES})
add_test(test-video-scope test-video-scope)

add_executable(test-result-forwarder
  test-result-forwarder.cpp)
target_link_libraries(test-result-forwarder
  scope-utils ${UNITY_LDFLAGS} ${gtest_libs})
add_test(test-result-forwarder test-result-forwarder)
