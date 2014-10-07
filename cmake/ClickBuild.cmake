set(CLICK_MODE OFF CACHE BOOL "Whether to install in click mode")

# Add top level target to build all click packages
add_custom_target(click COMMENT Build click packages)
find_program(click_bin click)

# Determine the architecture for the Click package based on the compiler target.
execute_process(
  COMMAND ${CMAKE_C_COMPILER} -dumpmachine
  OUTPUT_VARIABLE _cc_arch)
set(_click_arch "unknown")
if(${_cc_arch} MATCHES "^x86_64-.*")
  set(_click_arch "amd64")
elseif(${_cc_arch} MATCHES "^i.86-.*")
  set(_click_arch "amd64")
elseif(${_cc_arch} MATCHES "^arm-.*")
  set(_click_arch "armhf")
endif()
set(CLICK_ARCH "${_click_arch}" CACHE STRING "Click package architecture")

macro(click_build _pkg _version _manifest _apparmor)
  set(_builddir ${CMAKE_INSTALL_PREFIX}/${_pkg})

  add_custom_target(
    ${_pkg}.click
    COMMAND ${click_bin} build ${_builddir}
    WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})
  set_target_properties(${_pkg}.click PROPERTIES
    OUTPUT_NAME ${_pkg}_${_version}_${CLICK_ARCH}.click
    CLICK_DIR ${_builddir})
  add_dependencies(click ${_pkg}.click)

  install(
    FILES ${_manifest}
    DESTINATION "${_builddir}"
    RENAME manifest.json
    COMPONENT "${_pkg}")
  install(
    FILES ${_apparmor}
    DESTINATION "${_builddir}"
    RENAME apparmor.json
    COMPONENT "${_pkg}")
endmacro(click_build)
