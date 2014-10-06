include_directories(${UNITY_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

#add_library(mediascanner-music MODULE music-scope.cpp)
##set_target_properties(mediascanner-music PROPERTIES PREFIX "")
#target_link_libraries(mediascanner-music ${UNITY_LDFLAGS})

add_library(mediascanner-video MODULE video-scope.cpp)
#set_target_properties(mediascanner-video PROPERTIES PREFIX "")
target_link_libraries(mediascanner-video ${UNITY_LDFLAGS} ${Boost_LIBRARIES})

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

#install(
#  TARGETS mediascanner-music
#  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/mediascanner-music")

install(
  TARGETS mediascanner-video
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/mediascanner-video")

install(TARGETS musicaggregator
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/musicaggregator")

  install(TARGETS videoaggregator
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/unity-scopes/videoaggregator")
  
add_subdirectory(mymusic)
