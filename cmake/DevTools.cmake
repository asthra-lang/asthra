# Development Tools Infrastructure for Asthra
# Provides AST visualizer, grammar validator, symbol inspector, test generator,
# performance profiler, fuzzing harness, and language server support

option(ENABLE_DEV_TOOLS "Enable development tools" ON)
option(ENABLE_AST_VISUALIZER "Enable AST visualizer tool" ON)
option(ENABLE_GRAMMAR_VALIDATOR "Enable grammar validator" ON)
option(ENABLE_SYMBOL_INSPECTOR "Enable symbol inspector" ON)
option(ENABLE_TEST_GENERATOR "Enable test generator" ON)
option(ENABLE_PERFORMANCE_PROFILER "Enable performance profiler" ON)
option(ENABLE_FUZZING "Enable fuzzing harness" OFF)
option(ENABLE_LANGUAGE_SERVER "Enable language server" ON)

# Development tools output directory
set(DEV_TOOLS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/dev-tools" CACHE PATH "Development tools output directory")
file(MAKE_DIRECTORY ${DEV_TOOLS_OUTPUT_DIR})

if(ENABLE_DEV_TOOLS)
    message(STATUS "Development tools enabled")
    
    # AST Visualizer
    if(ENABLE_AST_VISUALIZER)
        add_custom_target(dev-ast-visualizer
            COMMAND ${CMAKE_COMMAND} -E echo "Running AST Visualizer..."
            COMMAND $<TARGET_FILE:asthra> --ast-visualize --output ${DEV_TOOLS_OUTPUT_DIR}/ast.dot
            DEPENDS asthra
            COMMENT "Generate AST visualization"
            USES_TERMINAL
        )
        
        add_custom_target(dev-ast-visualizer-png
            COMMAND ${CMAKE_COMMAND} -E echo "Converting AST to PNG..."
            COMMAND dot -Tpng ${DEV_TOOLS_OUTPUT_DIR}/ast.dot -o ${DEV_TOOLS_OUTPUT_DIR}/ast.png 2>/dev/null || echo "graphviz not installed"
            DEPENDS dev-ast-visualizer
            COMMENT "Convert AST visualization to PNG"
        )
        
        add_custom_target(dev-ast-visualizer-svg
            COMMAND ${CMAKE_COMMAND} -E echo "Converting AST to SVG..."
            COMMAND dot -Tsvg ${DEV_TOOLS_OUTPUT_DIR}/ast.dot -o ${DEV_TOOLS_OUTPUT_DIR}/ast.svg 2>/dev/null || echo "graphviz not installed"
            DEPENDS dev-ast-visualizer
            COMMENT "Convert AST visualization to SVG"
        )
    endif()
    
    # Grammar Validator
    if(ENABLE_GRAMMAR_VALIDATOR)
        add_custom_target(dev-grammar-validator
            COMMAND ${CMAKE_COMMAND} -E echo "Validating grammar..."
            COMMAND $<TARGET_FILE:asthra> --validate-grammar ${CMAKE_SOURCE_DIR}/grammar.txt
            DEPENDS asthra
            COMMENT "Validate Asthra grammar"
            USES_TERMINAL
        )
        
        add_custom_target(dev-grammar-report
            COMMAND ${CMAKE_COMMAND} -E echo "Generating grammar report..."
            COMMAND $<TARGET_FILE:asthra> --grammar-report --output ${DEV_TOOLS_OUTPUT_DIR}/grammar-report.txt
            DEPENDS asthra
            COMMENT "Generate grammar validation report"
        )
        
        add_custom_target(dev-grammar-conflicts
            COMMAND ${CMAKE_COMMAND} -E echo "Checking for grammar conflicts..."
            COMMAND $<TARGET_FILE:asthra> --check-grammar-conflicts ${CMAKE_SOURCE_DIR}/grammar.txt
            DEPENDS asthra
            COMMENT "Check for grammar conflicts and ambiguities"
        )
    endif()
    
    # Symbol Inspector
    if(ENABLE_SYMBOL_INSPECTOR)
        add_custom_target(dev-symbol-inspector
            COMMAND ${CMAKE_COMMAND} -E echo "Inspecting symbols..."
            COMMAND $<TARGET_FILE:asthra> --inspect-symbols --output ${DEV_TOOLS_OUTPUT_DIR}/symbols.json
            DEPENDS asthra
            COMMENT "Inspect symbol table"
            USES_TERMINAL
        )
        
        add_custom_target(dev-symbol-dump
            COMMAND ${CMAKE_COMMAND} -E echo "Dumping symbol table..."
            COMMAND $<TARGET_FILE:asthra> --dump-symbols --format=human --output ${DEV_TOOLS_OUTPUT_DIR}/symbols.txt
            DEPENDS asthra
            COMMENT "Dump human-readable symbol table"
        )
        
        add_custom_target(dev-symbol-stats
            COMMAND ${CMAKE_COMMAND} -E echo "Generating symbol statistics..."
            COMMAND $<TARGET_FILE:asthra> --symbol-stats --output ${DEV_TOOLS_OUTPUT_DIR}/symbol-stats.csv
            DEPENDS asthra
            COMMENT "Generate symbol table statistics"
        )
    endif()
    
    # Test Generator
    if(ENABLE_TEST_GENERATOR)
        add_custom_target(dev-test-generator
            COMMAND ${CMAKE_COMMAND} -E echo "Generating tests..."
            COMMAND $<TARGET_FILE:asthra> --generate-tests --output ${CMAKE_SOURCE_DIR}/tests/generated/
            DEPENDS asthra
            COMMENT "Generate test cases"
        )
        
        add_custom_target(dev-test-generator-fuzz
            COMMAND ${CMAKE_COMMAND} -E echo "Generating fuzz tests..."
            COMMAND $<TARGET_FILE:asthra> --generate-fuzz-tests --count 100 --output ${DEV_TOOLS_OUTPUT_DIR}/fuzz-tests/
            DEPENDS asthra
            COMMENT "Generate fuzz test cases"
        )
        
        add_custom_target(dev-test-generator-edge-cases
            COMMAND ${CMAKE_COMMAND} -E echo "Generating edge case tests..."
            COMMAND $<TARGET_FILE:asthra> --generate-edge-tests --output ${CMAKE_SOURCE_DIR}/tests/edge-cases/
            DEPENDS asthra
            COMMENT "Generate edge case tests"
        )
    endif()
    
    # Performance Profiler
    if(ENABLE_PERFORMANCE_PROFILER)
        add_custom_target(dev-performance-profiler
            COMMAND ${CMAKE_COMMAND} -E echo "Running performance profiler..."
            COMMAND $<TARGET_FILE:asthra> --profile --output ${DEV_TOOLS_OUTPUT_DIR}/profile.json
            DEPENDS asthra
            COMMENT "Profile compiler performance"
            USES_TERMINAL
        )
        
        add_custom_target(dev-performance-hotspots
            COMMAND ${CMAKE_COMMAND} -E echo "Analyzing performance hotspots..."
            COMMAND $<TARGET_FILE:asthra> --profile-hotspots --output ${DEV_TOOLS_OUTPUT_DIR}/hotspots.txt
            DEPENDS asthra
            COMMENT "Identify performance hotspots"
        )
        
        add_custom_target(dev-performance-memory
            COMMAND ${CMAKE_COMMAND} -E echo "Profiling memory usage..."
            COMMAND $<TARGET_FILE:asthra> --profile-memory --output ${DEV_TOOLS_OUTPUT_DIR}/memory-profile.json
            DEPENDS asthra
            COMMENT "Profile memory usage"
        )
        
        add_custom_target(dev-performance-report
            COMMAND ${CMAKE_COMMAND} -E echo "Generating performance report..."
            COMMAND ${CMAKE_COMMAND} -E echo "Hotspots: ${DEV_TOOLS_OUTPUT_DIR}/hotspots.txt"
            COMMAND ${CMAKE_COMMAND} -E echo "Memory: ${DEV_TOOLS_OUTPUT_DIR}/memory-profile.json"
            COMMAND ${CMAKE_COMMAND} -E echo "Full profile: ${DEV_TOOLS_OUTPUT_DIR}/profile.json"
            DEPENDS dev-performance-hotspots dev-performance-memory
            COMMENT "Generate complete performance report"
        )
    endif()
    
    # Fuzzing Harness
    if(ENABLE_FUZZING)
        # Check for fuzzing support
        if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            set(FUZZER_FLAGS "-fsanitize=fuzzer,address -g")
            
            # Lexer fuzzer
            if(TARGET asthra_lexer)
                add_executable(fuzz-lexer src/fuzzing/fuzz_lexer.c)
                target_link_libraries(fuzz-lexer asthra_lexer)
                target_compile_options(fuzz-lexer PRIVATE ${FUZZER_FLAGS})
                target_link_options(fuzz-lexer PRIVATE ${FUZZER_FLAGS})
                
                add_custom_target(run-fuzz-lexer
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEV_TOOLS_OUTPUT_DIR}/fuzz-lexer
                    COMMAND $<TARGET_FILE:fuzz-lexer> ${DEV_TOOLS_OUTPUT_DIR}/fuzz-lexer -max_total_time=60
                    DEPENDS fuzz-lexer
                    COMMENT "Run lexer fuzzer for 60 seconds"
                    USES_TERMINAL
                )
            endif()
            
            # Parser fuzzer
            if(TARGET asthra_parser)
                add_executable(fuzz-parser src/fuzzing/fuzz_parser.c)
                target_link_libraries(fuzz-parser asthra_parser)
                target_compile_options(fuzz-parser PRIVATE ${FUZZER_FLAGS})
                target_link_options(fuzz-parser PRIVATE ${FUZZER_FLAGS})
                
                add_custom_target(run-fuzz-parser
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEV_TOOLS_OUTPUT_DIR}/fuzz-parser
                    COMMAND $<TARGET_FILE:fuzz-parser> ${DEV_TOOLS_OUTPUT_DIR}/fuzz-parser -max_total_time=60
                    DEPENDS fuzz-parser
                    COMMENT "Run parser fuzzer for 60 seconds"
                    USES_TERMINAL
                )
            endif()
            
            # Combined fuzzing target
            add_custom_target(fuzz
                DEPENDS run-fuzz-lexer run-fuzz-parser
                COMMENT "Run all fuzzers"
            )
        else()
            message(STATUS "Fuzzing requires Clang compiler with libFuzzer support")
            add_custom_target(fuzz
                COMMAND ${CMAKE_COMMAND} -E echo "Fuzzing requires Clang compiler"
                COMMENT "Fuzzing not supported"
            )
        endif()
    endif()
    
    # Language Server
    if(ENABLE_LANGUAGE_SERVER)
        # Language server executable
        if(EXISTS ${CMAKE_SOURCE_DIR}/src/language_server/main.c)
            add_executable(asthra-lsp src/language_server/main.c)
            target_link_libraries(asthra-lsp asthra_compiler)
            
            add_custom_target(dev-language-server
                COMMAND $<TARGET_FILE:asthra-lsp>
                DEPENDS asthra-lsp
                COMMENT "Start Asthra Language Server"
                USES_TERMINAL
            )
            
            add_custom_target(dev-language-server-stdio
                COMMAND $<TARGET_FILE:asthra-lsp> --stdio
                DEPENDS asthra-lsp
                COMMENT "Start Asthra Language Server (stdio mode)"
                USES_TERMINAL
            )
            
            add_custom_target(dev-language-server-tcp
                COMMAND $<TARGET_FILE:asthra-lsp> --tcp --port 7777
                DEPENDS asthra-lsp
                COMMENT "Start Asthra Language Server (TCP mode on port 7777)"
                USES_TERMINAL
            )
        else()
            add_custom_target(dev-language-server
                COMMAND ${CMAKE_COMMAND} -E echo "Language server not yet implemented"
                COMMENT "Language server placeholder"
            )
        endif()
    endif()
    
    # Combined development tools targets
    add_custom_target(dev-tools
        COMMENT "Run all development tools"
    )
    
    if(ENABLE_AST_VISUALIZER)
        add_dependencies(dev-tools dev-ast-visualizer)
    endif()
    if(ENABLE_GRAMMAR_VALIDATOR)
        add_dependencies(dev-tools dev-grammar-validator)
    endif()
    if(ENABLE_SYMBOL_INSPECTOR)
        add_dependencies(dev-tools dev-symbol-inspector)
    endif()
    
    # Development diagnostics
    add_custom_target(dev-diagnostics
        COMMAND ${CMAKE_COMMAND} -E echo "=== Asthra Development Diagnostics ==="
        COMMAND $<TARGET_FILE:asthra> --diagnostics --verbose
        DEPENDS asthra
        COMMENT "Run compiler diagnostics"
        USES_TERMINAL
    )
    
    # Memory leak detector for development
    add_custom_target(dev-memcheck
        COMMAND ${CMAKE_COMMAND} -E echo "Running memory leak detection..."
        COMMAND valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes 
                $<TARGET_FILE:asthra> ${CMAKE_SOURCE_DIR}/examples/hello.ast 2>&1 | 
                tee ${DEV_TOOLS_OUTPUT_DIR}/memcheck.log || echo "valgrind not available"
        DEPENDS asthra
        COMMENT "Check for memory leaks"
        USES_TERMINAL
    )
    
    # Static analysis
    add_custom_target(dev-static-analysis
        COMMAND ${CMAKE_COMMAND} -E echo "Running static analysis..."
        COMMAND ${CMAKE_COMMAND} --build . --target analyze || echo "Static analysis not configured"
        COMMENT "Run static code analysis"
    )
    
    # Clean development tools output
    add_custom_target(dev-tools-clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEV_TOOLS_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEV_TOOLS_OUTPUT_DIR}
        COMMENT "Clean development tools output"
    )
    
