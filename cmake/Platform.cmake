# Comprehensive Platform-specific Configuration for Asthra
# Provides architecture detection, OS features, system capabilities, and cross-compilation support

include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(TestBigEndian)

# Architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|amd64|AMD64)")
    set(ASTHRA_ARCH "x86_64")
    set(ASTHRA_ARCH_X64 TRUE)
    set(ASTHRA_POINTER_SIZE 8)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(i[3-6]86|x86)")
    set(ASTHRA_ARCH "x86")
    set(ASTHRA_ARCH_X86 TRUE)
    set(ASTHRA_POINTER_SIZE 4)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64)")
    set(ASTHRA_ARCH "arm64")
    set(ASTHRA_ARCH_ARM64 TRUE)
    set(ASTHRA_POINTER_SIZE 8)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(ASTHRA_ARCH "arm")
    set(ASTHRA_ARCH_ARM TRUE)
    set(ASTHRA_POINTER_SIZE 4)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
    set(ASTHRA_ARCH "riscv64")
    set(ASTHRA_ARCH_RISCV64 TRUE)
    set(ASTHRA_POINTER_SIZE 8)
else()
    set(ASTHRA_ARCH "unknown")
    set(ASTHRA_POINTER_SIZE 8)
    message(WARNING "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Platform detection with version info
if(APPLE)
    set(ASTHRA_PLATFORM "macOS")
    set(ASTHRA_EXE_EXT "")
    set(ASTHRA_LIB_EXT ".a")
    set(ASTHRA_DLL_EXT ".dylib")
    set(ASTHRA_PATH_SEP "/")
    set(ASTHRA_ENV_SEP ":")
    
    # macOS version detection
    execute_process(
        COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE MACOS_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(ASTHRA_MACOS_VERSION ${MACOS_VERSION})
    
    # Check for Apple Silicon
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(ASTHRA_APPLE_SILICON TRUE)
    else()
        set(ASTHRA_APPLE_SILICON FALSE)
    endif()
    
    # macOS-specific settings
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
    
elseif(WIN32)
    set(ASTHRA_PLATFORM "Windows")
    set(ASTHRA_EXE_EXT ".exe")
    set(ASTHRA_LIB_EXT ".lib")
    set(ASTHRA_DLL_EXT ".dll")
    set(ASTHRA_PATH_SEP "\\")
    set(ASTHRA_ENV_SEP ";")
    
    # Windows version detection
    execute_process(
        COMMAND cmd /c "ver"
        OUTPUT_VARIABLE WINDOWS_VERSION_RAW
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Extract Windows version from ver command output
    if(WINDOWS_VERSION_RAW)
        string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" WINDOWS_VERSION "${WINDOWS_VERSION_RAW}")
        set(ASTHRA_WINDOWS_VERSION ${WINDOWS_VERSION})
    else()
        set(ASTHRA_WINDOWS_VERSION "Unknown")
    endif()
    
    # Windows-specific settings for Clang
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    
    # Use Unix-style tools on Windows when using Clang
    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)
    
elseif(UNIX)
    set(ASTHRA_PLATFORM "Linux")
    set(ASTHRA_EXE_EXT "")
    set(ASTHRA_LIB_EXT ".a")
    set(ASTHRA_DLL_EXT ".so")
    set(ASTHRA_PATH_SEP "/")
    set(ASTHRA_ENV_SEP ":")
    
    # Linux distribution detection
    if(EXISTS "/etc/os-release")
        file(READ "/etc/os-release" OS_RELEASE)
        string(REGEX MATCH "ID=([a-zA-Z]+)" _ ${OS_RELEASE})
        set(ASTHRA_LINUX_DISTRO ${CMAKE_MATCH_1})
    endif()
    
    # Check for specific Unix variants
    if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        set(ASTHRA_PLATFORM "FreeBSD")
        set(ASTHRA_BSD TRUE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
        set(ASTHRA_PLATFORM "OpenBSD")
        set(ASTHRA_BSD TRUE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
        set(ASTHRA_PLATFORM "NetBSD")
        set(ASTHRA_BSD TRUE)
    endif()
    
else()
    set(ASTHRA_PLATFORM "Unknown")
    set(ASTHRA_EXE_EXT "")
    set(ASTHRA_LIB_EXT ".a")
    set(ASTHRA_DLL_EXT ".so")
    set(ASTHRA_PATH_SEP "/")
    set(ASTHRA_ENV_SEP ":")
endif()

# Endianness detection
test_big_endian(ASTHRA_BIG_ENDIAN)
if(ASTHRA_BIG_ENDIAN)
    set(ASTHRA_ENDIANNESS "BIG")
    add_compile_definitions(ASTHRA_BIG_ENDIAN=1)
else()
    set(ASTHRA_ENDIANNESS "LITTLE")
    add_compile_definitions(ASTHRA_LITTLE_ENDIAN=1)
endif()

# Compiler detection and configuration - Clang only
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    set(ASTHRA_COMPILER_TYPE "Clang")
    set(ASTHRA_COMPILER_VERSION ${CMAKE_C_COMPILER_VERSION})
    
    # Clang-specific flags
    add_compile_options(
        -Wall -Wextra -Werror -pedantic
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-sign-compare
        -fcolor-diagnostics
    )
    
else()
    message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}. Asthra requires Clang/LLVM.")
endif()

# System feature detection
if(WIN32)
    # Windows-specific headers
    check_include_file("windows.h" HAVE_WINDOWS_H)
    check_include_file("io.h" HAVE_IO_H)
    check_include_file("process.h" HAVE_PROCESS_H)
    check_include_file("direct.h" HAVE_DIRECT_H)
    check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
    check_include_file("fcntl.h" HAVE_FCNTL_H)
    
    # Check for pthread-win32 on Windows
    check_include_file("pthread.h" HAVE_PTHREAD_H)
else()
    # Unix-like systems
    check_include_file("unistd.h" HAVE_UNISTD_H)
    check_include_file("sys/mman.h" HAVE_SYS_MMAN_H)
    check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
    check_include_file("fcntl.h" HAVE_FCNTL_H)
    check_include_file("dlfcn.h" HAVE_DLFCN_H)
    check_include_file("execinfo.h" HAVE_EXECINFO_H)
    check_include_file("pthread.h" HAVE_PTHREAD_H)
endif()

# Function existence checks
if(WIN32)
    # Windows-specific functions
    check_function_exists(VirtualAlloc HAVE_VIRTUALALLOC)
    check_function_exists(VirtualProtect HAVE_VIRTUALPROTECT)
    check_function_exists(LoadLibrary HAVE_LOADLIBRARY)
    check_function_exists(GetProcAddress HAVE_GETPROCADDRESS)
    check_function_exists(QueryPerformanceCounter HAVE_QUERYPERFORMANCECOUNTER)
    check_function_exists(_aligned_malloc HAVE_ALIGNED_MALLOC)
    check_function_exists(GetModuleFileName HAVE_GETMODULEFILENAME)
    
    # Check for POSIX-like functions that might be available
    check_function_exists(aligned_alloc HAVE_ALIGNED_ALLOC)
else()
    # Unix-like systems
    check_function_exists(mmap HAVE_MMAP)
    check_function_exists(mprotect HAVE_MPROTECT)
    check_function_exists(backtrace HAVE_BACKTRACE)
    check_function_exists(dlopen HAVE_DLOPEN)
    check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)
    check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
    check_function_exists(posix_memalign HAVE_POSIX_MEMALIGN)
    check_function_exists(aligned_alloc HAVE_ALIGNED_ALLOC)
endif()

# Create platform configuration header
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/platform_config.h.in"
    "${CMAKE_BINARY_DIR}/include/platform_config.h"
    @ONLY
)

# Platform-specific libraries
set(ASTHRA_PLATFORM_LIBS "")
if(WIN32)
    # Windows system libraries
    list(APPEND ASTHRA_PLATFORM_LIBS kernel32 user32 advapi32 shell32)
    list(APPEND ASTHRA_PLATFORM_LIBS ole32 oleaut32 uuid comdlg32 gdi32)
    
    # Add pthread library if available (pthread-win32)
    if(HAVE_PTHREAD_H)
        list(APPEND ASTHRA_PLATFORM_LIBS pthread)
    endif()
    
    # Add math library equivalent for Windows
    list(APPEND ASTHRA_PLATFORM_LIBS msvcrt)
elseif(UNIX)
    list(APPEND ASTHRA_PLATFORM_LIBS m)
    if(NOT APPLE)
        list(APPEND ASTHRA_PLATFORM_LIBS rt dl)
    endif()
    if(HAVE_PTHREAD_H)
        list(APPEND ASTHRA_PLATFORM_LIBS pthread)
    endif()
endif()

# Cross-compilation support
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_NAME} on ${CMAKE_SYSTEM_PROCESSOR}")
    set(ASTHRA_CROSS_COMPILE TRUE)
    
    # Set up cross-compilation sysroot if provided
    if(CMAKE_SYSROOT)
        message(STATUS "Using sysroot: ${CMAKE_SYSROOT}")
    endif()
