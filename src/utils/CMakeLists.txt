include_directories(${UNITY_INCLUDE_DIRS})

add_definitions(-fPIC)

add_library(scope-utils STATIC
  resultforwarder.cpp
  bufferedresultforwarder.cpp
  notify-strategy.cpp
  i18n.cpp)

target_link_libraries(scope-utils ${UNITY_SCOPES_LDFLAGS})
