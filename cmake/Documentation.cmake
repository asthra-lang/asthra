# Documentation Generation for Asthra
# Provides documentation generation using Doxygen and other tools

find_package(Doxygen OPTIONAL_COMPONENTS dot mscgen dia)

option(BUILD_DOCUMENTATION "Create and install documentation (requires Doxygen)" ${DOXYGEN_FOUND})
set(ASTHRA_DOC_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs" CACHE PATH "Documentation output directory")

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()
    
    # Create documentation output directory
    file(MAKE_DIRECTORY ${ASTHRA_DOC_OUTPUT_DIR})
    
    # Configure Doxyfile if it doesn't exist
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/Doxyfile.in")
        file(WRITE "${CMAKE_SOURCE_DIR}/Doxyfile.in"
"# Doxyfile for Asthra Programming Language

# Project information
PROJECT_NAME           = \"Asthra\"
PROJECT_NUMBER         = \"@CMAKE_PROJECT_VERSION@\"
PROJECT_BRIEF          = \"AI-Friendly Systems Programming Language\"
PROJECT_LOGO           = 

# Input/output
INPUT                  = @CMAKE_SOURCE_DIR@/src \\
                        @CMAKE_SOURCE_DIR@/runtime \\
                        @CMAKE_SOURCE_DIR@/include \\
                        @CMAKE_SOURCE_DIR@/docs
RECURSIVE              = YES
OUTPUT_DIRECTORY       = @ASTHRA_DOC_OUTPUT_DIR@

# Configuration
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = YES
EXTRACT_STATIC         = YES
EXTRACT_LOCAL_CLASSES  = YES
EXTRACT_LOCAL_METHODS  = YES
EXTRACT_ANON_NSPACES   = YES
HIDE_UNDOC_MEMBERS     = NO
HIDE_UNDOC_CLASSES     = NO
HIDE_FRIEND_COMPOUNDS  = NO
SHOW_INCLUDE_FILES     = YES
INLINE_INFO            = YES
SORT_MEMBER_DOCS       = YES
SORT_BRIEF_DOCS        = NO
GENERATE_TODOLIST      = YES
GENERATE_TESTLIST      = YES
GENERATE_BUGLIST       = YES
GENERATE_DEPRECATEDLIST = YES

# Build options
ENABLE_PREPROCESSING   = YES
MACRO_EXPANSION        = YES
EXPAND_ONLY_PREDEF     = NO
SEARCH_INCLUDES        = YES
INCLUDE_PATH           = @CMAKE_SOURCE_DIR@/include
PREDEFINED             = DOXYGEN_SHOULD_SKIP_THIS \\
                        ASTHRA_API= \\
                        ASTHRA_INTERNAL=

# Warnings and progress
QUIET                  = NO
WARNINGS               = YES
WARN_IF_UNDOCUMENTED   = YES
WARN_IF_DOC_ERROR      = YES
WARN_NO_PARAMDOC       = YES
WARN_AS_ERROR          = NO

# Input file patterns
FILE_PATTERNS          = *.c \\
                        *.cc \\
                        *.cxx \\
                        *.cpp \\
                        *.c++ \\
                        *.h \\
                        *.hh \\
                        *.hxx \\
                        *.hpp \\
                        *.h++ \\
                        *.md

# Exclusions
EXCLUDE                = @CMAKE_SOURCE_DIR@/third-party \\
                        @CMAKE_SOURCE_DIR@/build \\
                        @CMAKE_SOURCE_DIR@/tests/generated
EXCLUDE_SYMLINKS       = NO
EXCLUDE_PATTERNS       = */test/* \\
                        */tests/* \\
                        *_test.* \\
                        *_private.*

# Output formats
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
GENERATE_RTF           = NO
GENERATE_XML           = NO
GENERATE_DOCBOOK       = NO
GENERATE_MAN           = NO

# HTML output
HTML_OUTPUT            = html
HTML_FILE_EXTENSION    = .html
HTML_HEADER            = 
HTML_FOOTER            = 
HTML_STYLESHEET        = 
HTML_EXTRA_STYLESHEET  = 
HTML_COLORSTYLE_HUE    = 220
HTML_COLORSTYLE_SAT    = 100
HTML_COLORSTYLE_GAMMA  = 80
HTML_TIMESTAMP         = YES
HTML_DYNAMIC_SECTIONS  = YES
HTML_INDEX_NUM_ENTRIES = 100

# Source browsing
SOURCE_BROWSER         = YES
INLINE_SOURCES         = NO
STRIP_CODE_COMMENTS    = YES
REFERENCED_BY_RELATION = YES
REFERENCES_RELATION    = YES
REFERENCES_LINK_SOURCE = YES
SOURCE_TOOLTIPS        = YES
USE_HTAGS              = NO
VERBATIM_HEADERS       = YES

# Graphs
HAVE_DOT               = @DOXYGEN_DOT_FOUND@
DOT_NUM_THREADS        = 0
DOT_FONTNAME           = Helvetica
DOT_FONTSIZE           = 10
DOT_FONTPATH           = 
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
GROUP_GRAPHS           = YES
UML_LOOK               = NO
TEMPLATE_RELATIONS     = NO
INCLUDE_GRAPH          = YES
INCLUDED_BY_GRAPH      = YES
CALL_GRAPH             = YES
CALLER_GRAPH           = YES
GRAPHICAL_HIERARCHY    = YES
DIRECTORY_GRAPH        = YES
DOT_IMAGE_FORMAT       = png
INTERACTIVE_SVG        = NO
DOT_PATH               = @DOXYGEN_DOT_EXECUTABLE@
DOTFILE_DIRS           = 
DOT_GRAPH_MAX_NODES    = 50
MAX_DOT_GRAPH_DEPTH    = 0
DOT_TRANSPARENT        = NO
DOT_MULTI_TARGETS      = YES
GENERATE_LEGEND        = YES
DOT_CLEANUP            = YES
")
        message(STATUS "Created Doxyfile.in template")
    endif()
    
    # Configure Doxyfile
    configure_file(
        "${CMAKE_SOURCE_DIR}/Doxyfile.in"
        "${CMAKE_BINARY_DIR}/Doxyfile"
        @ONLY
    )
    
    # Main documentation target
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    
    # Documentation with automatic browser opening
    add_custom_target(doc-open
        COMMAND ${CMAKE_COMMAND} -E echo "Opening documentation..."
        COMMAND ${CMAKE_COMMAND} -E echo "file://${ASTHRA_DOC_OUTPUT_DIR}/html/index.html"
        DEPENDS doc
        COMMENT "Open documentation in browser"
    )
    
    # Clean documentation
    add_custom_target(doc-clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${ASTHRA_DOC_OUTPUT_DIR}
        COMMENT "Clean documentation"
    )
    
    # Install documentation
    install(DIRECTORY ${ASTHRA_DOC_OUTPUT_DIR}/html
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/asthra
        COMPONENT documentation
        OPTIONAL
    )
    
    # Generate markdown documentation from source
    add_custom_target(doc-markdown
        COMMAND ${CMAKE_COMMAND} -E echo "Generating markdown documentation..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ASTHRA_DOC_OUTPUT_DIR}/markdown
        COMMENT "Generate markdown documentation"
    )
    
    # API reference generation
    add_custom_target(doc-api
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
        COMMAND ${CMAKE_COMMAND} -E echo "API documentation generated in ${ASTHRA_DOC_OUTPUT_DIR}/html"
        DEPENDS doc
        COMMENT "Generate API reference documentation"
    )
    
else()
    # Dummy targets when documentation is disabled
    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E echo "Documentation generation disabled. Install Doxygen and set -DBUILD_DOCUMENTATION=ON"
        COMMENT "Documentation not available"
    )
    
    add_custom_target(doc-clean
        COMMAND ${CMAKE_COMMAND} -E echo "Documentation generation disabled"
        COMMENT "Documentation not available"
    )
endif()

# README and documentation validation
if(EXISTS ${CMAKE_SOURCE_DIR}/README.md)
    set(HAS_README "Yes")
else()
    set(HAS_README "No")
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/docs)
    set(HAS_DOCS "Yes")
else()
    set(HAS_DOCS "No")
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/CLAUDE.md)
    set(HAS_CLAUDE "Yes")
else()
    set(HAS_CLAUDE "No")
endif()

add_custom_target(doc-check
    COMMAND ${CMAKE_COMMAND} -E echo "Checking documentation..."
    COMMAND ${CMAKE_COMMAND} -E echo "README.md exists: ${HAS_README}"
    COMMAND ${CMAKE_COMMAND} -E echo "docs/ exists: ${HAS_DOCS}"
    COMMAND ${CMAKE_COMMAND} -E echo "CLAUDE.md exists: ${HAS_CLAUDE}"
    COMMENT "Check documentation files"
)

# Documentation help
add_custom_target(doc-help
    COMMAND ${CMAKE_COMMAND} -E echo "Documentation Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  doc                 - Generate documentation"
    COMMAND ${CMAKE_COMMAND} -E echo "  doc-open            - Generate and open docs"
    COMMAND ${CMAKE_COMMAND} -E echo "  doc-clean           - Clean documentation"
    COMMAND ${CMAKE_COMMAND} -E echo "  doc-api             - Generate API reference"
    COMMAND ${CMAKE_COMMAND} -E echo "  doc-markdown        - Generate markdown docs"
    COMMAND ${CMAKE_COMMAND} -E echo "  doc-check           - Check doc files exist"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Options:"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DBUILD_DOCUMENTATION=ON - Enable docs"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DASTHRA_DOC_OUTPUT_DIR=path - Set output dir"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Status:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Doxygen: $<IF:$<BOOL:${DOXYGEN_FOUND}>,Found,Not Found>"
    COMMAND ${CMAKE_COMMAND} -E echo "  Graphviz: $<IF:$<BOOL:${DOXYGEN_DOT_FOUND}>,Found,Not Found>"
    COMMENT "Documentation help"
)