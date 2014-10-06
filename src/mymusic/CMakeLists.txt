include_directories(${UNITY_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_library(mediascanner-music MODULE music-scope.cpp)
#set_target_properties(mediascanner-music PROPERTIES PREFIX "")
target_link_libraries(mediascanner-music ${UNITY_LDFLAGS})

configure_file(manifest.json.in manifest.json)
intltool_merge(${CMAKE_CURRENT_SOURCE_DIR}/mymusic.ini.in mymusic.ini)

click_build(com.ubuntu.scopes.mymusic ${VERSION}
  ${CMAKE_CURRENT_BINARY_DIR}/manifest.json apparmor.json)
click_scope(com.ubuntu.scopes.mymusic mymusic
  mediascanner-music ${CMAKE_CURRENT_BINARY_DIR}/mymusic.ini)
click_scope_data(com.ubuntu.scopes.mymusic mymusic
  screenshot.jpg
  icon.png)
