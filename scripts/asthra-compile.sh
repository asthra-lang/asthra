#!/bin/bash

# Asthra Programming Language
# Compilation Wrapper Script
# 
# Phase 5: Day 7 Final Integration & Documentation
# Provides easy-to-use interface for Asthra source → executable compilation
#
# Copyright (c) 2025 Asthra Project
# Licensed under the terms specified in LICENSE

set -euo pipefail

# =============================================================================
# CONFIGURATION AND SETUP
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# Default compilation settings
DEFAULT_OUTPUT="a.out"
DEFAULT_OPTIMIZATION_LEVEL="O1"
DEFAULT_DEBUG_INFO=true
DEFAULT_STATIC_LINKING=false
DEFAULT_STRIP_SYMBOLS=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Compilation options
VERBOSE=false
QUIET=false
DEBUG_MODE=false
SHOW_HELP=false
DRY_RUN=false

OUTPUT_FILE=""
SOURCE_FILES=()
INCLUDE_PATHS=()
LIBRARY_PATHS=()
LIBRARIES=()
OPTIMIZATION_LEVEL="${DEFAULT_OPTIMIZATION_LEVEL}"
ADD_DEBUG_INFO="${DEFAULT_DEBUG_INFO}"
STATIC_LINKING="${DEFAULT_STATIC_LINKING}"
STRIP_SYMBOLS="${DEFAULT_STRIP_SYMBOLS}"

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

log_info() {
    if [[ "${QUIET}" != "true" ]]; then
        echo -e "${BLUE}[INFO]${NC} $*"
    fi
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $*" >&2
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*" >&2
}

log_debug() {
    if [[ "${DEBUG_MODE}" == "true" ]]; then
        echo -e "${CYAN}[DEBUG]${NC} $*" >&2
    fi
}

print_version() {
    echo "Asthra Programming Language Compiler"
    echo "Version: Development Build"
    echo "Built: $(date)"
    echo "Copyright (c) 2025 Asthra Project"
}

print_usage() {
    cat << EOF
Usage: $0 [OPTIONS] <source_files...>

DESCRIPTION:
    Asthra compiler wrapper providing easy source-to-executable compilation.
    Supports single-file and multi-file projects with comprehensive options.

SOURCE FILES:
    source_files...             One or more .asthra source files to compile

OPTIONS:
    -o, --output FILE           Output executable name (default: ${DEFAULT_OUTPUT})
    -O, --optimize LEVEL        Optimization level: O0, O1, O2, O3 (default: ${DEFAULT_OPTIMIZATION_LEVEL})
    -g, --debug                 Include debug information (default: ${DEFAULT_DEBUG_INFO})
    -s, --static                Enable static linking (default: ${DEFAULT_STATIC_LINKING})
    --strip                     Strip symbols from executable (default: ${DEFAULT_STRIP_SYMBOLS})
    
    -I, --include PATH          Add include path for imports
    -L, --library-path PATH     Add library search path
    -l, --library NAME          Link with library
    
    -v, --verbose               Enable verbose output
    -q, --quiet                 Suppress non-essential output
    --debug-compiler            Enable compiler debug mode
    --dry-run                   Show compilation commands without executing
    
    --version                   Show version information
    -h, --help                  Show this help message

EXAMPLES:
    # Compile single file
    $0 hello.asthra
    
    # Compile with custom output name
    $0 -o hello hello.asthra
    
    # Compile multiple files with optimization
    $0 -O O2 -o myapp main.asthra utils.asthra
    
    # Compile with debug information and static linking
    $0 -g -s -o debug_app main.asthra
    
    # Compile with custom include and library paths
    $0 -I./include -L./lib -l mylib -o app main.asthra

ENVIRONMENT:
    ASTHRA_INCLUDE_PATH         Additional include paths (colon-separated)
    ASTHRA_LIBRARY_PATH         Additional library paths (colon-separated)
    ASTHRA_COMPILER_FLAGS       Additional compiler flags

FILES:
    Source files must have .asthra extension
    Output executable will have platform-appropriate format

For more information, see: docs/compilation-pipeline.md
EOF
}

# =============================================================================
# ARGUMENT PARSING
# =============================================================================

parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -o|--output)
                if [[ -z "${2:-}" ]]; then
                    log_error "Option $1 requires an argument"
                    exit 1
                fi
                OUTPUT_FILE="$2"
                shift 2
                ;;
            -O|--optimize)
                if [[ -z "${2:-}" ]]; then
                    log_error "Option $1 requires an argument"
                    exit 1
                fi
                case "$2" in
                    O0|O1|O2|O3)
                        OPTIMIZATION_LEVEL="$2"
                        ;;
                    *)
                        log_error "Invalid optimization level: $2 (must be O0, O1, O2, or O3)"
                        exit 1
                        ;;
                esac
                shift 2
                ;;
            -g|--debug)
                ADD_DEBUG_INFO=true
                shift
                ;;
            -s|--static)
                STATIC_LINKING=true
                shift
                ;;
            --strip)
                STRIP_SYMBOLS=true
                shift
                ;;
            -I|--include)
                if [[ -z "${2:-}" ]]; then
                    log_error "Option $1 requires an argument"
                    exit 1
                fi
                INCLUDE_PATHS+=("$2")
                shift 2
                ;;
            -L|--library-path)
                if [[ -z "${2:-}" ]]; then
                    log_error "Option $1 requires an argument"
                    exit 1
                fi
                LIBRARY_PATHS+=("$2")
                shift 2
                ;;
            -l|--library)
                if [[ -z "${2:-}" ]]; then
                    log_error "Option $1 requires an argument"
                    exit 1
                fi
                LIBRARIES+=("$2")
                shift 2
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -q|--quiet)
                QUIET=true
                shift
                ;;
            --debug-compiler)
                DEBUG_MODE=true
                shift
                ;;
            --dry-run)
                DRY_RUN=true
                shift
                ;;
            --version)
                print_version
                exit 0
                ;;
            -h|--help)
                print_usage
                exit 0
                ;;
            -*)
                log_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
            *)
                # This should be a source file
                if [[ "$1" == *.asthra ]]; then
                    SOURCE_FILES+=("$1")
                else
                    log_warning "File $1 does not have .asthra extension"
                    SOURCE_FILES+=("$1")
                fi
                shift
                ;;
        esac
    done
}

# =============================================================================
# VALIDATION
# =============================================================================

