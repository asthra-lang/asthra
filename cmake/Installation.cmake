# Installation and Distribution for Asthra
# Provides install, uninstall, and source distribution capabilities

include(CMakePackageConfigHelpers)

# Installation directories
set(ASTHRA_INSTALL_BIN_DIR "bin" CACHE PATH "Installation directory for executables")
set(ASTHRA_INSTALL_LIB_DIR "lib" CACHE PATH "Installation directory for libraries")
set(ASTHRA_INSTALL_INCLUDE_DIR "include/asthra" CACHE PATH "Installation directory for headers")
set(ASTHRA_INSTALL_DATA_DIR "share/asthra" CACHE PATH "Installation directory for data files")
set(ASTHRA_INSTALL_DOC_DIR "share/doc/asthra" CACHE PATH "Installation directory for documentation")
set(ASTHRA_INSTALL_MAN_DIR "share/man" CACHE PATH "Installation directory for man pages")

# Export targets
install(EXPORT AsthraTargets
    FILE AsthraTargets.cmake
    NAMESPACE Asthra::
    DESTINATION ${ASTHRA_INSTALL_LIB_DIR}/cmake/Asthra
)

# Create version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/AsthraConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Create config file
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/AsthraConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/AsthraConfig.cmake"
    @ONLY
)

# Install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AsthraConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AsthraConfigVersion.cmake"
    DESTINATION ${ASTHRA_INSTALL_LIB_DIR}/cmake/Asthra
)

# Install headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${ASTHRA_INSTALL_INCLUDE_DIR}
    FILES_MATCHING PATTERN "*.h"
    PATTERN "internal" EXCLUDE
    PATTERN "private" EXCLUDE
)

# Install runtime headers
install(DIRECTORY ${CMAKE_SOURCE_DIR}/runtime/
    DESTINATION ${ASTHRA_INSTALL_INCLUDE_DIR}/runtime
    FILES_MATCHING PATTERN "*.h"
    PATTERN "*_internal.h" EXCLUDE
    PATTERN "*_private.h" EXCLUDE
)

# Install data files
install(FILES
    ${CMAKE_SOURCE_DIR}/grammar.txt
    DESTINATION ${ASTHRA_INSTALL_DATA_DIR}
)

# Install documentation
install(FILES
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/LICENSE
    ${CMAKE_SOURCE_DIR}/CLAUDE.md
    DESTINATION ${ASTHRA_INSTALL_DOC_DIR}
)

# Install examples if they exist
if(EXISTS ${CMAKE_SOURCE_DIR}/examples)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/examples/
        DESTINATION ${ASTHRA_INSTALL_DATA_DIR}/examples
        PATTERN "*.ast"
        PATTERN "README.md"
    )
endif()

# Install standard library files
if(EXISTS ${CMAKE_SOURCE_DIR}/stdlib)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/stdlib/
        DESTINATION ${ASTHRA_INSTALL_DATA_DIR}/stdlib
        FILES_MATCHING 
        PATTERN "*.ast"
        PATTERN "*.asth"
    )
endif()

# Create uninstall target
if(NOT TARGET uninstall)
    # Create uninstall script
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
    )
    
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
        COMMENT "Uninstalling Asthra"
    )
endif()

