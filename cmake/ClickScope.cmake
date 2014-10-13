macro(click_scope _pkg _scopename _library _config)
  set(_fullname ${_pkg}_${_scopename})
  get_target_property(_builddir ${_pkg}.click CLICK_DIR)

  set_target_properties(${_library} PROPERTIES
    OUTPUT_NAME "${_fullname}")
  install(
    TARGETS ${_library}
    DESTINATION "${_builddir}/${_scopename}"
    COMPONENT "${_pkg}")
  install(
    FILES ${_config}
    DESTINATION "${_builddir}/${_scopename}"
    RENAME "${_fullname}.ini"
    COMPONENT "${_pkg}")

  if(${ARGC} EQUAL 5)
    set(_settings ${ARGV4})
    install(
      FILES ${_settings}
      DESTINATION "${_builddir}/${_scopename}"
      RENAME "${_fullname}-settings.ini"
      COMPONENT "${_pkg}")
  endif()
endmacro(click_scope)

macro(click_scope_data _pkg _scopename)
  get_target_property(_builddir ${_pkg}.click CLICK_DIR)
  install(
    FILES ${ARGN}
    DESTINATION "${_builddir}/${_scopename}"
    COMPONENT "${_pkg}")
endmacro(click_scope_data)
