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
  ../src/music-scope.cpp
)
target_link_libraries(test-music-scope ${UNITY_LDFLAGS} ${gtest_libs} ${GIO_DEPS_LDFLAGS})
add_test(test-music-scope test-music-scope)

add_executable(test-video-scope
  test-video-scope.cpp
  ../src/video-scope.cpp
)
target_link_libraries(test-video-scope ${UNITY_LDFLAGS} ${gtest_libs} ${Boost_LIBRARIES})
add_test(test-video-scope test-video-scope)

add_executable(test-result-forwarder
                test-result-forwarder.cpp
                ../src/notify-strategy.cpp
                ../src/resultforwarder.cpp
                ../src/bufferedresultforwarder.cpp)
target_link_libraries(test-result-forwarder ${UNITY_LDFLAGS} ${gtest_libs})
add_test(test-result-forwarder test-result-forwarder)
