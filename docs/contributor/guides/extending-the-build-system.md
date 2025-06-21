# Extending the CMake Build System

**Prerequisites**: Familiarity with CMake and the existing Asthra build system architecture (see [Build System Reference](../../reference/build-system.md)).

## Overview

The Asthra compiler uses a modern, modular CMake build system designed for extensibility and maintainability. This guide explains how to extend the build system by adding new components, platforms, tools, and configurations.

## CMake Module Organization

The build system is organized into focused CMake modules:

- **`cmake/Platform.cmake`**: Platform detection and configuration
- **`cmake/Compiler.cmake`**: Compiler-specific settings and flags
- **`cmake/ThirdParty.cmake`**: External dependency management
- **`cmake/BuildTypes.cmake`**: Custom build type definitions
- **`cmake/Testing.cmake`**: Test configuration and CTest integration
- **`cmake/Sanitizers.cmake`**: Memory and thread safety analysis
- **`cmake/Coverage.cmake`**: Code coverage analysis
- **`cmake/DevTools.cmake`**: Development tool integration
- **`cmake/Help.cmake`**: Comprehensive help system

## 1. Adding New CMake Modules

To introduce a new logical grouping of build rules or configurations, create a new CMake module.

**Steps:**

1. **Create Module File**: Create a new `.cmake` file in the `cmake/` directory (e.g., `cmake/NewFeature.cmake`).
2. **Define Module Purpose**: Add comments at the top clearly stating its purpose and responsibilities.
3. **Add Dependencies**: Use `include()` statements for any required modules.
4. **Update Root CMakeLists.txt**: Add an `include()` statement for your new module in the root `CMakeLists.txt`.
5. **Test Integration**: Run a full clean build to verify integration.

**Example: `cmake/NewFeature.cmake`**

```cmake
# =============================================================================
# NEW FEATURE BUILD MODULE
# Purpose: Defines build configuration for the new 'feature_X' component.
# Depends on: Platform.cmake, Compiler.cmake
# =============================================================================

# Check prerequisites
if(NOT ASTHRA_PLATFORM)
    message(FATAL_ERROR "Platform.cmake must be included before NewFeature.cmake")
endif()

# Feature-specific options
option(ENABLE_FEATURE_X "Enable new feature X" OFF)

if(ENABLE_FEATURE_X)
    message(STATUS "Feature X enabled")
    
    # Define feature sources
    set(FEATURE_X_SOURCES
        src/feature_x/core.c
        src/feature_x/utils.c
        src/feature_x/api.c
    )
    
    # Create feature library
    add_library(asthra_feature_x STATIC ${FEATURE_X_SOURCES})
    
    # Configure feature library
    target_include_directories(asthra_feature_x
        PUBLIC src/feature_x
        PRIVATE src
    )
    
    target_compile_definitions(asthra_feature_x
        PRIVATE ASTHRA_FEATURE_X_ENABLED=1
    )
    
    # Link to main compiler
    target_link_libraries(asthra_compiler asthra_feature_x)
    
    # Add feature tests
    if(BUILD_TESTING)
        add_subdirectory(tests/feature_x)
    endif()
endif()
```

## 2. Adding New Platforms

Adding support for a new operating system or CPU architecture involves modifying existing platform-related modules.

**Steps:**

1. **Update Platform Detection (`cmake/Platform.cmake`)**: Add detection logic for the new platform based on `CMAKE_SYSTEM_NAME` and `CMAKE_SYSTEM_PROCESSOR`.

   **Example:**
   ```cmake
   # ... existing code ...
   elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
       set(ASTHRA_PLATFORM "freebsd")
       set(ASTHRA_PLATFORM_FREEBSD TRUE)
       set(ASTHRA_COMPILER_TYPE "clang")
   endif()
   ```

2. **Add Compiler Configuration (`cmake/Compiler.cmake`)**: If the new platform requires specific compiler flags or settings.

   **Example:**
   ```cmake
   # ... existing code ...
   if(ASTHRA_PLATFORM_FREEBSD)
       list(APPEND CMAKE_C_FLAGS "-I/usr/local/include")
       list(APPEND CMAKE_EXE_LINKER_FLAGS "-L/usr/local/lib")
       
       # FreeBSD-specific libraries
       find_library(EXECINFO_LIBRARY execinfo)
       if(EXECINFO_LIBRARY)
           target_link_libraries(asthra_compiler ${EXECINFO_LIBRARY})
       endif()
   endif()
   ```

