set(SCOPE_SOURCES
  main.cpp
  base-scope.cpp
  music-scope.cpp
  video-scope.cpp
)

add_library(unity-scope-mediascanner MODULE ${SCOPE_SOURCES})
set_target_properties(unity-scope-mediascanner PROPERTIES PREFIX "")

target_link_libraries(unity-scope-mediascanner ${UNITY_LIBRARIES})

install(
  TARGETS unity-scope-mediascanner
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity")
