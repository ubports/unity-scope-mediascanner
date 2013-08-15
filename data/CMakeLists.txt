
# D-Bus service definition

configure_file(
  unity-scope-mediascanner.service.in
  unity-scope-mediascanner.service
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/unity-scope-mediascanner.service"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services"
)

# Unity Scope files

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
