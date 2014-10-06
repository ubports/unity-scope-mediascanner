include_directories(${UNITY_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_library(videoaggregator MODULE
  videoaggregatorscope.cpp
  videoaggregatorquery.cpp)
target_link_libraries(videoaggregator aggregator-utils ${UNITY_SCOPES_LDFLAGS})

configure_file(manifest.json.in manifest.json)
intltool_merge(${CMAKE_CURRENT_SOURCE_DIR}/videoaggregator.ini.in videoaggregator.ini)
intltool_merge(${CMAKE_CURRENT_SOURCE_DIR}/videoaggregator-settings.ini.in videoaggregator-settings.ini)

click_build(com.ubuntu.scopes.video ${VERSION}
  ${CMAKE_CURRENT_BINARY_DIR}/manifest.json apparmor.json)
click_scope(com.ubuntu.scopes.video videoaggregator
  videoaggregator
  ${CMAKE_CURRENT_BINARY_DIR}/videoaggregator.ini
  ${CMAKE_CURRENT_BINARY_DIR}/videoaggregator-settings.ini)
click_scope_data(com.ubuntu.scopes.video videoaggregator
  screenshot.jpg
  icon.png)
