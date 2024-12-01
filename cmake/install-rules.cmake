if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/cpp-toolbox-new-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package cpp-toolbox-new)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT cpp-toolbox-new_Development
)

install(
    TARGETS cpp-toolbox-new_cpp-toolbox-new
    EXPORT cpp-toolbox-newTargets
    RUNTIME #
    COMPONENT cpp-toolbox-new_Runtime
    LIBRARY #
    COMPONENT cpp-toolbox-new_Runtime
    NAMELINK_COMPONENT cpp-toolbox-new_Development
    ARCHIVE #
    COMPONENT cpp-toolbox-new_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    cpp-toolbox-new_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE cpp-toolbox-new_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(cpp-toolbox-new_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${cpp-toolbox-new_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT cpp-toolbox-new_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${cpp-toolbox-new_INSTALL_CMAKEDIR}"
    COMPONENT cpp-toolbox-new_Development
)

install(
    EXPORT cpp-toolbox-newTargets
    NAMESPACE cpp-toolbox-new::
    DESTINATION "${cpp-toolbox-new_INSTALL_CMAKEDIR}"
    COMPONENT cpp-toolbox-new_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
