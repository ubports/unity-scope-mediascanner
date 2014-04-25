file(GLOB CPPFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
                   "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB INIFILES RELATIVE "${CMAKE_CURRENT_BINARY_DIR}"
                   "${CMAKE_SOURCE_DIR}/data/*.ini.in.in")

#set(EXTRACT_CMD true)
#foreach(inifile ${INIFILES})
#    set(EXTRACT_CMD "${INTLTOOL_EXTRACT_BIN} --type=gettext/ini --local ${inifile} && ${EXTRACT_CMD}")
#endforeach()

#add_custom_target(pot-ini COMMAND ${EXTRACT_CMD})

foreach(inifile ${INIFILES})
    get_filename_component(fname ${inifile} NAME)
    list(APPEND GENERATED_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/tmp/${fname}.h")
endforeach()

add_custom_command(OUTPUT ${GENERATED_HEADERS})
foreach(inifile ${INIFILES})
    add_custom_command(OUTPUT ${GENERATED_HEADERS} COMMAND ${INTLTOOL_EXTRACT_BIN} --type=gettext/ini --local ${inifile} APPEND)
endforeach()

add_custom_target(pot-cpp
    COMMAND ${XGETTEXT_BIN} -o ${GETTEXT_PACKAGE}.pot
                            --copyright=\"Canonical Ltd.\"
                            --package-name ${GETTEXT_PACKAGE}
                            --c++ --add-comments=TRANSLATORS
                            --keyword=_ --keyword=N_ --from-code=UTF-8
                            ${CPPFILES} "${CMAKE_CURRENT_BINARY_DIR}/tmp/*.h"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS ${GENERATED_HEADERS}
)

add_custom_target(pot DEPENDS pot-cpp)

file(GLOB pofiles *.po)
foreach(f ${pofiles})
  get_filename_component(pofile ${f} NAME_WE)
  set(languages "${pofile};${languages}")
endforeach()

MESSAGE("${languages}")

foreach(i ${languages})
  add_custom_command(OUTPUT ${i}.mo
  COMMAND ${MSGFMT_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/${i}.po -o ${CMAKE_CURRENT_BINARY_DIR}/${i}.mo
  DEPENDS ${i}.po
  )
  add_custom_target(${i}gen ALL DEPENDS ${i}.mo)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${i}.mo 
  DESTINATION share/locale/${i}/LC_MESSAGES
  RENAME ${GETTEXT_PACKAGE}.mo)
endforeach()

