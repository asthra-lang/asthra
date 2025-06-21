use std::env;
use std::path::PathBuf;

fn main() {
    // Get the project root directory
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let manifest_path = PathBuf::from(&manifest_dir);
    let project_root = manifest_path
        .parent()
        .unwrap()
        .parent()
        .unwrap()
        .parent()
        .unwrap();
    
    println!("cargo:rerun-if-changed=../../semantic_api_core.c");
    println!("cargo:rerun-if-changed=../../symbol_queries.c");
    println!("cargo:rerun-if-changed=../../type_queries.c");
    println!("cargo:rerun-if-changed=../../ast_navigation.c");
    
    // Add include paths
    let include_paths = vec![
        project_root.join("src"),
        project_root.join("src/ai_api"),
        project_root.join("src/analysis"),
        project_root.join("src/parser"),
        project_root.join("src/lexer"),
        project_root.join("src/tools/common"),
    ];
    
    // Build the C API - use pre-compiled object files
    let mut build = cc::Build::new();
    build
        // Use pre-compiled AI API object files
        .object(project_root.join("build/src/ai_api/semantic_api_core.o"))
        .object(project_root.join("build/src/ai_api/symbol_queries.o"))
        .object(project_root.join("build/src/ai_api/type_queries.o"))
        .object(project_root.join("build/src/ai_api/ast_navigation.o"))
        .flag("-std=c17")
        .flag("-Wall")
        .flag("-Wextra")
        .flag("-pthread");
    
    for path in &include_paths {
        build.include(path);
    }
    
    // Add conditional compilation flags
    if cfg!(debug_assertions) {
        build.flag("-DDEBUG");
        build.flag("-g");
    } else {
        build.flag("-DNDEBUG");
        build.flag("-O2");
    }
    
    build.compile("asthra_semantic_api");
    
    // Link required libraries
    println!("cargo:rustc-link-lib=pthread");
    println!("cargo:rustc-link-lib=m");
    
    // Tell cargo where to find the compiled library
    println!("cargo:rustc-link-search=native={}", project_root.join("target").display());
} 
