
find_program(INTLTOOL_EXTRACT_BIN intltool-extract)
find_program(INTLTOOL_MERGE_BIN intltool-merge)

if(NOT INTLTOOL_MERGE_BIN)
  message(FATAL_ERROR "Intltool binaries not found.")
endif()

add_custom_target(intltool-merge ALL)

macro(intltool_merge _source _dest)
  add_custom_target(
    ${_dest}
    COMMAND ${INTLTOOL_MERGE_BIN} -d ${CMAKE_SOURCE_DIR}/po ${_source} ${_dest}
    DEPENDS ${_source})
  add_dependencies(intltool-merge ${_dest})
endmacro(intltool_merge)
