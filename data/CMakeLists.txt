
# Unity Scope files
set(SCOPES_DIR "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes")
set(OLD_SCOPES_DIR "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes")

configure_file(
  mediascanner-music.ini.in
  mediascanner-music.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/mediascanner-music.ini" music.png mediascanner-music-screenshot.jpg
  DESTINATION "${SCOPES_DIR}/mediascanner-music"
)

configure_file(
  mediascanner-video.ini.in
  mediascanner-video.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/mediascanner-video.ini" video.png mediascanner-video-screenshot.jpg
  DESTINATION "${SCOPES_DIR}/mediascanner-video"
)


configure_file(
  musicaggregator.ini.in
  musicaggregator.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/musicaggregator.ini" musicaggregator-screenshot.jpg
  DESTINATION "${SCOPES_DIR}/musicaggregator"
)

configure_file(
  videoaggregator.ini.in
  videoaggregator.ini
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/videoaggregator.ini" videoaggregator-screenshot.jpg
  DESTINATION "${SCOPES_DIR}/videoaggregator"
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
  DESTINATION "${OLD_SCOPES_DIR}/music"
)

configure_file(
  video/mediascanner.scope.in
  video/mediascanner.scope
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/video/mediascanner.scope"
  DESTINATION "${OLD_SCOPES_DIR}/video"
)
