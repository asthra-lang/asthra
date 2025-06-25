# Main build targets and configurations

# Install configuration
# Install compiler
install(TARGETS asthra
    RUNTIME DESTINATION bin
)

# Install libraries
install(TARGETS asthra_compiler asthra_runtime
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# Install headers
install(DIRECTORY src/
    DESTINATION include/asthra
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY runtime/
    DESTINATION include/asthra/runtime
    FILES_MATCHING PATTERN "*.h"
)

# Install standard library
install(DIRECTORY stdlib/
    DESTINATION share/asthra/stdlib
    FILES_MATCHING PATTERN "*.asthra"
)

# Install documentation
install(DIRECTORY docs/
    DESTINATION share/doc/asthra
    FILES_MATCHING PATTERN "*.md"
)

# Create package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    cmake/AshtraConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/AshtraConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Asthra
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/AshtraConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AshtraConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AshtraConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Asthra
)

# Package generation is now handled by Installation.cmake

# Platform-specific packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Asthra Programming Language")
    set(CPACK_NSIS_PACKAGE_NAME "Asthra")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/asthra-lang/asthra")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
    set(CPACK_DMG_VOLUME_NAME "Asthra")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Asthra Project")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libjson-c5")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc, json-c")
endif() 