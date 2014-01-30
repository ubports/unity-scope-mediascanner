set(OLDSCOPE_SOURCES
  main.cpp
  base-scope.cpp
  music-scope.cpp
  video-scope.cpp
)

include_directories(${OLDUNITY_INCLUDE_DIRS})

add_library(unity-scope-mediascanner MODULE ${OLDSCOPE_SOURCES})
set_target_properties(unity-scope-mediascanner PROPERTIES PREFIX "")

target_link_libraries(unity-scope-mediascanner ${OLDUNITY_LIBRARIES})

install(
  TARGETS unity-scope-mediascanner
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity")