# Create cmake_uninstall.cmake.in if it doesn't exist
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in")
    file(WRITE "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
"# Uninstall script for Asthra

if(NOT EXISTS \"@CMAKE_CURRENT_BINARY_DIR@/install_manifest.txt\")
    message(FATAL_ERROR \"Cannot find install manifest: @CMAKE_CURRENT_BINARY_DIR@/install_manifest.txt\")
endif()

file(READ \"@CMAKE_CURRENT_BINARY_DIR@/install_manifest.txt\" files)
string(REGEX REPLACE \"\\n\" \";\" files \"\${files}\")

foreach(file \${files})
    message(STATUS \"Uninstalling \$ENV{DESTDIR}\${file}\")
    if(IS_SYMLINK \"\$ENV{DESTDIR}\${file}\" OR EXISTS \"\$ENV{DESTDIR}\${file}\")
        exec_program(
            \"@CMAKE_COMMAND@\" ARGS \"-E remove \\\"\$ENV{DESTDIR}\${file}\\\"\"
            OUTPUT_VARIABLE rm_out
            RETURN_VALUE rm_retval
        )
        if(NOT \"\${rm_retval}\" STREQUAL 0)
            message(FATAL_ERROR \"Problem when removing \$ENV{DESTDIR}\${file}\")
        endif()
    else()
        message(STATUS \"File \$ENV{DESTDIR}\${file} does not exist.\")
    endif()
endforeach()
")
endif()

# Create AsthraConfig.cmake.in if it doesn't exist
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/cmake/AsthraConfig.cmake.in")
    file(WRITE "${CMAKE_SOURCE_DIR}/cmake/AsthraConfig.cmake.in"
"# AsthraConfig.cmake - Config file for the Asthra package

@PACKAGE_INIT@

include(\"\${CMAKE_CURRENT_LIST_DIR}/AsthraTargets.cmake\")

# Provide old-style variables
set(ASTHRA_INCLUDE_DIRS \"@PACKAGE_CMAKE_INSTALL_INCLUDEDIR@/asthra\")
set(ASTHRA_LIBRARIES Asthra::asthra Asthra::asthra_runtime)
set(ASTHRA_FOUND TRUE)

# Check required components
check_required_components(Asthra)
")
endif()

# Source distribution
set(CPACK_PACKAGE_NAME "asthra")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Asthra Programming Language - AI-Friendly Systems Programming")
set(CPACK_PACKAGE_VENDOR "Asthra Project")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/asthra-lang/asthra")

# Source package configuration
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
    "/\\\\.git/"
    "/\\\\.github/"
    "/build.*/"
    "/\\\\.vscode/"
    "/\\\\.idea/"
    "\\\\.swp$"
    "\\\\.DS_Store"
    "~$"
    "/CMakeCache\\\\.txt$"
    "/CMakeFiles/"
    "/cmake_install\\\\.cmake$"
    "/CTestTestfile\\\\.cmake$"
    "/Makefile$"
    "\\\\.o$"
    "\\\\.a$"
    "\\\\.so$"
    "\\\\.dylib$"
)

include(CPack)

# Distribution target
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} --build . --target package_source
    COMMENT "Creating source distribution"
)

# Binary distribution
add_custom_target(dist-binary
    COMMAND ${CMAKE_COMMAND} --build . --target package
    COMMENT "Creating binary distribution"
)

# Installation summary
add_custom_target(install-summary
    COMMAND ${CMAKE_COMMAND} -E echo "=== Asthra Installation Summary ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Executables: ${CMAKE_INSTALL_PREFIX}/${ASTHRA_INSTALL_BIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Libraries: ${CMAKE_INSTALL_PREFIX}/${ASTHRA_INSTALL_LIB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Headers: ${CMAKE_INSTALL_PREFIX}/${ASTHRA_INSTALL_INCLUDE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Data: ${CMAKE_INSTALL_PREFIX}/${ASTHRA_INSTALL_DATA_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Documentation: ${CMAKE_INSTALL_PREFIX}/${ASTHRA_INSTALL_DOC_DIR}"
    COMMENT "Display installation paths"
)

# Install help
add_custom_target(install-help
    COMMAND ${CMAKE_COMMAND} -E echo "Installation Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  install             - Install Asthra"
    COMMAND ${CMAKE_COMMAND} -E echo "  uninstall           - Uninstall Asthra"
    COMMAND ${CMAKE_COMMAND} -E echo "  install-summary     - Show install paths"
    COMMAND ${CMAKE_COMMAND} -E echo "  dist                - Create source package"
    COMMAND ${CMAKE_COMMAND} -E echo "  dist-binary         - Create binary package"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Installation Directories:"
    COMMAND ${CMAKE_COMMAND} -E echo "  CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Binaries: ${ASTHRA_INSTALL_BIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Libraries: ${ASTHRA_INSTALL_LIB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Headers: ${ASTHRA_INSTALL_INCLUDE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Usage:"
    COMMAND ${CMAKE_COMMAND} -E echo "  cmake --install . --prefix /usr/local"
    COMMAND ${CMAKE_COMMAND} -E echo "  cmake --build . --target uninstall"
    COMMENT "Installation help"
)