3. **Update Third-Party Dependencies (`cmake/ThirdParty.cmake`)**: Handle platform-specific dependency locations.

   **Example:**
   ```cmake
   # ... existing code ...
   if(ASTHRA_PLATFORM_FREEBSD)
       # FreeBSD uses different pkg-config paths
       set(ENV{PKG_CONFIG_PATH} "/usr/local/libdata/pkgconfig:$ENV{PKG_CONFIG_PATH}")
   endif()
   ```

4. **Add Platform-Specific Features**: Create platform-specific targets or configurations if needed.

5. **Test Platform Support**: Set up a build environment for the new platform and verify compilation.

## 3. Adding New Development Tools

To integrate a new development tool (e.g., a custom linter, profiler, or code generator):

**Steps:**

1. **Define Tool in DevTools Module (`cmake/DevTools.cmake`)**:

   **Example:**
   ```cmake
   # Custom linter tool
   option(ENABLE_CUSTOM_LINTER "Enable custom linter tool" OFF)
   
   if(ENABLE_CUSTOM_LINTER)
       # Find linter sources
       file(GLOB LINTER_SOURCES tools/linter/*.c)
       
       # Create linter executable
       add_executable(asthra-linter ${LINTER_SOURCES})
       
       # Configure linter
       target_link_libraries(asthra-linter asthra_compiler)
       target_include_directories(asthra-linter PRIVATE src)
       
       # Add linter target
       add_custom_target(lint
           COMMAND asthra-linter src/
           DEPENDS asthra-linter
           COMMENT "Running custom linter"
           USES_TERMINAL
       )
       
       # Install linter
       install(TARGETS asthra-linter
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
       )
   endif()
   ```

2. **Add Tool Tests**: Create tests for the new tool in the appropriate test directory.

3. **Update Help System**: Add help information for the new tool in `cmake/Help.cmake`.

4. **Document Tool Usage**: Update relevant documentation.

## 4. Adding New Build Types

To create custom build configurations beyond Debug/Release:

**Steps:**

1. **Define Build Type (`cmake/BuildTypes.cmake`)**:

   **Example:**
   ```cmake
   # Profiling build type
   set(CMAKE_C_FLAGS_PROFILING "-O2 -g -pg -fno-omit-frame-pointer"
       CACHE STRING "Flags used by the C compiler during profiling builds.")
   set(CMAKE_EXE_LINKER_FLAGS_PROFILING "-pg"
       CACHE STRING "Flags used for linking binaries during profiling builds.")
   
   # Mark as advanced
   mark_as_advanced(
       CMAKE_C_FLAGS_PROFILING
       CMAKE_EXE_LINKER_FLAGS_PROFILING
   )
   
   # Add to build types list
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
       "Debug" "Release" "RelWithDebInfo" "MinSizeRel" "Profiling"
   )
   ```

2. **Create Build Type Target**:

   **Example:**
   ```cmake
   # Profiling target
   add_custom_target(build-profiling
       COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Profiling ${CMAKE_SOURCE_DIR}
       COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
       COMMENT "Building with profiling configuration"
   )
   ```

## 5. Adding New Test Categories

To add new test categories with CTest labels:

**Steps:**

1. **Update Testing Module (`cmake/Testing.cmake`)**:

   **Example:**
   ```cmake
   # Add new test category
   function(add_new_feature_tests)
       file(GLOB NEW_FEATURE_TESTS tests/new_feature/*_test.c)
       
       foreach(test_file ${NEW_FEATURE_TESTS})
           get_filename_component(test_name ${test_file} NAME_WE)
           
           add_executable(${test_name} ${test_file})
           target_link_libraries(${test_name} asthra_compiler test_framework)
           
           add_test(NAME ${test_name} COMMAND ${test_name})
           set_tests_properties(${test_name} PROPERTIES
               LABELS "new_feature"
               TIMEOUT 30
           )
       endforeach()
   endfunction()
   
   # Call the function if testing is enabled
   if(BUILD_TESTING)
       add_new_feature_tests()
   endif()
   ```

2. **Update Help System**: Add the new test category to help documentation.

## 6. Adding Third-Party Dependencies

To add new external dependencies:

**Steps:**

