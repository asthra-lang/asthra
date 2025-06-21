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
    
    FetchContent_MakeAvailable(json-c)
    
    set(JSON_C_LIBRARIES json-c::json-c)
    set(JSON_C_INCLUDE_DIRS "")
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