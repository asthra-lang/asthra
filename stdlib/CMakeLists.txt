# Asthra Standard Library
# The Asthra standard library consists of .asthra source files, not C files
# We don't need to compile these into a C library - they are Asthra language files

# Discover all Asthra standard library files
file(GLOB_RECURSE ASTHRA_STDLIB_FILES "*.asthra")

# Create a custom target to track stdlib files (for IDE integration)
if(ASTHRA_STDLIB_FILES)
    add_custom_target(asthra_stdlib_files
        SOURCES ${ASTHRA_STDLIB_FILES}
        COMMENT "Asthra Standard Library Files"
    )
    
    message(STATUS "Found ${CMAKE_CURRENT_LIST_LENGTH} Asthra stdlib files")
else()
    message(STATUS "No Asthra stdlib files found")
endif()

# Note: The actual stdlib "compilation" happens when the Asthra compiler
# processes .asthra files, not during the CMake build process

# Install Asthra standard library files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/asthra/stdlib
    FILES_MATCHING PATTERN "*.asthra"
) 