
# Unity Scope files
configure_file(
  mediascanner-music.ini.in
  mediascanner-music.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/mediascanner-music.ini"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/mediascanner-music"
)

configure_file(
  mediascanner-video.ini.in
  mediascanner-video.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/mediascanner-video.ini"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/mediascanner-video"
)
