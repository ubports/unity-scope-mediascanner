set(SCOPE_SOURCES
  main.cpp
  base-scope.cpp
  music-scope.cpp
  video-scope.cpp
)

include_directories(${UNITY_INCLUDE_DIRS})

add_library(mediascanner-music MODULE music-scope.cpp)
#set_target_properties(mediascanner-music PROPERTIES PREFIX "")
target_link_libraries(mediascanner-music ${UNITY_LDFLAGS})

add_library(mediascanner-video MODULE video-scope.cpp)
#set_target_properties(mediascanner-video PROPERTIES PREFIX "")
target_link_libraries(mediascanner-video ${UNITY_LDFLAGS})



install(
  TARGETS mediascanner-music
  LIBRARY DESTINATION "${CMAKE_INSTALL_DATADIR}/unity/scopes/mediascanner-music")

install(
  TARGETS mediascanner-video
  LIBRARY DESTINATION "${CMAKE_INSTALL_DATADIR}/unity/scopes/mediascanner-video")
