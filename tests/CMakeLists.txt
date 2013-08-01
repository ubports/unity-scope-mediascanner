add_executable(
  test-scope
  test-scope.c
  ../src/base-scope.c
  ../src/music-scope.c
  ../src/video-scope.c
)
add_test(test-scope test-scope)

target_link_libraries(test-scope ${UNITY_LIBRARIES})
