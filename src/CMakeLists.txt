set(SCOPE_SOURCES
  main.c
  base-scope.c
  music-scope.c
  video-scope.c
)

add_library(unity-scope-hollywood MODULE ${SCOPE_SOURCES})
set_target_properties(unity-scope-hollywood PROPERTIES PREFIX "")

target_link_libraries(unity-scope-hollywood ${UNITY_LIBRARIES})

install(
  TARGETS unity-scope-hollywood
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIB_DIR}/unity")
