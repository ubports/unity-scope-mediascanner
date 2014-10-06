include_directories(${UNITY_INCLUDE_DIRS})

add_subdirectory(utils)

add_library(musicaggregator SHARED
musicaggregatorscope.cpp
musicaggregatorquery.cpp
onlinemusicresultforwarder.cpp)
target_link_libraries(musicaggregator aggregator-utils ${UNITY_SCOPES_LDFLAGS})

add_library(videoaggregator SHARED
videoaggregatorscope.cpp
videoaggregatorquery.cpp)
target_link_libraries(videoaggregator aggregator-utils ${UNITY_SCOPES_LDFLAGS})

install(TARGETS musicaggregator
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/musicaggregator")

  install(TARGETS videoaggregator
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/videoaggregator")
  
add_subdirectory(mymusic)
add_subdirectory(myvideos)