else()
    set(ASTHRA_CROSS_COMPILE FALSE)
endif()

# Platform-specific installation paths
if(APPLE)
    set(ASTHRA_INSTALL_BIN_DIR "bin")
    set(ASTHRA_INSTALL_LIB_DIR "lib")
    set(ASTHRA_INSTALL_INCLUDE_DIR "include")
    set(ASTHRA_INSTALL_DATA_DIR "share/asthra")
else()
    # Standard Unix paths without GNUInstallDirs
    set(ASTHRA_INSTALL_BIN_DIR "bin")
    set(ASTHRA_INSTALL_LIB_DIR "lib")
    set(ASTHRA_INSTALL_INCLUDE_DIR "include")
    set(ASTHRA_INSTALL_DATA_DIR "share/asthra")
endif()

# Platform capability flags
set(ASTHRA_HAS_ATOMICS TRUE)
set(ASTHRA_HAS_THREADS TRUE)
set(ASTHRA_HAS_TLS TRUE)
set(ASTHRA_HAS_SIMD FALSE)

# Check for SIMD support
if(ASTHRA_ARCH_X64 OR ASTHRA_ARCH_X86)
    include(CheckCSourceRuns)
    check_c_source_runs("
        #include <immintrin.h>
        int main() {
            __m128 a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            __m128 b = _mm_add_ps(a, a);
            return 0;
        }
    " HAVE_SSE)
    
    if(HAVE_SSE)
        set(ASTHRA_HAS_SIMD TRUE)
        add_compile_definitions(ASTHRA_HAS_SSE=1)
    endif()
elseif(ASTHRA_ARCH_ARM64 OR ASTHRA_ARCH_ARM)
    check_c_source_compiles("
        #include <arm_neon.h>
        int main() {
            float32x4_t a = vdupq_n_f32(1.0f);
            float32x4_t b = vaddq_f32(a, a);
            return 0;
        }
    " HAVE_NEON)
    
    if(HAVE_NEON)
        set(ASTHRA_HAS_SIMD TRUE)
        add_compile_definitions(ASTHRA_HAS_NEON=1)
    endif()
endif()

# Export platform information
set(ASTHRA_PLATFORM_INFO 
    "Platform: ${ASTHRA_PLATFORM} ${CMAKE_SYSTEM_VERSION}"
    "Architecture: ${ASTHRA_ARCH}"
    "Compiler: ${ASTHRA_COMPILER_TYPE} ${ASTHRA_COMPILER_VERSION}"
    "Endianness: ${ASTHRA_ENDIANNESS}"
    CACHE STRING "Platform information"
)

# Platform-specific targets
add_custom_target(platform-info
    COMMAND ${CMAKE_COMMAND} -E echo "=== Asthra Platform Information ==="
    COMMAND ${CMAKE_COMMAND} -E echo "Platform: ${ASTHRA_PLATFORM} ${CMAKE_SYSTEM_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Architecture: ${ASTHRA_ARCH}"
    COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${ASTHRA_COMPILER_TYPE} ${ASTHRA_COMPILER_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E echo "Endianness: ${ASTHRA_ENDIANNESS}"
    COMMAND ${CMAKE_COMMAND} -E echo "Pointer Size: ${ASTHRA_POINTER_SIZE} bytes"
    COMMAND ${CMAKE_COMMAND} -E echo "Cross-compiling: ${ASTHRA_CROSS_COMPILE}"
    COMMAND ${CMAKE_COMMAND} -E echo "Has Atomics: ${ASTHRA_HAS_ATOMICS}"
    COMMAND ${CMAKE_COMMAND} -E echo "Has Threads: ${ASTHRA_HAS_THREADS}"
    COMMAND ${CMAKE_COMMAND} -E echo "Has TLS: ${ASTHRA_HAS_TLS}"
    COMMAND ${CMAKE_COMMAND} -E echo "Has SIMD: ${ASTHRA_HAS_SIMD}"
    COMMENT "Display platform information"
)

# Platform capability test
add_custom_target(platform-test
    COMMAND ${CMAKE_COMMAND} -E echo "Testing platform capabilities..."
    COMMAND ${CMAKE_CTEST_COMMAND} -L platform --output-on-failure
    COMMENT "Test platform-specific features"
)

# Display configuration
message(STATUS "=== Asthra Platform Configuration ===")
message(STATUS "Platform: ${ASTHRA_PLATFORM}")
message(STATUS "Architecture: ${ASTHRA_ARCH}")
message(STATUS "Compiler: ${ASTHRA_COMPILER_TYPE} ${ASTHRA_COMPILER_VERSION}")
message(STATUS "Endianness: ${ASTHRA_ENDIANNESS}")
message(STATUS "Cross-compiling: ${ASTHRA_CROSS_COMPILE}")
if(APPLE AND ASTHRA_APPLE_SILICON)
    message(STATUS "Apple Silicon: Yes")
endif()
if(ASTHRA_LINUX_DISTRO)
    message(STATUS "Linux Distribution: ${ASTHRA_LINUX_DISTRO}")
endif()
if(WIN32 AND ASTHRA_WINDOWS_VERSION)
    message(STATUS "Windows Version: ${ASTHRA_WINDOWS_VERSION}")
endif()
message(STATUS "=====================================")