include_directories(${UNITY_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

add_library(musicaggregator SHARED
musicaggregatorscope.cpp
musicaggregatorquery.cpp
resultforwarder.cpp
onlinemusicresultforwarder.cpp
bufferedresultforwarder.cpp
notify-strategy.cpp)
target_link_libraries(musicaggregator ${UNITY_SCOPES_LDFLAGS})

add_library(videoaggregator SHARED
videoaggregatorscope.cpp
videoaggregatorquery.cpp
resultforwarder.cpp
bufferedresultforwarder.cpp
notify-strategy.cpp)
target_link_libraries(videoaggregator ${UNITY_SCOPES_LDFLAGS})

install(TARGETS musicaggregator
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/musicaggregator")

  install(TARGETS videoaggregator
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/videoaggregator")
  
add_subdirectory(mymusic)
