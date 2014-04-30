if (NOT DEFINED GMOCK_ROOT)
    set(GMOCK_ROOT /usr/src/gmock)
endif()
add_subdirectory(${GMOCK_ROOT} gmock)
include_directories(${GMOCK_ROOT}/gtest/include)

set(gtest_libs gtest gmock gmock_main)
foreach(target ${gtest_libs})
  set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-missing-field-initializers -Wno-missing-declarations")
endforeach()

include_directories(${UNITY_INCLUDE_DIRS} ${OLDUNITY_INCLUDE_DIRS})

add_executable(test-music-scope
  test-music-scope.cpp
  ../src/music-scope.cpp
)
target_link_libraries(test-music-scope ${UNITY_LDFLAGS} ${gtest_libs})
add_test(test-music-scope test-music-scope)

add_executable(test-video-scope
  test-video-scope.cpp
  ../src/video-scope.cpp
)
target_link_libraries(test-video-scope ${UNITY_LDFLAGS} ${gtest_libs})
add_test(test-video-scope test-video-scope)

add_executable(
  test-old-scope
  test-old-scope.cpp
  utils.cpp
  ../oldscope/base-scope.cpp
  ../oldscope/music-scope.cpp
  ../oldscope/video-scope.cpp
)
target_link_libraries(test-old-scope ${OLDUNITY_LDFLAGS})

add_test(test-old-scope test-old-scope)

#add_test(
#  test-with-mediascanner
#  ${CMAKE_CURRENT_SOURCE_DIR}/test-with-mediascanner.py -v
#)
