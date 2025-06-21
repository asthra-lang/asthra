/// CLI handler for the fmt command
/// Provides the command-line interface for the Asthra formatter

use crate::error::Result;
use crate::formatter::{Formatter, FormatConfig, load_config_from_project};
use std::env;
use std::path::Path;
use std::process;
use tracing::{info, warn, error};

/// Handle the fmt command
pub async fn handle_fmt(
    check: bool,
    diff: bool,
    exclude: Vec<String>,
    include: Vec<String>,
    paths: Vec<String>,
) -> Result<()> {
    info!("Starting Asthra formatter");

    // Get current working directory
    let current_dir = env::current_dir()
        .map_err(|e| crate::error::BuildError::Io(format!("Failed to get current directory: {}", e)))?;

    // Load configuration from project
    let mut config = load_config_from_project(&current_dir).await?;

    // Override config with command-line options
    if !exclude.is_empty() {
        config.exclude.extend(exclude);
    }
    if !include.is_empty() {
        config.include = include;
    }

    // Create formatter
    let formatter = Formatter::with_config(config);

    // Determine operation mode
    let show_diff = diff;
    let check_only = check || diff;

    // Format files
    let result = formatter.format_paths(&paths, check_only, show_diff).await?;

    // Report results
    report_results(&result, check_only, show_diff).await?;

    // Exit with appropriate code
    if check_only && !result.changed_files.is_empty() {
        info!("Files need formatting");
        process::exit(1);
    } else if !result.error_files.is_empty() {
        error!("Formatting completed with errors");
        process::exit(1);
    } else {
        info!("Formatting completed successfully");
        Ok(())
    }
}

/// Report formatting results to the user
async fn report_results(
    result: &crate::formatter::FormatResult,
    check_only: bool,
    show_diff: bool,
) -> Result<()> {
    if result.total_files == 0 {
        warn!("No Asthra files found to format");
        return Ok(());
    }

    info!("Processed {} files", result.total_files);

    if !result.error_files.is_empty() {
        error!("Failed to format {} files:", result.error_files.len());
        for (file, error) in &result.error_files {
            error!("  {}: {}", file, error);
        }
    }

    if check_only {
        if result.changed_files.is_empty() {
            info!("All files are properly formatted");
        } else {
            warn!("The following files need formatting:");
            for file in &result.changed_files {
                warn!("  {}", file);
            }
            
            if !show_diff {
                info!("Run 'ampu fmt --diff' to see the changes needed");
                info!("Run 'ampu fmt' to apply the formatting");
            }
        }
    } else {
        if result.changed_files.is_empty() {
            info!("No files needed formatting");
        } else {
            info!("Formatted {} files:", result.changed_files.len());
            for file in &result.changed_files {
                info!("  {}", file);
            }
        }
    }

    Ok(())
}

/// Validate that we're in an Asthra project
async fn validate_asthra_project() -> Result<()> {
    let current_dir = env::current_dir()
        .map_err(|e| crate::error::BuildError::Io(format!("Failed to get current directory: {}", e)))?;
    
    crate::validate_project(&current_dir).await
}

/// Get default paths to format if none specified
fn get_default_format_paths() -> Vec<String> {
    let mut paths = Vec::new();
    
    // Check if src/ directory exists
    if Path::new("src").exists() {
        paths.push("src".to_string());
    }
    
    // Check if tests/ directory exists
    if Path::new("tests").exists() {
        paths.push("tests".to_string());
    }
    
    // If no standard directories found, format current directory
    if paths.is_empty() {
        paths.push(".".to_string());
    }
    
    paths
}

/// Check if the formatter should run in project mode
async fn should_use_project_mode(paths: &[String]) -> bool {
    // If no paths specified, we're in project mode
    if paths.is_empty() {
        return true;
    }
    
    // If paths are just "." or standard project directories, use project mode
    if paths.len() == 1 {
        match paths[0].as_str() {
            "." | "src" | "tests" => return true,
            _ => {}
        }
    }
    
    // Check if we're in an Asthra project
    validate_asthra_project().await.is_ok()
}

