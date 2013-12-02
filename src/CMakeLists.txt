set(SCOPE_SOURCES
  main.cpp
  base-scope.cpp
  music-scope.cpp
  video-scope.cpp
)

#add_library(unity-scope-mediascanner MODULE ${SCOPE_SOURCES})
#set_target_properties(unity-scope-mediascanner PROPERTIES PREFIX "")

#target_link_libraries(unity-scope-mediascanner ${UNITY_LIBRARIES})

add_library(mediascanner-music MODULE music-scope.cpp)
set_target_properties(mediascanner-music PROPERTIES PREFIX "")
target_link_libraries(mediascanner-music ${UNITY_LIBRARIES})


install(
  TARGETS mediascanner-music
  LIBRARY DESTINATION "${CMAKE_INSTALL_DATADIR}/unity/scopes/mediascanner-music")
