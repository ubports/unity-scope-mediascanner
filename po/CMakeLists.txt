file(GLOB CPPFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
                   "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB INIFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
                   "${CMAKE_SOURCE_DIR}/data/*.ini.in.in")

foreach(inifile ${INIFILES})
    get_filename_component(fname ${inifile} NAME)
    set(iniheaderfile "${CMAKE_CURRENT_SOURCE_DIR}/tmp/${fname}.h")
    list(APPEND GENERATED_HEADERS ${iniheaderfile})
    add_custom_command(OUTPUT ${iniheaderfile}
        COMMAND ${INTLTOOL_EXTRACT_BIN} --type=gettext/ini --local ${inifile}
        DEPENDS ${inifile}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endforeach()

add_custom_target(pot-ini
    DEPENDS ${GENERATED_HEADERS}
)

add_custom_target(pot
    COMMAND ${XGETTEXT_BIN} -o ${GETTEXT_PACKAGE}.pot
                            --copyright=\"Canonical Ltd.\"
                            --package-name ${GETTEXT_PACKAGE}
                            --c++ --add-comments=TRANSLATORS
                            --keyword=_ --keyword=N_ --from-code=UTF-8
                            ${CPPFILES} "tmp/*.h"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS pot-ini
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locale DESTINATION share)

file(GLOB pofiles *.po)
foreach(f ${pofiles})
  get_filename_component(pofile ${f} NAME_WE)
  set(languages "${pofile};${languages}")
endforeach()

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

