#include "test_immutable_by_default_common.h"

// =============================================================================
// FFI INTEGRATION TESTS
// =============================================================================

bool test_ffi_immutable_integration(void) {
    printf("Testing FFI integration with immutable-by-default...\n");
    
    IntegrationTestCase test_cases[] = {
        // FFI function with immutable parameters
        {
            "extern \"C\" {\n"
            "    fn c_process_string(input: *const char) -> *const char;\n"
            "}\n"
            "\n"
            "pub fn safe_string_processing(data: string) -> string {\n"
            "    let c_string: *const char = data.as_ptr();\n"
            "    let result_ptr: *const char = c_process_string(c_string);\n"
            "    let result: string = string_from_ptr(result_ptr);\n"
            "    return result;\n"
            "}",
            "const char* c_string",
            true,
            false
        },
        
        // FFI with unsafe block for mutation
        {
            "extern \"C\" {\n"
            "    fn c_modify_buffer(buffer: *mut u8, size: usize) -> i32;\n"
            "}\n"
            "\n"
            "pub fn modify_buffer_safely(data: [u8]) -> Result<[u8], string> {\n"
            "    let mut buffer: [u8] = data;\n"
            "    \n"
            "    unsafe {\n"
            "        let result_code: i32 = c_modify_buffer(buffer.as_mut_ptr(), buffer.len());\n"
            "        \n"
            "        if result_code == 0 {\n"
            "            return Result.Ok(buffer);\n"
            "        } else {\n"
            "            let error_msg: string = \"C function failed\";\n"
            "            return Result.Err(error_msg);\n"
            "        }\n"
            "    }\n"
            "}",
            "uint8_t buffer",
            true,
            false
        },
        
        // FFI callback with immutable data
        {
            "extern \"C\" {\n"
            "    fn register_callback(callback: fn(data: *const u8, len: usize) -> i32);\n"
            "}\n"
            "\n"
            "fn process_callback_data(data: *const u8, len: usize) -> i32 {\n"
            "    let slice: [u8] = unsafe { slice_from_raw_parts(data, len) };\n"
            "    let sum: u32 = 0;\n"
            "    \n"
            "    for byte in slice {\n"
            "        sum = sum + byte as u32;\n"
            "    }\n"
            "    \n"
            "    return sum as i32;\n"
            "}\n"
            "\n"
            "pub fn setup_data_processor(none) -> none {\n"
            "    register_callback(process_callback_data);\n"
            "}",
            "uint32_t sum = 0",
            true,
            false
        }
    };
    
    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (size_t i = 0; i < num_tests; i++) {
        bool result = compile_and_validate_asthra_code(
            test_cases[i].asthra_code, 
            test_cases[i].expected_c_pattern
        );
        
        if (!result) {
            printf("  ❌ FFI integration test case %zu failed\n", i);
            return false;
        }
    }
    
    printf("  ✅ FFI immutable integration tests passed\n");
    return true;
} 