/// Enhanced fmt handler with better project integration
pub async fn handle_fmt_enhanced(
    check: bool,
    diff: bool,
    exclude: Vec<String>,
    include: Vec<String>,
    mut paths: Vec<String>,
) -> Result<()> {
    info!("Starting Asthra formatter");

    // Determine if we should use project mode
    let use_project_mode = should_use_project_mode(&paths).await;
    
    if use_project_mode {
        // Validate we're in an Asthra project
        if let Err(e) = validate_asthra_project().await {
            warn!("Not in an Asthra project: {}", e);
            warn!("Formatting files without project configuration");
        }
        
        // Use default paths if none specified
        if paths.is_empty() {
            paths = get_default_format_paths();
            info!("No paths specified, formatting default directories: {:?}", paths);
        }
    }

    // Get current working directory
    let current_dir = env::current_dir()
        .map_err(|e| crate::error::BuildError::Io(format!("Failed to get current directory: {}", e)))?;

    // Load configuration
    let mut config = if use_project_mode {
        load_config_from_project(&current_dir).await.unwrap_or_default()
    } else {
        FormatConfig::default()
    };

    // Override config with command-line options
    if !exclude.is_empty() {
        config.exclude.extend(exclude);
    }
    if !include.is_empty() {
        config.include = include;
    }

    // Create formatter
    let formatter = Formatter::with_config(config);

    // Determine operation mode
    let show_diff = diff;
    let check_only = check || diff;

    // Format files
    let result = formatter.format_paths(&paths, check_only, show_diff).await?;

    // Report results
    report_results(&result, check_only, show_diff).await?;

    // Exit with appropriate code
    if check_only && !result.changed_files.is_empty() {
        info!("Files need formatting");
        process::exit(1);
    } else if !result.error_files.is_empty() {
        error!("Formatting completed with errors");
        process::exit(1);
    } else {
        info!("Formatting completed successfully");
        Ok(())
    }
}

/// Integration with build system
pub async fn check_formatting_in_build() -> Result<bool> {
    info!("Checking code formatting as part of build");
    
    let current_dir = env::current_dir()
        .map_err(|e| crate::error::BuildError::Io(format!("Failed to get current directory: {}", e)))?;

    let config = load_config_from_project(&current_dir).await?;
    let formatter = Formatter::with_config(config);
    
    let paths = get_default_format_paths();
    let result = formatter.format_paths(&paths, true, false).await?;
    
    if !result.changed_files.is_empty() {
        error!("Code formatting check failed. The following files need formatting:");
        for file in &result.changed_files {
            error!("  {}", file);
        }
        error!("Run 'ampu fmt' to fix formatting issues");
        return Ok(false);
    }
    
    if !result.error_files.is_empty() {
        error!("Code formatting check failed due to parse errors:");
        for (file, error) in &result.error_files {
            error!("  {}: {}", file, error);
        }
        return Ok(false);
    }
    
    info!("Code formatting check passed");
    Ok(true)
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;
    use tokio::fs;

    #[tokio::test]
    async fn test_get_default_format_paths() {
        let temp_dir = TempDir::new().unwrap();
        let old_dir = env::current_dir().unwrap();
        
        // Change to temp directory
        env::set_current_dir(temp_dir.path()).unwrap();
        
        // Test with no src or tests directories
        let paths = get_default_format_paths();
        assert_eq!(paths, vec!["."]);
        
        // Create src directory
        fs::create_dir("src").await.unwrap();
        let paths = get_default_format_paths();
        assert!(paths.contains(&"src".to_string()));
        
        // Create tests directory
        fs::create_dir("tests").await.unwrap();
        let paths = get_default_format_paths();
        assert!(paths.contains(&"src".to_string()));
        assert!(paths.contains(&"tests".to_string()));
        
        // Restore original directory
        env::set_current_dir(old_dir).unwrap();
    }

    #[tokio::test]
    async fn test_should_use_project_mode() {
        // Empty paths should use project mode
        assert!(should_use_project_mode(&[]).await);
        
        // Standard paths should use project mode
        assert!(should_use_project_mode(&["src".to_string()]).await);
        assert!(should_use_project_mode(&["tests".to_string()]).await);
        assert!(should_use_project_mode(&[".".to_string()]).await);
        
        // Specific files should not necessarily use project mode
        // (depends on whether we're in an Asthra project)
        let specific_file = vec!["some_file.asthra".to_string()];
        // This will depend on the current directory, so we can't assert a specific result
        let _ = should_use_project_mode(&specific_file).await;
    }
} 
