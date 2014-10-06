include_directories(${UNITY_INCLUDE_DIRS})

add_library(musicaggregator MODULE
  musicaggregatorscope.cpp
  musicaggregatorquery.cpp
  onlinemusicresultforwarder.cpp)
target_link_libraries(musicaggregator aggregator-utils ${UNITY_SCOPES_LDFLAGS})

configure_file(manifest.json.in manifest.json)
intltool_merge(${CMAKE_CURRENT_SOURCE_DIR}/musicaggregator.ini.in musicaggregator.ini)
intltool_merge(${CMAKE_CURRENT_SOURCE_DIR}/musicaggregator-settings.ini.in musicaggregator-settings.ini)

click_build(com.ubuntu.scopes.music ${VERSION}
  ${CMAKE_CURRENT_BINARY_DIR}/manifest.json apparmor.json)
click_scope(com.ubuntu.scopes.music musicaggregator
  musicaggregator
  ${CMAKE_CURRENT_BINARY_DIR}/musicaggregator.ini
  ${CMAKE_CURRENT_BINARY_DIR}/musicaggregator-settings.ini)
click_scope_data(com.ubuntu.scopes.music musicaggregator
  screenshot.jpg
  icon.png)
