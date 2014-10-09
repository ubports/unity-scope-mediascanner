include_directories(${UNITY_INCLUDE_DIRS})

add_library(mediascanner-music MODULE music-scope.cpp)
#set_target_properties(mediascanner-music PROPERTIES PREFIX "")
target_link_libraries(mediascanner-music scope-utils ${UNITY_LDFLAGS} ${GIO_DEPS_LDFLAGS})

configure_file(manifest.json.in manifest.json)
intltool_merge(${CMAKE_CURRENT_SOURCE_DIR}/mediascanner-music.ini.in mediascanner-music.ini)

if(CLICK_MODE)
  click_build(com.ubuntu.scopes.mymusic ${VERSION}
    ${CMAKE_CURRENT_BINARY_DIR}/manifest.json apparmor.json)
  click_scope(com.ubuntu.scopes.mymusic mymusic
    mediascanner-music ${CMAKE_CURRENT_BINARY_DIR}/mediascanner-music.ini)
  click_scope_data(com.ubuntu.scopes.mymusic mymusic
    screenshot.jpg
    icon.png)
else()
  set(scopedir "${CMAKE_INSTALL_LIBDIR}/unity-scopes/mediascanner-music")
  install(
    TARGETS mediascanner-music
    LIBRARY DESTINATION "${scopedir}")
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/mediascanner-music.ini
      screenshot.jpg
      icon.png
    DESTINATION "${scopedir}")
endif(CLICK_MODE)
