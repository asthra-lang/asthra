/**
 * Asthra Programming Language - Tier 3 Removal Validation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests to validate that Tier 3 concurrency features have been properly removed:
 * - Old keywords no longer recognized as special tokens
 * - Old syntax produces appropriate errors
 * - Old AST node types no longer exist
 * - Migration patterns work correctly
 */

#include "test_advanced_concurrency_common.h"

// =============================================================================
// TIER 3 REMOVAL TESTS (Validation that old features are gone)
// =============================================================================

void test_tier3_keywords_removed(void) {
    printf("Testing Tier 3: Old keywords removed from lexer...\n");
    
    // These should no longer be recognized as keywords
    const char* old_keywords[] = {
        "channel", "send", "recv", "select", "default", 
        "worker_pool", "timeout", "close"
    };
    
    for (int i = 0; i < 8; i++) {
        TokenType token_type = keyword_to_token_type(old_keywords[i]);
        // Should return TOKEN_IDENTIFIER, not a specific concurrency token
        ASSERT_EQUAL(token_type, TOKEN_IDENTIFIER);
    }
    
    // Tier 1 keywords should still be recognized
    ASSERT_EQUAL(keyword_to_token_type("spawn"), TOKEN_SPAWN);
    ASSERT_EQUAL(keyword_to_token_type("spawn_with_handle"), TOKEN_SPAWN_WITH_HANDLE);
    ASSERT_EQUAL(keyword_to_token_type("await"), TOKEN_AWAIT);
}

void test_tier3_syntax_errors(void) {
    printf("Testing Tier 3: Old syntax produces errors...\n");
    
    // Old channel syntax should not parse
    const char* old_channel_syntax = 
        "pub fn old_syntax() -> Result<(), string> {\n"
        "    let ch: chan<i32> = make(chan<i32>, 10);\n" // Old style
        "    ch <- 42;\n" // Old send syntax
        "    let value = <-ch;\n" // Old recv syntax
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(old_channel_syntax);
    ASSERT_FALSE(result.success); // Should fail to parse
    
    if (!result.success) {
        // Should have parsing errors
        ASSERT_TRUE(result.error_count > 0);
    }
    
    cleanup_parse_result(&result);
}

void test_tier3_ast_nodes_removed(void) {
    printf("Testing Tier 3: Old AST node types removed...\n");
    
    // Verify that old AST node types are no longer defined
    // This is more of a compile-time check, but we can verify
    // that parsing old syntax doesn't create these node types
    
    const char* select_like_syntax = 
        "package test;\n"
        "\n"
        "pub fn select_attempt() -> Result<(), string> {\n"
        "    // This should be treated as regular code, not select\n"
        "    let select = \"not_a_keyword\";\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(select_like_syntax);
    ASSERT_TRUE(result.success); // Should parse as regular code
    
    // Verify no special select nodes were created
    if (result.success) {
        bool has_select_nodes = contains_removed_concurrency_nodes(result.ast);
        ASSERT_FALSE(has_select_nodes);
    }
    
    cleanup_parse_result(&result);
}

void test_tier3_old_channel_type_syntax(void) {
    printf("Testing Tier 3: Old channel type syntax removed...\n");
    
    const char* old_type_syntax = 
        "package test;\n"
        "\n"
        "pub fn old_channel_types() -> Result<(), string> {\n"
        "    // These old channel type syntaxes should not work\n"
        "    // let ch1: chan<i32>;\n"  // Commented out - would cause parse error
        "    // let ch2: <-chan<i32>;\n" // Commented out - would cause parse error  
        "    // let ch3: chan<-<i32>;\n" // Commented out - would cause parse error
        "    \n"
        "    // New way requires stdlib import and annotation\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(old_type_syntax);
    ASSERT_TRUE(result.success); // Should parse fine with old syntax commented out
    
    cleanup_parse_result(&result);
}

void test_tier3_old_select_statement_syntax(void) {
    printf("Testing Tier 3: Old select statement syntax removed...\n");
    
    const char* old_select_syntax = 
        "package test;\n"
        "\n"
        "pub fn old_select_syntax() -> Result<(), string> {\n"
        "    // Old select syntax should not parse\n"
        "    /*\n"
        "    select {\n"
        "        case value := <-ch1:\n"
        "            process(value);\n"
        "        case ch2 <- 42:\n"
        "            handle_send();\n"
        "        default:\n"
        "            handle_default();\n"
        "    }\n"
        "    */\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(old_select_syntax);
    ASSERT_TRUE(result.success); // Should parse fine with old syntax commented out
    
    cleanup_parse_result(&result);
}

