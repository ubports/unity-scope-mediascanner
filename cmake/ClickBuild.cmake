add_custom_target(click COMMENT Build click packages)
find_program(click_bin click)

macro(click_build _pkg _version _arch _manifest _apparmor)
  set(_builddir ${CMAKE_CURRENT_BINARY_DIR}/${_pkg})

  add_custom_target(
    ${_pkg}.click
    COMMAND ${click_bin} build ${_builddir})
  set_target_properties(${_pkg}.click
    PROPERTIES OUTPUT_NAME ${_pkg}_${_version}_${_arch}.click)
  add_dependencies(click ${_pkg}.click)

  add_custom_target(
    ${_pkg}.builddir
    COMMAND mkdir ${_builddir}
    COMMAND cp ${_manifest} ${_builddir}/manifest.json
    COMMAND cp ${_apparmor} ${_builddir}/apparmor.json)
  add_dependencies(${_pkg}.click ${_pkg}.builddir)
endmacro(click_build)