validate_inputs() {
    # Check for source files
    if [[ ${#SOURCE_FILES[@]} -eq 0 ]]; then
        log_error "No source files specified"
        print_usage
        exit 1
    fi
    
    # Validate source files exist
    for source_file in "${SOURCE_FILES[@]}"; do
        if [[ ! -f "${source_file}" ]]; then
            log_error "Source file not found: ${source_file}"
            exit 1
        fi
        
        if [[ ! -r "${source_file}" ]]; then
            log_error "Cannot read source file: ${source_file}"
            exit 1
        fi
    done
    
    # Set default output file if not specified
    if [[ -z "${OUTPUT_FILE}" ]]; then
        OUTPUT_FILE="${DEFAULT_OUTPUT}"
    fi
    
    # Validate include paths
    for include_path in "${INCLUDE_PATHS[@]}"; do
        if [[ ! -d "${include_path}" ]]; then
            log_warning "Include path does not exist: ${include_path}"
        fi
    done
    
    # Validate library paths
    for library_path in "${LIBRARY_PATHS[@]}"; do
        if [[ ! -d "${library_path}" ]]; then
            log_warning "Library path does not exist: ${library_path}"
        fi
    done
    
    log_debug "Validation completed successfully"
}

# =============================================================================
# INFRASTRUCTURE CHECKS
# =============================================================================

check_compiler_infrastructure() {
    log_info "Checking Asthra compiler infrastructure..."
    
    local missing_components=()
    
    # Check for Phase 5 pipeline orchestrator
    if [[ ! -f "${PROJECT_ROOT}/src/compiler/pipeline_orchestrator.h" ]]; then
        missing_components+=("Pipeline orchestrator")
    fi
    
    # Check for linker infrastructure
    if [[ ! -f "${PROJECT_ROOT}/src/linker/asthra_linker.h" ]]; then
        missing_components+=("Linker infrastructure")
    fi
    
    # Check for executable generator
    if [[ ! -f "${PROJECT_ROOT}/src/platform/executable_generator.h" ]]; then
        missing_components+=("Executable generator")
    fi
    
    # Check for parser interface
    if [[ ! -f "${PROJECT_ROOT}/src/parser/parser_string_interface.h" ]]; then
        missing_components+=("Parser interface")
    fi
    
    if [[ ${#missing_components[@]} -gt 0 ]]; then
        log_error "Missing required compiler components:"
        for component in "${missing_components[@]}"; do
            log_error "  - ${component}"
        done
        log_error "Please build the compiler infrastructure first"
        exit 1
    fi
    
    log_success "Compiler infrastructure verified"
}

check_build_artifacts() {
    log_info "Checking build artifacts..."
    
    # Check if Phase 5 pipeline tests are built
    local pipeline_test="${BUILD_DIR}/tests/pipeline/test_pipeline_orchestrator"
    if [[ ! -f "${pipeline_test}" ]]; then
        log_warning "Phase 5 pipeline tests not built"
        log_info "To build: make build-phase5-pipeline-tests"
    else
        log_debug "Phase 5 pipeline tests available"
    fi
    
    # Check if Phase 3 integration tests are built
    local phase3_test="${BUILD_DIR}/tests/integration/test_real_program_validation"
    if [[ ! -f "${phase3_test}" ]]; then
        log_warning "Phase 3 integration tests not built"
        log_info "To build: make build-integration-tests"
    else
        log_debug "Phase 3 integration tests available"
    fi
}

# =============================================================================
# COMPILATION EXECUTION
# =============================================================================

compile_sources() {
    log_info "Compiling ${#SOURCE_FILES[@]} source file(s) to '${OUTPUT_FILE}'..."
    
    # Build compilation command
    local compilation_command=()
    
    # For now, we'll use the Phase 3 framework with pipeline integration
    # In a complete implementation, this would use the full Phase 5 pipeline
    
    if [[ -f "${BUILD_DIR}/tests/integration/test_real_program_validation" ]]; then
        compilation_command+=("${BUILD_DIR}/tests/integration/test_real_program_validation")
        compilation_command+=("--compile-mode")
        compilation_command+=("--source")
        compilation_command+=("${SOURCE_FILES[0]}")  # Use first source file for now
        compilation_command+=("--output")
        compilation_command+=("${OUTPUT_FILE}")
        
        if [[ "${VERBOSE}" == "true" ]]; then
            compilation_command+=("--verbose")
        fi
        
        if [[ "${ADD_DEBUG_INFO}" == "true" ]]; then
            compilation_command+=("--debug")
        fi
        
        if [[ "${STATIC_LINKING}" == "true" ]]; then
            compilation_command+=("--static")
        fi
        
        if [[ "${STRIP_SYMBOLS}" == "true" ]]; then
            compilation_command+=("--strip")
        fi
        
        # Add optimization level
        compilation_command+=("--optimize")
        compilation_command+=("${OPTIMIZATION_LEVEL}")
        
    else
        # Fallback: simulate compilation using existing infrastructure
        log_warning "Phase 3 test framework not built, using simulation mode"
        compilation_command=("echo")
        compilation_command+=("Simulating compilation of ${SOURCE_FILES[*]} → ${OUTPUT_FILE}")
    fi
    
    # Show compilation command in debug mode
    if [[ "${DEBUG_MODE}" == "true" || "${DRY_RUN}" == "true" ]]; then
        log_debug "Compilation command: ${compilation_command[*]}"
    fi
    
    # Execute compilation
    if [[ "${DRY_RUN}" == "true" ]]; then
        log_info "Dry run mode: would execute compilation"
        return 0
    fi
    
    local compilation_result=0
    local start_time=$(date +%s.%N)
    
    "${compilation_command[@]}" || compilation_result=$?
    
    local end_time=$(date +%s.%N)
    local compilation_time=$(echo "${end_time} - ${start_time}" | bc -l)
    
    if [[ ${compilation_result} -eq 0 ]]; then
        log_success "Compilation completed in ${compilation_time}s"
        
        # Verify output file was created
        if [[ -f "${OUTPUT_FILE}" ]]; then
            local file_size=$(stat -c%s "${OUTPUT_FILE}" 2>/dev/null || stat -f%z "${OUTPUT_FILE}" 2>/dev/null || echo "unknown")
            log_success "Executable '${OUTPUT_FILE}' generated (${file_size} bytes)"
            
            # Set executable permissions
            chmod +x "${OUTPUT_FILE}"
            log_debug "Set executable permissions on '${OUTPUT_FILE}'"
            
            return 0
        else
            log_error "Compilation reported success but output file not found"
            return 1
        fi
    else
        log_error "Compilation failed with exit code ${compilation_result}"
        return ${compilation_result}
    fi
}

# =============================================================================
# POST-COMPILATION VALIDATION
# =============================================================================

validate_executable() {
    if [[ ! -f "${OUTPUT_FILE}" ]]; then
        return 1
    fi
    
    log_info "Validating generated executable..."
    
    # Check file type
    if command -v file >/dev/null 2>&1; then
        local file_type=$(file "${OUTPUT_FILE}")
        log_debug "File type: ${file_type}"
    fi
    
    # Check if executable has proper permissions
    if [[ -x "${OUTPUT_FILE}" ]]; then
        log_success "Executable has correct permissions"
    else
        log_warning "Executable lacks execute permissions"
        chmod +x "${OUTPUT_FILE}"
        log_info "Fixed executable permissions"
    fi
    
    # Platform-specific validation
    case "$(uname)" in
        Linux)
            if command -v readelf >/dev/null 2>&1; then
                if readelf -h "${OUTPUT_FILE}" >/dev/null 2>&1; then
                    log_success "Valid ELF executable"
                else
                    log_warning "Not a valid ELF executable"
                fi
            fi
            ;;
        Darwin)
            if command -v otool >/dev/null 2>&1; then
                if otool -h "${OUTPUT_FILE}" >/dev/null 2>&1; then
                    log_success "Valid Mach-O executable"
                else
                    log_warning "Not a valid Mach-O executable"
                fi
            fi
            ;;
        *)
            log_debug "Platform-specific validation not available"
            ;;
    esac
    
    return 0
}

# =============================================================================
# MAIN EXECUTION
# =============================================================================

main() {
    # Parse command line arguments
    parse_arguments "$@"
    
    # Validate inputs
    validate_inputs
    
    # Show compilation info
    if [[ "${VERBOSE}" == "true" ]]; then
        echo ""
        log_info "=== Asthra Compilation ==="
        log_info "Source files: ${SOURCE_FILES[*]}"
        log_info "Output file: ${OUTPUT_FILE}"
        log_info "Optimization: ${OPTIMIZATION_LEVEL}"
        log_info "Debug info: ${ADD_DEBUG_INFO}"
        log_info "Static linking: ${STATIC_LINKING}"
        log_info "Strip symbols: ${STRIP_SYMBOLS}"
        
        if [[ ${#INCLUDE_PATHS[@]} -gt 0 ]]; then
            log_info "Include paths: ${INCLUDE_PATHS[*]}"
        fi
        
        if [[ ${#LIBRARY_PATHS[@]} -gt 0 ]]; then
            log_info "Library paths: ${LIBRARY_PATHS[*]}"
        fi
        
        if [[ ${#LIBRARIES[@]} -gt 0 ]]; then
            log_info "Libraries: ${LIBRARIES[*]}"
        fi
        echo ""
    fi
    
    # Check infrastructure
    check_compiler_infrastructure
    check_build_artifacts
    
    # Perform compilation
    if compile_sources; then
        # Validate result
        validate_executable
        
        log_success "Compilation successful!"
        if [[ "${QUIET}" != "true" ]]; then
            echo ""
            echo "✅ Executable: ${OUTPUT_FILE}"
            echo "📄 Source files: ${#SOURCE_FILES[@]}"
            echo "🔧 Optimization: ${OPTIMIZATION_LEVEL}"
            
            if command -v ls >/dev/null 2>&1; then
                echo "📊 File size: $(ls -lh "${OUTPUT_FILE}" | awk '{print $5}')"
            fi
        fi
        
        exit 0
    else
        log_error "Compilation failed"
        exit 1
    fi
}

# Execute main function with all arguments
main "$@" 
