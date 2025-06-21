# Fast Check and AI Integration for Asthra
# Provides fast compilation checking, AI-powered linting, code suggestions, and API integration

option(ENABLE_FAST_CHECK "Enable fast check system" ON)
option(ENABLE_AI_INTEGRATION "Enable AI-powered features" ON)
option(ENABLE_AI_LINTER "Enable AI-powered linting" ON)
option(ENABLE_AI_API "Enable AI API integration" ON)
option(ENABLE_AI_ANNOTATIONS "Enable AI annotations" ON)

# Fast check configuration
set(FAST_CHECK_TIMEOUT "5" CACHE STRING "Fast check timeout in seconds")
set(FAST_CHECK_CACHE_SIZE "100" CACHE STRING "Fast check cache size in MB")
set(AI_MODEL "gpt-3.5-turbo" CACHE STRING "AI model to use")
set(AI_API_KEY "" CACHE STRING "AI API key (optional)")

# Fast check and AI output directory
set(FAST_CHECK_OUTPUT_DIR "${CMAKE_BINARY_DIR}/fast-check" CACHE PATH "Fast check output directory")
file(MAKE_DIRECTORY ${FAST_CHECK_OUTPUT_DIR})

if(ENABLE_FAST_CHECK)
    message(STATUS "Fast check system enabled")
    
    # Fast check executable
    if(TARGET asthra_fast_check)
        # Basic fast check
        add_custom_target(fast-check
            COMMAND $<TARGET_FILE:asthra_fast_check> --timeout ${FAST_CHECK_TIMEOUT}
            DEPENDS asthra_fast_check
            COMMENT "Run fast compilation check"
            USES_TERMINAL
        )
        
        # Fast check with caching
        add_custom_target(fast-check-cached
            COMMAND $<TARGET_FILE:asthra_fast_check> 
                    --cache-dir ${FAST_CHECK_OUTPUT_DIR}/cache
                    --cache-size ${FAST_CHECK_CACHE_SIZE}
                    --timeout ${FAST_CHECK_TIMEOUT}
            DEPENDS asthra_fast_check
            COMMENT "Run fast check with caching"
            USES_TERMINAL
        )
        
        # Fast check specific file
        add_custom_target(fast-check-file
            COMMAND ${CMAKE_COMMAND} -E echo "Usage: make fast-check-file FILE=path/to/file.ast"
            COMMAND $<TARGET_FILE:asthra_fast_check> $ENV{FILE}
            DEPENDS asthra_fast_check
            COMMENT "Fast check specific file"
            USES_TERMINAL
        )
        
        # Fast check watch mode
        add_custom_target(fast-check-watch
            COMMAND $<TARGET_FILE:asthra_fast_check> --watch --interval 1
            DEPENDS asthra_fast_check
            COMMENT "Run fast check in watch mode"
            USES_TERMINAL
        )
        
        # Fast check incremental
        add_custom_target(fast-check-incremental
            COMMAND $<TARGET_FILE:asthra_fast_check> 
                    --incremental
                    --state-file ${FAST_CHECK_OUTPUT_DIR}/incremental.state
            DEPENDS asthra_fast_check
            COMMENT "Run incremental fast check"
            USES_TERMINAL
        )
    else()
        add_custom_target(fast-check
            COMMAND ${CMAKE_COMMAND} -E echo "Fast check not built. Building now..."
            COMMAND ${CMAKE_COMMAND} --build . --target asthra_fast_check
            COMMENT "Fast check not available"
        )
    endif()
    
    # AI Integration
    if(ENABLE_AI_INTEGRATION)
        message(STATUS "AI integration enabled")
        
        # AI Linter
        if(ENABLE_AI_LINTER AND TARGET asthra_ai_linter)
            add_custom_target(ai-lint
                COMMAND $<TARGET_FILE:asthra_ai_linter> 
                        --model ${AI_MODEL}
                        --output ${FAST_CHECK_OUTPUT_DIR}/ai-lint-report.json
                DEPENDS asthra_ai_linter
                COMMENT "Run AI-powered linting"
                USES_TERMINAL
            )
            
            add_custom_target(ai-lint-fix
                COMMAND $<TARGET_FILE:asthra_ai_linter>
                        --fix
                        --model ${AI_MODEL}
                        --backup-dir ${FAST_CHECK_OUTPUT_DIR}/backups
                DEPENDS asthra_ai_linter
                COMMENT "Run AI linting with auto-fix"
                USES_TERMINAL
            )
            
            add_custom_target(ai-lint-explain
                COMMAND $<TARGET_FILE:asthra_ai_linter>
                        --explain
                        --verbose
                        --model ${AI_MODEL}
                DEPENDS asthra_ai_linter
                COMMENT "Run AI linting with explanations"
                USES_TERMINAL
            )
            
            # AI linting rules
            add_custom_target(ai-lint-security
                COMMAND $<TARGET_FILE:asthra_ai_linter>
                        --rules security
                        --severity high
                DEPENDS asthra_ai_linter
                COMMENT "Run AI security linting"
            )
            
            add_custom_target(ai-lint-performance
                COMMAND $<TARGET_FILE:asthra_ai_linter>
                        --rules performance
                        --suggest-optimizations
                DEPENDS asthra_ai_linter
                COMMENT "Run AI performance linting"
            )
            
            add_custom_target(ai-lint-style
                COMMAND $<TARGET_FILE:asthra_ai_linter>
                        --rules style
                        --config ${CMAKE_SOURCE_DIR}/.asthra-style.yaml
                DEPENDS asthra_ai_linter
                COMMENT "Run AI style linting"
            )
        endif()
        
        # AI API
        if(ENABLE_AI_API AND TARGET asthra_ai_api)
            add_custom_target(ai-api-server
                COMMAND $<TARGET_FILE:asthra_ai_api>
                        --port 8080
                        --model ${AI_MODEL}
                        $<$<NOT:$<STREQUAL:${AI_API_KEY},"">>:--api-key ${AI_API_KEY}>
                DEPENDS asthra_ai_api
                COMMENT "Start AI API server"
                USES_TERMINAL
            )
            
            add_custom_target(ai-complete
                COMMAND ${CMAKE_COMMAND} -E echo "Code completion endpoint: http://localhost:8080/complete"
                COMMAND curl -X POST http://localhost:8080/complete -d "{\"code\": \"fn main\", \"position\": 7}" || echo "AI API server not running"
                COMMENT "Test AI code completion"
            )
            
            add_custom_target(ai-suggest
                COMMAND ${CMAKE_COMMAND} -E echo "Code suggestion endpoint: http://localhost:8080/suggest"
                COMMAND curl -X POST http://localhost:8080/suggest -d "{\"code\": \"// TODO: implement\", \"context\": \"parser\"}" || echo "AI API server not running"
                COMMENT "Test AI code suggestions"
            )
            
            add_custom_target(ai-explain
                COMMAND ${CMAKE_COMMAND} -E echo "Code explanation endpoint: http://localhost:8080/explain"
                COMMAND curl -X POST http://localhost:8080/explain -d "{\"code\": \"fn factorial(n: i32)\", \"verbose\": true}" || echo "AI API server not running"
                COMMENT "Test AI code explanation"
            )
        endif()
        
        # AI Annotations
        if(ENABLE_AI_ANNOTATIONS)
            add_custom_target(ai-annotate
                COMMAND $<TARGET_FILE:asthra>
                        --ai-annotate
                        --output ${FAST_CHECK_OUTPUT_DIR}/annotations.json
                DEPENDS asthra
                COMMENT "Generate AI annotations"
            )
            
            add_custom_target(ai-annotate-types
                COMMAND $<TARGET_FILE:asthra>
                        --ai-annotate-types
                        --infer-missing
                DEPENDS asthra
                COMMENT "AI-powered type annotations"
            )
            
            add_custom_target(ai-annotate-docs
                COMMAND $<TARGET_FILE:asthra>
                        --ai-generate-docs
                        --format markdown
                        --output ${CMAKE_SOURCE_DIR}/docs/ai-generated/
                DEPENDS asthra
                COMMENT "Generate AI documentation"
            )
        endif()
        
        # Combined AI features
        add_custom_target(ai-analyze
            COMMENT "Run complete AI analysis"
        )
        
        if(TARGET ai-lint)
            add_dependencies(ai-analyze ai-lint)
        endif()
        if(TARGET ai-annotate)
            add_dependencies(ai-analyze ai-annotate)
        endif()
        
        # AI training data generation
        add_custom_target(ai-generate-training-data
            COMMAND ${CMAKE_COMMAND} -E echo "Generating AI training data..."
            COMMAND $<TARGET_FILE:asthra>
                    --generate-training-data
                    --format jsonl
                    --output ${FAST_CHECK_OUTPUT_DIR}/training-data.jsonl
            DEPENDS asthra
            COMMENT "Generate AI training data"
        )
    endif()
    
    # Fast check benchmarks and additional targets
    if(TARGET asthra_fast_check)
        add_custom_target(fast-check-benchmark
            COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking fast check..."
            COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra_fast_check>
                    --benchmark
                    --iterations 1000
                    --output ${FAST_CHECK_OUTPUT_DIR}/benchmark.json
            DEPENDS asthra_fast_check
            COMMENT "Benchmark fast check performance"
        )
        
        # Fast check statistics
        add_custom_target(fast-check-stats
            COMMAND $<TARGET_FILE:asthra_fast_check>
                    --stats
                    --cache-dir ${FAST_CHECK_OUTPUT_DIR}/cache
            DEPENDS asthra_fast_check
            COMMENT "Display fast check statistics"
        )
        
        # Fast parallel check
        add_custom_target(fast-check-parallel
            COMMAND $<TARGET_FILE:asthra_fast_check>
                    --parallel $(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
                    --timeout ${FAST_CHECK_TIMEOUT}
            DEPENDS asthra_fast_check
            COMMENT "Run fast check with parallel processing"
            USES_TERMINAL
        )
    endif()
    
    # Clean fast check cache (doesn't depend on asthra_fast_check)
    add_custom_target(fast-check-clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${FAST_CHECK_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${FAST_CHECK_OUTPUT_DIR}
        COMMENT "Clean fast check cache and output"
    )
    
    # IDE integration helpers
    add_custom_target(ide-setup
        COMMAND ${CMAKE_COMMAND} -E echo "Setting up IDE integration..."
        COMMAND ${CMAKE_COMMAND} -E echo "VSCode: Install 'Asthra Language Support' extension"
        COMMAND ${CMAKE_COMMAND} -E echo "IntelliJ: Import project as CMake project"
        COMMAND ${CMAKE_COMMAND} -E echo "Emacs: Add asthra-mode to init.el"
        COMMAND ${CMAKE_COMMAND} -E echo "Vim: Install asthra.vim plugin"
        COMMENT "IDE setup instructions"
    )
    
else()
    # Dummy targets when fast check is disabled
    add_custom_target(fast-check
        COMMAND ${CMAKE_COMMAND} -E echo "Fast check is disabled. Use -DENABLE_FAST_CHECK=ON"
        COMMENT "Fast check disabled"
    )
    
    add_custom_target(ai-lint
        COMMAND ${CMAKE_COMMAND} -E echo "AI features are disabled. Use -DENABLE_AI_INTEGRATION=ON"
        COMMENT "AI integration disabled"
    )
endif()

# Fast check and AI help
add_custom_target(fast-check-help
    COMMAND ${CMAKE_COMMAND} -E echo "Fast Check & AI Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  fast-check             - Quick compilation check"
    COMMAND ${CMAKE_COMMAND} -E echo "  fast-check-cached      - Fast check with cache"
    COMMAND ${CMAKE_COMMAND} -E echo "  fast-check-watch       - Watch mode"
    COMMAND ${CMAKE_COMMAND} -E echo "  fast-check-incremental - Incremental checking"
    COMMAND ${CMAKE_COMMAND} -E echo "  fast-check-parallel    - Parallel checking"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "AI Features:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-lint                - AI-powered linting"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-lint-fix            - Auto-fix issues"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-lint-security       - Security analysis"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-lint-performance    - Performance analysis"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-api-server          - Start AI API server"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-complete            - Code completion"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-annotate            - Generate annotations"
    COMMAND ${CMAKE_COMMAND} -E echo "  ai-analyze             - Complete AI analysis"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Options:"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_FAST_CHECK=ON       - Enable fast check"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_AI_INTEGRATION=ON   - Enable AI features"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DFAST_CHECK_TIMEOUT=5       - Check timeout"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DAI_MODEL=gpt-3.5-turbo     - AI model to use"
    COMMENT "Fast check and AI help"
)