else()
    # Dummy targets when dev tools are disabled
    add_custom_target(dev-tools
        COMMAND ${CMAKE_COMMAND} -E echo "Development tools are disabled. Use -DENABLE_DEV_TOOLS=ON"
        COMMENT "Development tools disabled"
    )
endif()

# Development tools help
add_custom_target(dev-tools-help
    COMMAND ${CMAKE_COMMAND} -E echo "Development Tool Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  dev-tools              - Run all development tools"
    COMMAND ${CMAKE_COMMAND} -E echo "  ast-visualizer         - Generate AST visualization"
    COMMAND ${CMAKE_COMMAND} -E echo "  ast-visualizer-png     - Convert AST to PNG"
    COMMAND ${CMAKE_COMMAND} -E echo "  grammar-validator      - Validate grammar"
    COMMAND ${CMAKE_COMMAND} -E echo "  grammar-conflicts      - Check grammar conflicts"
    COMMAND ${CMAKE_COMMAND} -E echo "  symbol-inspector       - Inspect symbol table"
    COMMAND ${CMAKE_COMMAND} -E echo "  symbol-dump            - Dump symbol table"
    COMMAND ${CMAKE_COMMAND} -E echo "  test-generator         - Generate test cases"
    COMMAND ${CMAKE_COMMAND} -E echo "  performance-profiler   - Profile performance"
    COMMAND ${CMAKE_COMMAND} -E echo "  performance-hotspots   - Find hotspots"
    COMMAND ${CMAKE_COMMAND} -E echo "  fuzz                   - Run fuzzers"
    COMMAND ${CMAKE_COMMAND} -E echo "  language-server        - Start language server"
    COMMAND ${CMAKE_COMMAND} -E echo "  dev-diagnostics        - Run diagnostics"
    COMMAND ${CMAKE_COMMAND} -E echo "  dev-memcheck           - Check memory leaks"
    COMMAND ${CMAKE_COMMAND} -E echo "  dev-static-analysis    - Static analysis"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Options:"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_DEV_TOOLS=ON          - Enable all tools"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_AST_VISUALIZER=ON     - Enable AST viz"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_FUZZING=ON            - Enable fuzzing"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_LANGUAGE_SERVER=ON    - Enable LSP"
    COMMENT "Development tools help"
)