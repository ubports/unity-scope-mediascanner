set(SCOPE_SOURCES
  main.c
  base-scope.c
  music-scope.c
  video-scope.c
)

add_library(unity-scope-mediascanner MODULE ${SCOPE_SOURCES})
set_target_properties(unity-scope-mediascanner PROPERTIES PREFIX "")

target_link_libraries(unity-scope-mediascanner ${UNITY_LIBRARIES})

install(
  TARGETS unity-scope-mediascanner
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity")
