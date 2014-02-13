
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


configure_file(
  musicaggregator.ini.in
  musicaggregator.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/musicaggregator.ini"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/musicaggregator"
)

configure_file(
  videoaggregator.ini.in
  videoaggregator.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/videoaggregator.ini"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/videoaggregator"
)
# D-Bus service definition for old scope

configure_file(
  unity-scope-mediascanner.service.in
  unity-scope-mediascanner.service
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/unity-scope-mediascanner.service"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services"
)

# Old Unity Scope files

configure_file(
  music/mediascanner.scope.in
  music/mediascanner.scope
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/music/mediascanner.scope"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/music"
)

configure_file(
  video/mediascanner.scope.in
  video/mediascanner.scope
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/video/mediascanner.scope"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/video"
)
