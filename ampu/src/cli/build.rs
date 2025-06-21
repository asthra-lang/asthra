use crate::error::Result;
use std::env;
use std::path::PathBuf;
use tracing::{info, warn};

pub async fn handle_build(release: bool, target: Option<String>, jobs: Option<usize>) -> Result<()> {
    info!("Starting build process...");
    
    let current_dir = env::current_dir()?;
    let asthra_toml_path = current_dir.join("asthra.toml");
    
    if !asthra_toml_path.exists() {
        return Err(crate::error::BuildError::InvalidConfiguration(
            "No asthra.toml found in current directory. Run 'ampu init' to create a new project.".to_string()
        ));
    }
    
    // Parse asthra.toml
    let asthra_mod = crate::config::asthra_toml::AsthraTOMLParser::parse_file(&asthra_toml_path).await?;
    info!("Parsed asthra.toml for package: {}", asthra_mod.package.name);
    
    // Create build configuration
    let profile = if release { 
        crate::storage::BuildProfile::Release 
    } else { 
        crate::storage::BuildProfile::Debug 
    };
    
    let mut full_build_config = crate::compiler::BuildConfig::new(current_dir.clone(), profile)?;
    
    // Override with command line options
    if let Some(target_str) = target {
        full_build_config.target = match target_str.as_str() {
            "native" => crate::config::Target::Native,
            "x86_64" => crate::config::Target::X86_64,
            "arm64" => crate::config::Target::Arm64,
            "wasm32" => crate::config::Target::Wasm32,
            _ => return Err(crate::error::BuildError::InvalidConfiguration(
                format!("Unknown target: {}", target_str)
            )),
        };
    }
    
    if let Some(jobs_count) = jobs {
        full_build_config.parallel_jobs = jobs_count;
    }
    
    // Validate imports and access control
    info!("Validating imports and access control...");
    let validator = crate::resolver::access_control::BuildValidator::new()?;
    validator.validate_project(&current_dir).await?;
    info!("✅ Import validation passed");
    
    // Create build orchestrator
    let mut orchestrator = crate::compiler::BuildOrchestrator::new(full_build_config);
    
    // Execute build
    let build_result = orchestrator.build_project(&current_dir).await?;
    
    info!("✅ Build completed successfully!");
    info!("Output: {}", build_result.output_path.display());
    info!("Compiled {} packages", build_result.compiled_packages.len());
    info!("Build time: {:.2}s", build_result.build_time.as_secs_f64());
    info!("Cache hits: {}, Cache misses: {}", build_result.cache_hits, build_result.cache_misses);
    
    // Print build statistics
    print_build_stats(&build_result);
    
    Ok(())
}

fn print_build_stats(build_result: &crate::compiler::BuildResult) {
    info!("Build Statistics:");
    info!("  Compiled packages: {}", build_result.compiled_packages.len());
    
    for package_result in &build_result.compiled_packages {
        info!("  📦 {}", package_result.package.name);
        if !package_result.warnings.is_empty() {
            warn!("    ⚠️  {} warnings", package_result.warnings.len());
        }
    }
}

// Add num_cpus to Cargo.toml dependencies for parallel job detection
// This is a placeholder - in a real implementation you'd add this dependency 