1. **Update ThirdParty Module (`cmake/ThirdParty.cmake`)**:

   **Example:**
   ```cmake
   # New library dependency
   option(USE_SYSTEM_NEWLIB "Use system-installed newlib" ON)
   
   if(USE_SYSTEM_NEWLIB)
       find_package(PkgConfig QUIET)
       if(PkgConfig_FOUND)
           pkg_check_modules(NEWLIB QUIET newlib)
       endif()
       
       if(NOT NEWLIB_FOUND)
           find_library(NEWLIB_LIBRARIES newlib)
           find_path(NEWLIB_INCLUDE_DIRS newlib.h)
       endif()
   endif()
   
   # Fallback to bundled version
   if(NOT NEWLIB_FOUND)
       message(STATUS "Using bundled newlib")
       add_subdirectory(third-party/newlib)
       set(NEWLIB_LIBRARIES newlib)
       set(NEWLIB_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/third-party/newlib/include)
   endif()
   
   # Export for use by other modules
   set(NEWLIB_LIBRARIES ${NEWLIB_LIBRARIES} CACHE INTERNAL "")
   set(NEWLIB_INCLUDE_DIRS ${NEWLIB_INCLUDE_DIRS} CACHE INTERNAL "")
   ```

2. **Link Dependencies**: Update target configurations to use the new dependency.

## 7. Adding Custom Targets

To add utility targets for development workflows:

**Example:**
```cmake
# Format code target
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        src/*.c src/*.h
        tests/*.c tests/*.h
        tools/*.c tools/*.h
    )
    
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting source code"
        USES_TERMINAL
    )
endif()

# Documentation generation target
find_program(DOXYGEN doxygen)
if(DOXYGEN)
    add_custom_target(docs
        COMMAND ${DOXYGEN} Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating documentation"
        USES_TERMINAL
    )
endif()
```

## Best Practices

### CMake Module Design
- **Single responsibility**: Each module should have one clear purpose
- **Clear dependencies**: Use `include()` statements for required modules
- **Consistent naming**: Follow established naming conventions
- **Comprehensive documentation**: Document all options and targets

### Variable Management
- **Use cache variables**: For user-configurable options
- **Namespace variables**: Use `ASTHRA_` prefix for project-specific variables
- **Mark advanced**: Use `mark_as_advanced()` for internal variables
- **Provide defaults**: Always provide sensible default values

### Target Definition
- **Use modern CMake**: Prefer target-based commands over global settings
- **Proper dependencies**: Ensure all target dependencies are correctly specified
- **Interface properties**: Use `target_include_directories()`, `target_link_libraries()`, etc.
- **Generator expressions**: Use generator expressions for conditional compilation

### Testing Integration
- **CTest integration**: Use `add_test()` for automatic test discovery
- **Test labels**: Assign appropriate labels for test categorization
- **Test properties**: Set timeouts, working directories, and other properties
- **Parallel testing**: Design tests to run safely in parallel

## Example: Complete Feature Addition

Here's a complete example of adding a new "optimization" feature:

**1. Create `cmake/Optimization.cmake`:**
```cmake
# Optimization feature module
option(ENABLE_OPTIMIZATION_PASSES "Enable advanced optimization passes" OFF)

if(ENABLE_OPTIMIZATION_PASSES)
    # Find optimization sources
    file(GLOB OPTIMIZATION_SOURCES src/optimization/*.c)
    
    # Create optimization library
    add_library(asthra_optimization STATIC ${OPTIMIZATION_SOURCES})
    
    # Configure library
    target_include_directories(asthra_optimization
        PUBLIC src/optimization
        PRIVATE src
    )
    
    target_compile_definitions(asthra_optimization
        PRIVATE ASTHRA_OPTIMIZATION_ENABLED=1
    )
    
    # Link to compiler
    target_link_libraries(asthra_compiler asthra_optimization)
    
    # Add optimization tests
    if(BUILD_TESTING)
        file(GLOB OPTIMIZATION_TESTS tests/optimization/*_test.c)
        foreach(test_file ${OPTIMIZATION_TESTS})
            get_filename_component(test_name ${test_file} NAME_WE)
            add_executable(${test_name} ${test_file})
            target_link_libraries(${test_name} asthra_optimization test_framework)
            add_test(NAME ${test_name} COMMAND ${test_name})
            set_tests_properties(${test_name} PROPERTIES LABELS "optimization")
        endforeach()
    endif()
    
    # Add optimization benchmark
    add_custom_target(benchmark-optimization
        COMMAND ${CMAKE_CTEST_COMMAND} -L optimization --verbose
        DEPENDS asthra_optimization
        COMMENT "Running optimization benchmarks"
    )
endif()
```

**2. Update root `CMakeLists.txt`:**
```cmake
# ... existing includes ...
include(cmake/Optimization.cmake)
```

**3. Update help system in `cmake/Help.cmake`:**
```cmake
# Add to help target
COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_OPTIMIZATION_PASSES=ON  - Enable optimization passes"
```

This modular approach ensures that the CMake build system remains maintainable, extensible, and easy to understand as the project grows. 