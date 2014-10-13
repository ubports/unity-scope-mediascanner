include_directories(${UNITY_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_library(mediascanner-video MODULE video-scope.cpp)
#set_target_properties(mediascanner-video PROPERTIES PREFIX "")
target_link_libraries(mediascanner-video scope-utils ${UNITY_LDFLAGS} ${Boost_LIBRARIES})

configure_file(manifest.json.in manifest.json)
intltool_merge(${CMAKE_CURRENT_SOURCE_DIR}/mediascanner-video.ini.in mediascanner-video.ini)

if(CLICK_MODE)
  click_build(com.ubuntu.scopes.myvideos ${VERSION}
    ${CMAKE_CURRENT_BINARY_DIR}/manifest.json apparmor.json)
  click_scope(com.ubuntu.scopes.myvideos myvideos
    mediascanner-video ${CMAKE_CURRENT_BINARY_DIR}/mediascanner-video.ini)
  click_scope_data(com.ubuntu.scopes.myvideos myvideos
    screenshot.jpg
    icon.png)
else()
  set(scopedir "${CMAKE_INSTALL_LIBDIR}/unity-scopes/mediascanner-video")
  install(
    TARGETS mediascanner-video
    LIBRARY DESTINATION "${scopedir}")
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/mediascanner-video.ini
      screenshot.jpg
      icon.png
    DESTINATION "${scopedir}")
endif(CLICK_MODE)
