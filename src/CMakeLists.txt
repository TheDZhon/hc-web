# CMake build script

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

SET (WT_PROJECT_TARGET hc_001.wt)

ADD_EXECUTABLE (${WT_PROJECT_TARGET} ${HC_SOURCES} ${HC_HEADERS})
ADD_DEFINITIONS (-std=c++11)

TARGET_LINK_LIBRARIES (${WT_PROJECT_TARGET} ${WT_CONNECTOR} wt boost_system)