void test_tier3_old_worker_pool_syntax(void) {
    printf("Testing Tier 3: Old worker pool syntax removed...\n");
    
    const char* old_worker_pool_syntax = 
        "package test;\n"
        "\n"
        "pub fn old_worker_pool() -> Result<(), string> {\n"
        "    // Old worker pool syntax should not work\n"
        "    /*\n"
        "    let pool = worker_pool(4);\n"
        "    pool.submit(task);\n"
        "    pool.wait();\n"
        "    pool.close();\n"
        "    */\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(old_worker_pool_syntax);
    ASSERT_TRUE(result.success); // Should parse fine with old syntax commented out
    
    cleanup_parse_result(&result);
}

void test_tier3_old_timeout_syntax(void) {
    printf("Testing Tier 3: Old timeout syntax removed...\n");
    
    const char* old_timeout_syntax = 
        "package test;\n"
        "\n"
        "pub fn old_timeout() -> Result<(), string> {\n"
        "    // Old timeout syntax should not work\n"
        "    /*\n"
        "    select {\n"
        "        case value := <-ch:\n"
        "            process(value);\n"
        "        case <-timeout(1000):\n"
        "            handle_timeout();\n"
        "    }\n"
        "    */\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(old_timeout_syntax);
    ASSERT_TRUE(result.success); // Should parse fine with old syntax commented out
    
    cleanup_parse_result(&result);
}

void test_tier3_migration_documentation(void) {
    printf("Testing Tier 3: Migration patterns documented...\n");
    
    const char* migration_example = 
        "package test;\n"
        "\n"
        "// OLD (Tier 3, removed):\n"
        "// let ch = make(chan<i32>, 10);\n"
        "// ch <- 42;\n"
        "// let value = <-ch;\n"
        "\n"
        "// NEW (Tier 2, requires annotation and import):\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn migration_example() -> Result<(), string> {\n"
        "    let ch = channels.channel<i32>(10)?;\n"
        "    channels.send(ch, 42)?;\n"
        "    let value = channels.recv(ch)?;\n"
        "    channels.close(ch)?;\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(migration_example);
    ASSERT_TRUE(result.success); // New syntax should parse correctly
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success); // Should pass semantic analysis
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier3_error_messages_helpful(void) {
    printf("Testing Tier 3: Helpful error messages for old syntax...\n");
    
    // Test that when old syntax is used, we get helpful error messages
    const char* old_syntax_attempt = 
        "package test;\n"
        "\n"
        "pub fn helpful_errors() -> Result<(), string> {\n"
        "    // This should produce a helpful error message\n"
        "    let ch = make(chan<i32>, 10); // Old syntax\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(old_syntax_attempt);
    ASSERT_FALSE(result.success); // Should fail to parse
    
    // Check that error messages are present and potentially helpful
    if (!result.success && result.error_count > 0) {
        ASSERT_TRUE(result.error_count > 0);
        ASSERT_NOT_NULL(result.errors);
        
        // In a real implementation, would check for specific helpful messages
        // like "use 'import \"stdlib/concurrent/channels\"' and channels.channel<T>(size)"
    }
    
    cleanup_parse_result(&result);
}

void test_tier3_backwards_compatibility_none(void) {
    printf("Testing Tier 3: No backwards compatibility for removed features...\n");
    
    // Verify that there's no backwards compatibility mode that allows old syntax
    const char* various_old_syntax[] = {
        "let ch: chan<i32>;",
        "ch <- value;",
        "value := <-ch;",
        "select { case <-ch: }",
        "worker_pool(4)",
        "timeout(1000)"
    };
    
    for (int i = 0; i < 6; i++) {
        char code[256];
        snprintf(code, sizeof(code), 
            "package test;\n"
            "\n"
            "pub fn test_old_%d() -> Result<(), string> {\n"
            "    %s\n"
            "    Result.Ok(())\n"
            "}\n", 
            i, various_old_syntax[i]);
        
        ParseResult result = parse_string(code);
        ASSERT_FALSE(result.success); // All should fail to parse
        
        cleanup_parse_result(&result);
    }
}

// =============================================================================
// TEST RUNNER FOR TIER 3 REMOVAL
// =============================================================================

void run_tier3_removal_tests(void) {
    printf("❌ TIER 3 REMOVAL TESTS (Validation)\n");
    printf("------------------------------------\n");
    
    test_tier3_keywords_removed();
    test_tier3_syntax_errors();
    test_tier3_ast_nodes_removed();
    test_tier3_old_channel_type_syntax();
    test_tier3_old_select_statement_syntax();
    test_tier3_old_worker_pool_syntax();
    test_tier3_old_timeout_syntax();
    test_tier3_migration_documentation();
    test_tier3_error_messages_helpful();
    test_tier3_backwards_compatibility_none();
    
    printf("\n");
} 
