
# D-Bus service definition

configure_file(
  unity-scope-hollywood.service.in
  unity-scope-hollywood.service
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/unity-scope-hollywood.service"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/dbus-1/services"
)

# Unity Scope files

configure_file(
  music/hollywood.scope.in
  music/hollywood.scope
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/music/hollywood.scope"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/music"
)

configure_file(
  video/hollywood.scope.in
  video/hollywood.scope
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/video/hollywood.scope"
  DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes/video"
)
