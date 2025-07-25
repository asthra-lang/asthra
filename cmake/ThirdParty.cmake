# Third-party dependency management
include(FetchContent)

# Option to use system libraries
option(USE_SYSTEM_JSON_C "Use system-installed json-c library" ON)

if(USE_SYSTEM_JSON_C)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(JSON_C QUIET json-c)
        if(JSON_C_FOUND)
            # Get the full library path from pkg-config
            pkg_get_variable(JSON_C_LIBDIR json-c libdir)
            find_library(JSON_C_LIBRARY_FULL json-c PATHS ${JSON_C_LIBDIR} NO_DEFAULT_PATH)
            if(JSON_C_LIBRARY_FULL)
                set(JSON_C_LIBRARIES ${JSON_C_LIBRARY_FULL})
            endif()
        endif()
    endif()
    
    if(NOT JSON_C_FOUND)
        find_path(JSON_C_INCLUDE_DIR json-c/json.h)
        find_library(JSON_C_LIBRARY json-c)
        
        if(JSON_C_INCLUDE_DIR AND JSON_C_LIBRARY)
            set(JSON_C_FOUND TRUE)
            set(JSON_C_INCLUDE_DIRS ${JSON_C_INCLUDE_DIR})
            set(JSON_C_LIBRARIES ${JSON_C_LIBRARY})
        endif()
    endif()
endif()

