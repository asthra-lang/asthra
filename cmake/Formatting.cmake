# Code Formatting and Static Analysis for Asthra
# Provides code formatting, style checking, and static analysis capabilities

# Find formatting and analysis tools
find_program(CLANG_FORMAT_EXECUTABLE clang-format)
find_program(CLANG_TIDY_EXECUTABLE clang-tidy)
find_program(CPPCHECK_EXECUTABLE cppcheck)
find_program(IWYU_EXECUTABLE include-what-you-use)

# Formatting configuration
set(ASTHRA_FORMAT_STYLE "file" CACHE STRING "Clang-format style (file, LLVM, Google, etc.)")
option(ASTHRA_FORMAT_CHECK "Check formatting instead of fixing" OFF)

# Create .clang-format file if it doesn't exist
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.clang-format")
    file(WRITE "${CMAKE_SOURCE_DIR}/.clang-format" 
"---
Language: Cpp
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
BreakBeforeBraces: Attach
AllowShortIfStatementsOnASingleLine: false
IndentCaseLabels: false
ColumnLimit: 100
AccessModifierOffset: -4
AlignAfterOpenBracket: Align
AlignConsecutiveAssignments: false
AlignConsecutiveDeclarations: false
AlignOperands: true
AlignTrailingComments: true
AllowAllParametersOfDeclarationOnNextLine: true
AllowShortBlocksOnASingleLine: false
AllowShortCaseLabelsOnASingleLine: false
AllowShortFunctionsOnASingleLine: None
AllowShortLoopsOnASingleLine: false
AlwaysBreakAfterReturnType: None
AlwaysBreakBeforeMultilineStrings: false
BinPackArguments: true
BinPackParameters: true
BreakBeforeBinaryOperators: None
BreakBeforeTernaryOperators: true
BreakStringLiterals: true
CommentPragmas: '^ IWYU pragma:'
CompactNamespaces: false
DerivePointerAlignment: false
DisableFormat: false
FixNamespaceComments: true
IncludeBlocks: Preserve
IndentPPDirectives: None
IndentWrappedFunctionNames: false
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1
NamespaceIndentation: None
PointerAlignment: Right
ReflowComments: true
SortIncludes: true
SpaceAfterCStyleCast: false
SpaceBeforeAssignmentOperators: true
SpaceBeforeParens: ControlStatements
SpaceInEmptyParentheses: false
SpacesInCStyleCastParentheses: false
SpacesInParentheses: false
SpacesInSquareBrackets: false
Standard: c++17
...
")
    message(STATUS "Created .clang-format file")
endif()

# Collect source files for formatting
file(GLOB_RECURSE ASTHRA_FORMAT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/runtime/*.c
    ${CMAKE_SOURCE_DIR}/runtime/*.h
    ${CMAKE_SOURCE_DIR}/tests/*.c
    ${CMAKE_SOURCE_DIR}/tests/*.h
    ${CMAKE_SOURCE_DIR}/tools/*.c
)

# Exclude third-party and generated files
list(FILTER ASTHRA_FORMAT_SOURCES EXCLUDE REGEX "third-party|build|generated")

if(CLANG_FORMAT_EXECUTABLE)
    # Format target - formats code in place
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
                -i
                -style=${ASTHRA_FORMAT_STYLE}
                ${ASTHRA_FORMAT_SOURCES}
        COMMENT "Formatting code with clang-format"
        VERBATIM
    )
    
    # Format check target - checks if formatting is needed
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
                --dry-run
                --Werror
                -style=${ASTHRA_FORMAT_STYLE}
                ${ASTHRA_FORMAT_SOURCES}
        COMMENT "Checking code formatting"
        VERBATIM
    )
    
    # Format diff target - shows formatting differences
    add_custom_target(format-diff
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
                -style=${ASTHRA_FORMAT_STYLE}
                ${ASTHRA_FORMAT_SOURCES}
                | diff -u ${ASTHRA_FORMAT_SOURCES} -
        COMMENT "Showing formatting differences"
        VERBATIM
    )
else()
    message(STATUS "clang-format not found - formatting targets will not be available")
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found. Install clang-format to use formatting."
        COMMENT "Formatting not available"
    )
endif()

# Static analysis targets
if(CLANG_TIDY_EXECUTABLE)
    # Create .clang-tidy file if it doesn't exist
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.clang-tidy")
        file(WRITE "${CMAKE_SOURCE_DIR}/.clang-tidy"
"---
Checks: '-*,
  bugprone-*,
  cert-*,
  clang-analyzer-*,
  cppcoreguidelines-*,
  misc-*,
  modernize-*,
  performance-*,
  portability-*,
  readability-*,
  -modernize-use-trailing-return-type,
  -cppcoreguidelines-avoid-magic-numbers,
  -readability-magic-numbers'
WarningsAsErrors: ''
HeaderFilterRegex: '.*'
AnalyzeTemporaryDtors: false
FormatStyle: file
...")
        message(STATUS "Created .clang-tidy file")
    endif()
    
    add_custom_target(analyze
        COMMAND ${CLANG_TIDY_EXECUTABLE}
                -p ${CMAKE_BINARY_DIR}
                ${ASTHRA_FORMAT_SOURCES}
        COMMENT "Running clang-tidy static analysis"
        VERBATIM
    )
    
    add_custom_target(analyze-fix
        COMMAND ${CLANG_TIDY_EXECUTABLE}
                -p ${CMAKE_BINARY_DIR}
                -fix
                -fix-errors
                ${ASTHRA_FORMAT_SOURCES}
        COMMENT "Running clang-tidy with automatic fixes"
        VERBATIM
    )
else()
    message(STATUS "clang-tidy not found - static analysis will use alternative tools if available")
endif()

# MSVC static analysis
if(MSVC)
    add_custom_target(analyze-msvc
        COMMAND ${CMAKE_COMMAND} --build . -- /analyze
        COMMENT "Running MSVC static analysis"
    )
    
    if(NOT CLANG_TIDY_EXECUTABLE)
        add_custom_target(analyze DEPENDS analyze-msvc)
    endif()
endif()

# Cppcheck analysis
if(CPPCHECK_EXECUTABLE)
    add_custom_target(analyze-cppcheck
        COMMAND ${CPPCHECK_EXECUTABLE}
                --enable=all
                --inconclusive
                --force
                --inline-suppr
                --project=${CMAKE_BINARY_DIR}/compile_commands.json
                --suppress=missingIncludeSystem
                --suppress=unmatchedSuppression
                --error-exitcode=1
        COMMENT "Running cppcheck static analysis"
        VERBATIM
    )
    
    if(NOT CLANG_TIDY_EXECUTABLE AND NOT MSVC)
        add_custom_target(analyze DEPENDS analyze-cppcheck)
    endif()
else()
    if(NOT CLANG_TIDY_EXECUTABLE AND NOT MSVC)
        add_custom_target(analyze
            COMMAND ${CMAKE_COMMAND} -E echo "No static analysis tools found. Install clang-tidy or cppcheck."
            COMMENT "Static analysis not available"
        )
    endif()
endif()

# Include what you use
if(IWYU_EXECUTABLE)
    add_custom_target(iwyu
        COMMAND ${IWYU_EXECUTABLE}
                -p ${CMAKE_BINARY_DIR}
                ${ASTHRA_FORMAT_SOURCES}
        COMMENT "Running include-what-you-use"
        VERBATIM
    )
endif()

# Combined format and analyze
add_custom_target(lint
    COMMENT "Running format and static analysis"
)

if(CLANG_FORMAT_EXECUTABLE)
    add_dependencies(lint format)
endif()

if(TARGET analyze)
    add_dependencies(lint analyze)
endif()

# Format and analyze help
add_custom_target(format-help
    COMMAND ${CMAKE_COMMAND} -E echo "Formatting and Analysis Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  format              - Format code with clang-format"
    COMMAND ${CMAKE_COMMAND} -E echo "  format-check        - Check if formatting is needed"
    COMMAND ${CMAKE_COMMAND} -E echo "  format-diff         - Show formatting differences"
    COMMAND ${CMAKE_COMMAND} -E echo "  analyze             - Run static analysis"
    COMMAND ${CMAKE_COMMAND} -E echo "  analyze-fix         - Run analysis with auto-fix"
    COMMAND ${CMAKE_COMMAND} -E echo "  analyze-cppcheck    - Run cppcheck analysis"
    COMMAND ${CMAKE_COMMAND} -E echo "  iwyu                - Run include-what-you-use"
    COMMAND ${CMAKE_COMMAND} -E echo "  lint                - Run format and analyze"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Tools Status:"
    COMMAND ${CMAKE_COMMAND} -E echo "  clang-format: $<IF:$<BOOL:${CLANG_FORMAT_EXECUTABLE}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  clang-tidy: $<IF:$<BOOL:${CLANG_TIDY_EXECUTABLE}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  cppcheck: $<IF:$<BOOL:${CPPCHECK_EXECUTABLE}>,Found,Not Found>"
    COMMENT "Format and analysis help"
)