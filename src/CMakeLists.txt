# CMake build script

# --> Boost

IF (WIN32)
  SET (Boost_USE_STATIC_LIBS ON)
ENDIF (WIN32)

SET (Boost_USE_MULTITHREADED ON)
FIND_PACKAGE (Boost COMPONENTS system REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

# --> Threads support

FIND_PACKAGE (Threads)

SET (WT_CONNECTOR "wthttp" CACHE STRING "Connector for wt (wthttp or wtfcgi)")

SET (HC_SOURCES
  main.cpp
  hc_wgt.cpp
  hc_cntl.cpp
  hc_master.cpp)

SET (HC_HEADERS
  hc_wgt.hpp
  hc_cntl.hpp
  hc_master.hpp)

SET (WT_PROJECT_TARGET hc-web.wt)

# Additional definitions

IF(WIN32)
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x0502)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
ENDIF(WIN32)

ADD_EXECUTABLE (${WT_PROJECT_TARGET} ${HC_SOURCES} ${HC_HEADERS})

TARGET_LINK_LIBRARIES (${WT_PROJECT_TARGET} 
  ${WT_CONNECTOR} wt 
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})
