# CMake build script

SET (WT_CONNECTOR "wthttp" CACHE STRING "Connector for wt (wthttp or wtfcgi)")

SET (HC_SOURCES
  3rdparty/serial.cpp
  main.cpp
  hc_wgt.cpp
  hc_serial.cpp)

SET (HC_HEADERS
  3rdparty/serial.h
  hc_wgt.hpp
  hc_serial.hpp)

SET (WT_PROJECT_TARGET hc_001.wt)

ADD_EXECUTABLE (${WT_PROJECT_TARGET} ${HC_SOURCES} ${HC_HEADERS})
ADD_DEFINITIONS (-std=c++11)

TARGET_LINK_LIBRARIES (${WT_PROJECT_TARGET} ${WT_CONNECTOR} wt boost_system)
