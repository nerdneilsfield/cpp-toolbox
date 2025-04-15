include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

# 如果支持LTO，则为整个项目启用
if(supported)
  message(STATUS "IPO / LTO enabled")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(STATUS "IPO / LTO not supported: ${error}")
endif()