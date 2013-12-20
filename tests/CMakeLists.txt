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

add_executable(test-music-scope
  test-music-scope.cpp
  ../src/music-scope.cpp
)
target_link_libraries(test-music-scope ${UNITY_LIBRARIES} gtest)
add_test(test-music-scope test-music-scope)

#add_executable(
#  test-scope
#  test-scope.cpp
#  utils.cpp
#  ../src/base-scope.cpp
#  ../src/music-scope.cpp
#  ../src/video-scope.cpp
#)
#target_link_libraries(test-scope ${UNITY_LIBRARIES})

#add_test(test-scope test-scope)

#add_test(
#  test-with-mediascanner
#  ${CMAKE_CURRENT_SOURCE_DIR}/test-with-mediascanner.py -v
#)
