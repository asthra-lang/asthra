# Build Types and Configurations for Asthra
# Provides debug, release, profile, and PGO build configurations

# Define custom build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile;PGO-Generate;PGO-Use" CACHE STRING "" FORCE)

# Common flags for all builds
set(COMMON_C_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable")
set(COMMON_C_FLAGS "${COMMON_C_FLAGS} -Wno-sign-compare -Wno-unused-function")

# Platform-specific PIC and threading flags
set(COMMON_C_FLAGS "${COMMON_C_FLAGS} -fPIC -pthread")

# Platform-specific common flags
if(APPLE)
    set(COMMON_C_FLAGS "${COMMON_C_FLAGS} -mmacosx-version-min=10.15")
elseif(UNIX AND NOT APPLE)
    # No additional flags for Linux
endif()

# Debug configuration
set(CMAKE_C_FLAGS_DEBUG "${COMMON_C_FLAGS} -g3 -O0 -DDEBUG -DASTHRA_DEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fstack-protector-strong")

# Release configuration
set(CMAKE_C_FLAGS_RELEASE "${COMMON_C_FLAGS} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native -mtune=native")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fomit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto")

# Profile configuration
set(CMAKE_C_FLAGS_PROFILE "${COMMON_C_FLAGS} -O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_PROFILE} -pg")

# PGO Generate configuration
set(CMAKE_C_FLAGS_PGO-GENERATE "${COMMON_C_FLAGS} -O2 -fprofile-generate")
set(CMAKE_C_FLAGS_PGO-GENERATE "${CMAKE_C_FLAGS_PGO-GENERATE} -fprofile-dir=${CMAKE_BINARY_DIR}/pgo-data")

# PGO Use configuration
set(CMAKE_C_FLAGS_PGO-USE "${COMMON_C_FLAGS} -O3 -fprofile-use")
set(CMAKE_C_FLAGS_PGO-USE "${CMAKE_C_FLAGS_PGO-USE} -fprofile-dir=${CMAKE_BINARY_DIR}/pgo-data")
set(CMAKE_C_FLAGS_PGO-USE "${CMAKE_C_FLAGS_PGO-USE} -fprofile-correction")

# Linker flags for each configuration
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")
if(APPLE)
    # macOS: -s is obsolete, use -Wl,-x for stripping local symbols only
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O3 -flto -Wl,-x")
else()
    # Linux/Unix: use traditional -s for stripping
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O3 -flto -Wl,-s")
endif()
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "-pg")
set(CMAKE_EXE_LINKER_FLAGS_PGO-GENERATE "-fprofile-generate")
set(CMAKE_EXE_LINKER_FLAGS_PGO-USE "-fprofile-use -flto")

# Custom targets for easy build type switching
add_custom_target(debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build . --config Debug
    COMMENT "Building Debug configuration"
)

add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build . --config Release
    COMMENT "Building Release configuration"
)

add_custom_target(profile
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Profile ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build . --config Profile
    COMMENT "Building Profile configuration"
)

# PGO workflow targets
add_custom_target(pgo-generate
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=PGO-Generate ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build . --config PGO-Generate
    COMMENT "Building PGO generation phase"
)

add_custom_target(pgo-run
    COMMAND ${CMAKE_COMMAND} -E echo "Running PGO training workload..."
    COMMAND ${CMAKE_BINARY_DIR}/bin/asthra --version
    COMMAND ${CMAKE_CTEST_COMMAND} -L "basic|parser|lexer" --output-on-failure
    COMMENT "Running PGO training workload"
)

add_custom_target(pgo-use
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=PGO-Use ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build . --config PGO-Use
    COMMENT "Building PGO optimized version"
)

add_custom_target(pgo
    COMMAND ${CMAKE_COMMAND} -E echo "Starting complete PGO workflow..."
    COMMAND ${CMAKE_COMMAND} --build . --target pgo-generate
    COMMAND ${CMAKE_COMMAND} --build . --target pgo-run
    COMMAND ${CMAKE_COMMAND} --build . --target pgo-use
    COMMENT "Complete PGO workflow"
)

# Function to set build-specific defines
function(asthra_configure_build_target target)
    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:ASTHRA_DEBUG>
        $<$<CONFIG:Debug>:ASTHRA_ENABLE_ASSERTIONS>
        $<$<CONFIG:Profile>:ASTHRA_PROFILE>
        $<$<CONFIG:PGO-Generate>:ASTHRA_PGO_GENERATE>
        $<$<CONFIG:PGO-Use>:ASTHRA_PGO_USE>
    )
endfunction()