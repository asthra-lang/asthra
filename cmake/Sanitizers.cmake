# Sanitizer support for Asthra
# Provides ASan, TSan, UBSan, MSan and combined sanitizer configurations

# Include guard to prevent duplicate inclusion
if(ASTHRA_SANITIZERS_INCLUDED)
    return()
endif()
set(ASTHRA_SANITIZERS_INCLUDED TRUE)

include(CheckCCompilerFlag)

# Check for sanitizer support (Clang)
function(check_sanitizer_support sanitizer flag var)
    check_c_compiler_flag(${flag} ${var})
    if(${var})
        message(STATUS "Sanitizer ${sanitizer} is supported")
    else()
        message(STATUS "Sanitizer ${sanitizer} is NOT supported")
    endif()
endfunction()

# Check available sanitizers
check_sanitizer_support("AddressSanitizer" "-fsanitize=address" ASAN_SUPPORTED)
check_sanitizer_support("ThreadSanitizer" "-fsanitize=thread" TSAN_SUPPORTED)
check_sanitizer_support("UndefinedBehaviorSanitizer" "-fsanitize=undefined" UBSAN_SUPPORTED)
check_sanitizer_support("MemorySanitizer" "-fsanitize=memory" MSAN_SUPPORTED)

# Common sanitizer flags
set(SANITIZER_COMMON_FLAGS "-fno-omit-frame-pointer -g")

# Address Sanitizer configuration
set(ASAN_FLAGS "-fsanitize=address ${SANITIZER_COMMON_FLAGS}")
set(ASAN_FLAGS "${ASAN_FLAGS} -fsanitize-address-use-after-scope")
set(ASAN_LINK_FLAGS "-fsanitize=address")

# Thread Sanitizer configuration
set(TSAN_FLAGS "-fsanitize=thread ${SANITIZER_COMMON_FLAGS}")
set(TSAN_LINK_FLAGS "-fsanitize=thread")

# Undefined Behavior Sanitizer configuration
set(UBSAN_FLAGS "-fsanitize=undefined ${SANITIZER_COMMON_FLAGS}")
set(UBSAN_FLAGS "${UBSAN_FLAGS} -fno-sanitize-recover=all")
set(UBSAN_FLAGS "${UBSAN_FLAGS} -fsanitize=float-divide-by-zero")
set(UBSAN_FLAGS "${UBSAN_FLAGS} -fsanitize=unsigned-integer-overflow")
set(UBSAN_LINK_FLAGS "-fsanitize=undefined")

# Memory Sanitizer configuration
set(MSAN_FLAGS "-fsanitize=memory ${SANITIZER_COMMON_FLAGS}")
set(MSAN_FLAGS "${MSAN_FLAGS} -fsanitize-memory-track-origins")
set(MSAN_LINK_FLAGS "-fsanitize=memory")

# Combined sanitizers
set(ASAN_UBSAN_FLAGS "-fsanitize=address,undefined ${SANITIZER_COMMON_FLAGS}")
set(ASAN_UBSAN_LINK_FLAGS "-fsanitize=address,undefined")

# Function to create sanitizer build variant
function(create_sanitizer_target target_name sanitizer_flags link_flags)
    if(TARGET asthra)
        add_custom_target(${target_name}
            COMMAND ${CMAKE_COMMAND} -E echo "Building with ${target_name}..."
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${target_name}
            COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}/${target_name}
                    -DCMAKE_C_FLAGS="${sanitizer_flags}"
                    -DCMAKE_EXE_LINKER_FLAGS="${link_flags}"
                    -DCMAKE_BUILD_TYPE=Debug
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/${target_name}
            COMMENT "Building ${target_name} variant"
        )
    endif()
endfunction()

# Create individual sanitizer targets
if(ASAN_SUPPORTED)
    create_sanitizer_target(asan "${ASAN_FLAGS}" "${ASAN_LINK_FLAGS}")
    
    add_custom_target(test-asan
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/asan --target test-all
        DEPENDS asan
        COMMENT "Running tests with AddressSanitizer"
    )
endif()

if(TSAN_SUPPORTED)
    create_sanitizer_target(tsan "${TSAN_FLAGS}" "${TSAN_LINK_FLAGS}")
    
    add_custom_target(test-tsan
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/tsan --target test-all
        DEPENDS tsan
        COMMENT "Running tests with ThreadSanitizer"
    )
endif()

if(UBSAN_SUPPORTED)
    create_sanitizer_target(ubsan "${UBSAN_FLAGS}" "${UBSAN_LINK_FLAGS}")
    
    add_custom_target(test-ubsan
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/ubsan --target test-all
        DEPENDS ubsan
        COMMENT "Running tests with UndefinedBehaviorSanitizer"
    )
endif()

if(MSAN_SUPPORTED)
    create_sanitizer_target(msan "${MSAN_FLAGS}" "${MSAN_LINK_FLAGS}")
    
    add_custom_target(test-msan
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/msan --target test-all
        DEPENDS msan
        COMMENT "Running tests with MemorySanitizer"
    )
endif()

