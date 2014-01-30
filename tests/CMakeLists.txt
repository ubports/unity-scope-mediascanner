if (NOT DEFINED GTEST_ROOT)
    set(GTEST_ROOT /usr/src/gtest)
endif()

set(GTEST_SRC_DIR "${GTEST_ROOT}/src")
set(GTEST_INCLUDE_DIR ${GTEST_ROOT})

add_library(gtest STATIC
${GTEST_SRC_DIR}/gtest-all.cc
)
set_target_properties(gtest PROPERTIES INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR})
target_link_libraries(gtest ${CMAKE_THREAD_LIBS_INIT})

include_directories(${UNITY_INCLUDE_DIRS} ${OLDUNITY_INCLUDE_DIRS})

add_executable(test-music-scope
  test-music-scope.cpp
  ../src/music-scope.cpp
)
target_link_libraries(test-music-scope ${UNITY_LDFLAGS} gtest)
add_test(test-music-scope test-music-scope)

add_executable(
  test-old-scope
  test-old-scope.cpp
  utils.cpp
  ../oldscope/base-scope.cpp
  ../oldscope/music-scope.cpp
  ../oldscope/video-scope.cpp
)
target_link_libraries(test-old-scope ${OLDUNITY_LIBRARIES})

add_test(test-old-scope test-old-scope)

#add_test(
#  test-with-mediascanner
#  ${CMAKE_CURRENT_SOURCE_DIR}/test-with-mediascanner.py -v
#)
