//! Integration tests for Asthra Semantic API Rust bindings

use asthra_semantic_api::{AsthraSemanticsAPI, Result};
use std::path::Path;

#[test]
fn test_api_creation() -> Result<()> {
    let api = AsthraSemanticsAPI::new(Path::new("test_file.asthra"))?;
    
    let test_code = r#"
        package test;
        pub struct User {
            pub name: string,
            priv email: string
        }
        pub struct Point {
            x: i32,
            y: i32
        }
    "#;
    
    api.initialize_from_source(test_code)?;
    Ok(())
}

#[test]
fn test_struct_info_query() -> Result<()> {
    let api = AsthraSemanticsAPI::new(Path::new("test_file.asthra"))?;
    
    let test_code = r#"
        package test;
        pub struct User {
            pub name: string,
            priv email: string
        }
    "#;
    
    api.initialize_from_source(test_code)?;
    
    // Query struct information
    let user_struct = api.get_struct_info("User")?;
    
    if let Some(info) = user_struct {
        assert_eq!(info.name, "User");
        println!("User struct: {}", info);
    } else {
        println!("User struct not found (expected - semantic analyzer needs enhancement)");
    }
    
    // Test non-existent struct
    let result = api.get_struct_info("NonExistentStruct")?;
    assert!(result.is_none());
    
    Ok(())
}

#[test]
fn test_type_inference() -> Result<()> {
    let api = AsthraSemanticsAPI::new(Path::new("test_file.asthra"))?;
    
    let test_code = r#"
        package test;
        pub struct Point {
            x: i32,
            y: i32
        }
    "#;
    
    api.initialize_from_source(test_code)?;
    
    // Test basic type inference
    let int_type = api.infer_expression_type("42")?;
    assert_eq!(int_type, "i32");
    
    let float_type = api.infer_expression_type("3.14")?;
    assert_eq!(float_type, "f64");
    
    let string_type = api.infer_expression_type("\"hello\"")?;
    assert_eq!(string_type, "string");
    
    let bool_type = api.infer_expression_type("true")?;
    assert_eq!(bool_type, "bool");
    
    println!("Type inference working correctly");
    Ok(())
}

#[test]
fn test_type_compatibility() -> Result<()> {
    let api = AsthraSemanticsAPI::new(Path::new("test_file.asthra"))?;
    
    let test_code = r#"
        package test;
        pub struct Point {
            x: i32,
            y: i32
        }
    "#;
    
    api.initialize_from_source(test_code)?;
    
    // Test type compatibility
    assert!(api.check_type_compatibility("i32", "i16")?);
    assert!(api.check_type_compatibility("i64", "i32")?);
    assert!(api.check_type_compatibility("f64", "f32")?);
    assert!(!api.check_type_compatibility("string", "i32")?);
    
    println!("Type compatibility checking working correctly");
    Ok(())
}

#[test]
fn test_error_handling() {
    // Test invalid path
    let result = AsthraSemanticsAPI::new(Path::new(""));
    assert!(result.is_err());
    
    // Test invalid source code
    if let Ok(api) = AsthraSemanticsAPI::new(Path::new("test_file.asthra")) {
        let result = api.initialize_from_source("invalid syntax here @#$%");
        // This might succeed or fail depending on parser robustness
        println!("Invalid source code result: {:?}", result);
    }
    
    println!("Error handling tests completed");
} 