# Combined sanitizer targets
if(ASAN_SUPPORTED AND UBSAN_SUPPORTED)
    create_sanitizer_target(asan-ubsan "${ASAN_UBSAN_FLAGS}" "${ASAN_UBSAN_LINK_FLAGS}")
    
    add_custom_target(test-asan-ubsan
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/asan-ubsan --target test-all
        DEPENDS asan-ubsan
        COMMENT "Running tests with AddressSanitizer + UndefinedBehaviorSanitizer"
    )
endif()

# All sanitizers target
add_custom_target(sanitize
    COMMENT "Running all available sanitizers"
)

if(ASAN_SUPPORTED AND TARGET asan)
    add_dependencies(sanitize asan)
endif()
if(TSAN_SUPPORTED AND TARGET tsan)
    add_dependencies(sanitize tsan)
endif()
if(UBSAN_SUPPORTED AND TARGET ubsan)
    add_dependencies(sanitize ubsan)
endif()
if(MSAN_SUPPORTED AND TARGET msan)
    add_dependencies(sanitize msan)
endif()

# Test all sanitizers
add_custom_target(test-sanitize
    COMMENT "Running tests with all available sanitizers"
)

if(ASAN_SUPPORTED AND TARGET test-asan)
    add_dependencies(test-sanitize test-asan)
endif()
if(TSAN_SUPPORTED AND TARGET test-tsan)
    add_dependencies(test-sanitize test-tsan)
endif()
if(UBSAN_SUPPORTED AND TARGET test-ubsan)
    add_dependencies(test-sanitize test-ubsan)
endif()
if(MSAN_SUPPORTED AND TARGET test-msan)
    add_dependencies(test-sanitize test-msan)
endif()

# Function to enable sanitizer for a specific target
function(enable_sanitizer_for_target target sanitizer)
    if(${sanitizer} STREQUAL "asan" AND ASAN_SUPPORTED)
        target_compile_options(${target} PRIVATE ${ASAN_FLAGS})
        target_link_options(${target} PRIVATE ${ASAN_LINK_FLAGS})
    elseif(${sanitizer} STREQUAL "tsan" AND TSAN_SUPPORTED)
        target_compile_options(${target} PRIVATE ${TSAN_FLAGS})
        target_link_options(${target} PRIVATE ${TSAN_LINK_FLAGS})
    elseif(${sanitizer} STREQUAL "ubsan" AND UBSAN_SUPPORTED)
        target_compile_options(${target} PRIVATE ${UBSAN_FLAGS})
        target_link_options(${target} PRIVATE ${UBSAN_LINK_FLAGS})
    elseif(${sanitizer} STREQUAL "msan" AND MSAN_SUPPORTED)
        target_compile_options(${target} PRIVATE ${MSAN_FLAGS})
        target_link_options(${target} PRIVATE ${MSAN_LINK_FLAGS})
    elseif(${sanitizer} STREQUAL "asan-ubsan" AND ASAN_SUPPORTED AND UBSAN_SUPPORTED)
        target_compile_options(${target} PRIVATE ${ASAN_UBSAN_FLAGS})
        target_link_options(${target} PRIVATE ${ASAN_UBSAN_LINK_FLAGS})
    endif()
endfunction()

# Option to enable sanitizer for current build
set(ASTHRA_SANITIZER "none" CACHE STRING "Enable sanitizer for build (none|asan|tsan|ubsan|msan|asan-ubsan)")
set_property(CACHE ASTHRA_SANITIZER PROPERTY STRINGS none asan tsan ubsan msan asan-ubsan)

# Apply sanitizer to all targets if requested
if(NOT ASTHRA_SANITIZER STREQUAL "none")
    message(STATUS "Building with sanitizer: ${ASTHRA_SANITIZER}")
    
    if(ASTHRA_SANITIZER STREQUAL "asan" AND ASAN_SUPPORTED)
        add_compile_options(${ASAN_FLAGS})
        add_link_options(${ASAN_LINK_FLAGS})
    elseif(ASTHRA_SANITIZER STREQUAL "tsan" AND TSAN_SUPPORTED)
        add_compile_options(${TSAN_FLAGS})
        add_link_options(${TSAN_LINK_FLAGS})
    elseif(ASTHRA_SANITIZER STREQUAL "ubsan" AND UBSAN_SUPPORTED)
        add_compile_options(${UBSAN_FLAGS})
        add_link_options(${UBSAN_LINK_FLAGS})
    elseif(ASTHRA_SANITIZER STREQUAL "msan" AND MSAN_SUPPORTED)
        add_compile_options(${MSAN_FLAGS})
        add_link_options(${MSAN_LINK_FLAGS})
    elseif(ASTHRA_SANITIZER STREQUAL "asan-ubsan" AND ASAN_SUPPORTED AND UBSAN_SUPPORTED)
        add_compile_options(${ASAN_UBSAN_FLAGS})
        add_link_options(${ASAN_UBSAN_LINK_FLAGS})
    else()
        message(WARNING "Requested sanitizer ${ASTHRA_SANITIZER} is not supported")
    endif()
endif()