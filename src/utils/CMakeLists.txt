include_directories(${UNITY_INCLUDE_DIRS})

add_definitions(-fPIC)

add_library(aggregator-utils STATIC
  resultforwarder.cpp
  bufferedresultforwarder.cpp
  notify-strategy.cpp)
