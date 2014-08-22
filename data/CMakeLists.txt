
# Unity Scope files
set(SCOPES_DATADIR "${CMAKE_INSTALL_FULL_DATADIR}/unity/scopes")
set(SCOPES_LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}/unity-scopes")

set(SCOPE_CONFIG_FILES
  mediascanner-music.ini
  mediascanner-video.ini
  musicaggregator.ini
  musicaggregator-settings.ini
  videoaggregator.ini
  videoaggregator-settings.ini
)

foreach(configfile ${SCOPE_CONFIG_FILES})
    configure_file(${configfile}.in.in ${configfile}.in)
    get_filename_component(basename ${configfile} NAME_WE)
    string(REPLACE "-settings" "" basename ${basename})
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${configfile}"
            DESTINATION "${SCOPES_LIBDIR}/${basename}")
    add_custom_command(OUTPUT ${configfile} COMMAND ${INTLTOOL_MERGE_BIN} -d ${CMAKE_CURRENT_SOURCE_DIR}/../po ${configfile}.in ${configfile})
endforeach()

add_custom_target(ini-i18n ALL DEPENDS ${SCOPE_CONFIG_FILES})

install(
  FILES music.png music-scope.png mediascanner-music-screenshot.jpg
  DESTINATION "${SCOPES_DATADIR}/mediascanner-music"
)

install(
  FILES video.png video-scope.png mediascanner-video-screenshot.jpg
  DESTINATION "${SCOPES_DATADIR}/mediascanner-video"
)

install(
  FILES musicaggregator-screenshot.jpg
  DESTINATION "${SCOPES_DATADIR}/musicaggregator"
)

install(
  FILES videoaggregator-screenshot.jpg
  DESTINATION "${SCOPES_DATADIR}/videoaggregator"
)
