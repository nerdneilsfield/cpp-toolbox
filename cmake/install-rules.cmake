if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/cpp-toolbox-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package cpp-toolbox)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT cpp-toolbox_Development
)

install(
    TARGETS cpp-toolbox_cpp-toolbox
    EXPORT cpp-toolboxTargets
    RUNTIME #
    COMPONENT cpp-toolbox_Runtime
    LIBRARY #
    COMPONENT cpp-toolbox_Runtime
    NAMELINK_COMPONENT cpp-toolbox_Development
    ARCHIVE #
    COMPONENT cpp-toolbox_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    cpp-toolbox_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE cpp-toolbox_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(cpp-toolbox_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${cpp-toolbox_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT cpp-toolbox_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${cpp-toolbox_INSTALL_CMAKEDIR}"
    COMPONENT cpp-toolbox_Development
)

install(
    EXPORT cpp-toolboxTargets
    NAMESPACE cpp-toolbox::
    DESTINATION "${cpp-toolbox_INSTALL_CMAKEDIR}"
    COMPONENT cpp-toolbox_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
