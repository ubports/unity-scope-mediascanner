set(SCOPE_SOURCES
  main.cpp
  base-scope.cpp
  music-scope.cpp
  video-scope.cpp
)

#add_library(unity-scope-mediascanner MODULE ${SCOPE_SOURCES})
#set_target_properties(unity-scope-mediascanner PROPERTIES PREFIX "")

#target_link_libraries(unity-scope-mediascanner ${UNITY_LIBRARIES})

add_library(unity-scope-mediascanner-music MODULE music-scope.cpp)
set_target_properties(unity-scope-mediascanner-music PROPERTIES PREFIX "")
target_link_libraries(unity-scope-mediascanner-music ${UNITY_LIBRARIES})


install(
  TARGETS unity-scope-mediascanner-music
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity")
