add_executable(
  test-scope
  test-scope.c
  utils.c
  ../src/base-scope.cpp
  ../src/music-scope.cpp
  ../src/video-scope.cpp
)
target_link_libraries(test-scope ${UNITY_LIBRARIES})

add_test(test-scope test-scope)

#add_test(
#  test-with-mediascanner
#  ${CMAKE_CURRENT_SOURCE_DIR}/test-with-mediascanner.py -v
#)