if(NOT JSON_C_FOUND)
    message(STATUS "Using bundled json-c library")
    FetchContent_Declare(
        json-c
        GIT_REPOSITORY https://github.com/json-c/json-c.git
        GIT_TAG json-c-0.17-20230812
    )
    
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  
    set(BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
    set(DISABLE_WERROR ON CACHE BOOL "" FORCE)
    
    # json-c uses its own DISABLE_WERROR variable but seems to ignore it
    # We need to ensure warnings don't fail the build
    set(JSON_C_DISABLE_WERROR ON CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(json-c)
    
    # After json-c target is created, modify its compile options
    if(TARGET json-c)
        # Remove any -Werror flags and add specific warning suppressions
        get_target_property(JSON_C_COMPILE_OPTIONS json-c COMPILE_OPTIONS)
        if(JSON_C_COMPILE_OPTIONS)
            list(REMOVE_ITEM JSON_C_COMPILE_OPTIONS "-Werror")
        endif()
        
        # Add flags to suppress the specific warnings causing issues
        target_compile_options(json-c PRIVATE 
            -Wno-error
            -Wno-error=duplicated-branches
            -Wno-error=pedantic
            -Wno-duplicated-branches
            -Wno-pedantic
        )
    endif()
    
    # Create the namespace alias that our code expects
    if(TARGET json-c AND NOT TARGET json-c::json-c)
        add_library(json-c::json-c ALIAS json-c)
    endif()
    
    # Get the json-c source and build directories for include paths
    FetchContent_GetProperties(json-c)
    
    set(JSON_C_LIBRARIES json-c::json-c)
    set(JSON_C_INCLUDE_DIRS "${json-c_SOURCE_DIR};${json-c_BINARY_DIR}")
else()
    message(STATUS "Using system json-c library")
    add_library(json-c::json-c INTERFACE IMPORTED)
    target_include_directories(json-c::json-c INTERFACE ${JSON_C_INCLUDE_DIRS})
    target_link_libraries(json-c::json-c INTERFACE ${JSON_C_LIBRARIES})
    
    # Set JSON_C_LIBRARY for get_filename_component
    if(NOT JSON_C_LIBRARY AND JSON_C_LIBRARIES)
        list(GET JSON_C_LIBRARIES 0 JSON_C_LIBRARY)
    endif()
    
    # Add library directories for linking
    if(JSON_C_LIBRARY)
        get_filename_component(JSON_C_LIB_DIR ${JSON_C_LIBRARY} DIRECTORY)
        target_link_directories(json-c::json-c INTERFACE ${JSON_C_LIB_DIR})
    endif()
endif()

message(STATUS "JSON-C Configuration: ${JSON_C_FOUND}")
if(JSON_C_FOUND)
    message(STATUS "  Include Dirs: ${JSON_C_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${JSON_C_LIBRARIES}")
endif()

# LLVM Configuration (Required)
message(STATUS "Configuring LLVM...")

# Set minimum required LLVM version
set(LLVM_MIN_VERSION "18.0")

# Try to find LLVM using CMake config
find_package(LLVM ${LLVM_MIN_VERSION} QUIET CONFIG)

# Windows support has been removed

if(NOT LLVM_FOUND)
    # Fall back to manual search - look for any llvm-config (Unix-like systems)
    find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config llvm-config-20 llvm-config-19 llvm-config-18)
    
    if(LLVM_CONFIG_EXECUTABLE)
        execute_process(
            COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
            OUTPUT_VARIABLE LLVM_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        # Check if the version is at least 18.0
        string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" VERSION_MATCH ${LLVM_VERSION})
        if(CMAKE_MATCH_1 LESS 18)
            message(FATAL_ERROR "Found LLVM ${LLVM_VERSION}, but LLVM 18.0 or later is required.")
        endif()
        
        execute_process(
            COMMAND ${LLVM_CONFIG_EXECUTABLE} --cflags
            OUTPUT_VARIABLE LLVM_CFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        execute_process(
            COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
            OUTPUT_VARIABLE LLVM_LDFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        execute_process(
            COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core support native bitwriter passes target asmparser asmprinter mc mcparser
            OUTPUT_VARIABLE LLVM_LIBS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        set(LLVM_FOUND TRUE)
    endif()
endif()

if(NOT LLVM_FOUND)
    message(FATAL_ERROR "LLVM 18.0 or later not found. Please install LLVM 18.0+.\n"
                        "On macOS: brew install llvm\n"
                        "On Ubuntu: apt install llvm-dev (or llvm-18-dev for specific version)\n"
                        "You may need to set LLVM_DIR or ensure llvm-config is in your PATH.")
else()
    # Validate version for CMake package path as well
    if(LLVM_VERSION)
        string(REGEX MATCH "^([0-9]+)\\.([0-9]+)" VERSION_MATCH ${LLVM_VERSION})
        if(CMAKE_MATCH_1 LESS 18)
            message(FATAL_ERROR "Found LLVM ${LLVM_VERSION}, but LLVM 18.0 or later is required.")
        endif()
    endif()
    
    message(STATUS "Found LLVM ${LLVM_VERSION}")
    
    # Create LLVM interface library
    add_library(llvm::llvm INTERFACE IMPORTED)
    
    if(LLVM_CONFIG_EXECUTABLE)
        # Using llvm-config
        separate_arguments(LLVM_CFLAGS_LIST UNIX_COMMAND "${LLVM_CFLAGS}")
        separate_arguments(LLVM_LDFLAGS_LIST UNIX_COMMAND "${LLVM_LDFLAGS}")
        separate_arguments(LLVM_LIBS_LIST UNIX_COMMAND "${LLVM_LIBS}")
        
        target_compile_options(llvm::llvm INTERFACE ${LLVM_CFLAGS_LIST})
        target_link_options(llvm::llvm INTERFACE ${LLVM_LDFLAGS_LIST})
        target_link_libraries(llvm::llvm INTERFACE ${LLVM_LIBS_LIST})
        
        # Link with C++ standard library for LLVM
        if(APPLE)
            target_link_libraries(llvm::llvm INTERFACE "-lc++")
        else()
            target_link_libraries(llvm::llvm INTERFACE "-lstdc++")
        endif()
    else()
        # Using CMake package
        llvm_map_components_to_libnames(llvm_libs support core irreader bitwriter asmparser asmprinter mc mcparser native)
        target_link_libraries(llvm::llvm INTERFACE ${llvm_libs})
        target_include_directories(llvm::llvm INTERFACE ${LLVM_INCLUDE_DIRS})
        target_compile_definitions(llvm::llvm INTERFACE ${LLVM_DEFINITIONS})
        
        # Link with C++ standard library for LLVM
        if(APPLE)
            target_link_libraries(llvm::llvm INTERFACE "-lc++")
        else()
            target_link_libraries(llvm::llvm INTERFACE "-lstdc++")
        endif()
    endif()
    
    # Always define LLVM backend enabled
    add_compile_definitions(ASTHRA_LLVM_BACKEND)
endif() 