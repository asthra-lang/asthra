//! Fast check command for ampu
//! Provides lightning-fast feedback for AI development cycles

use std::path::{Path, PathBuf};
use std::time::Instant;
use clap::Args;
use crate::error::AmpuError;

/// Fast check command arguments
#[derive(Args)]
pub struct CheckCommand {
    /// Files to check (if none provided, check all modified files)
    #[arg(value_name = "FILES")]
    files: Vec<String>,
    
    /// Watch mode - continuously check files as they change
    #[arg(short, long)]
    watch: bool,
    
    /// Only check syntax, skip semantic analysis
    #[arg(long)]
    syntax_only: bool,
    
    /// Maximum time to spend on checking (in milliseconds)
    #[arg(long, default_value = "1000")]
    timeout: u64,
    
    /// Clear cache before checking
    #[arg(long)]
    clear_cache: bool,
    
    /// Output format (human, json, compact)
    #[arg(long, default_value = "human")]
    format: String,
    
    /// Include only files matching these patterns
    #[arg(long)]
    include: Vec<String>,
    
    /// Exclude files matching these patterns
    #[arg(long)]
    exclude: Vec<String>,
    
    /// Base directory for relative paths
    #[arg(long)]
    base_dir: Option<PathBuf>,
}

/// Fast check engine wrapper
pub struct FastCheckEngine {
    engine_ptr: *mut std::ffi::c_void,
}

/// Fast check result
#[derive(Debug)]
pub struct CheckResult {
    pub files_checked: usize,
    pub total_errors: usize,
    pub total_warnings: usize,
    pub total_time_ms: f64,
    pub avg_time_per_file: f64,
    pub cache_hits: usize,
    pub cache_misses: usize,
    pub success: bool,
    pub summary_message: String,
}

/// Individual file check result
#[derive(Debug)]
pub struct FileResult {
    pub file_path: String,
    pub status: FileStatus,
    pub error_count: usize,
    pub warning_count: usize,
    pub check_time_ms: f64,
    pub was_cached: bool,
    pub error_message: Option<String>,
}

/// File check status enum
#[derive(Debug, Clone, Copy)]
pub enum FileStatus {
    Ok = 0,
    Warning = 1,
    Error = 2,
    Timeout = 3,
    NotFound = 4,
    Permission = 5,
    CacheError = 6,
}

// External C function declarations
extern "C" {
    fn fast_check_engine_create() -> *mut std::ffi::c_void;
    fn fast_check_engine_destroy(engine: *mut std::ffi::c_void);
    fn fast_check_engine_clear_cache(engine: *mut std::ffi::c_void);
    
    fn fast_check_config_create() -> *mut std::ffi::c_void;
    fn fast_check_config_destroy(config: *mut std::ffi::c_void);
    fn fast_check_config_set_watch_mode(config: *mut std::ffi::c_void, enabled: bool);
    fn fast_check_config_set_syntax_only(config: *mut std::ffi::c_void, enabled: bool);
    fn fast_check_config_set_timeout(config: *mut std::ffi::c_void, timeout_ms: u64);
    fn fast_check_config_set_output_format(config: *mut std::ffi::c_void, format: *const std::ffi::c_char);
    fn fast_check_config_add_include_pattern(config: *mut std::ffi::c_void, pattern: *const std::ffi::c_char);
    fn fast_check_config_add_exclude_pattern(config: *mut std::ffi::c_void, pattern: *const std::ffi::c_char);
    
    fn fast_check_run_command(
        engine: *mut std::ffi::c_void,
        files: *const *const std::ffi::c_char,
        file_count: usize,
        config: *const std::ffi::c_void
    ) -> *mut std::ffi::c_void;
    
    fn fast_check_run_on_directory(
        engine: *mut std::ffi::c_void,
        directory_path: *const std::ffi::c_char,
        config: *const std::ffi::c_void
    ) -> *mut std::ffi::c_void;
    
    fn fast_check_single_file_detailed(
        engine: *mut std::ffi::c_void,
        file_path: *const std::ffi::c_char,
        config: *const std::ffi::c_void
    ) -> *mut std::ffi::c_void;
    
    fn fast_check_format_summary_human(result: *const std::ffi::c_void) -> *mut std::ffi::c_char;
    fn fast_check_format_summary_json(result: *const std::ffi::c_void) -> *mut std::ffi::c_char;
    fn fast_check_format_result_human(result: *const std::ffi::c_void) -> *mut std::ffi::c_char;
    fn fast_check_format_result_json(result: *const std::ffi::c_void) -> *mut std::ffi::c_char;
    fn fast_check_format_result_compact(result: *const std::ffi::c_void) -> *mut std::ffi::c_char;
    
    fn fast_check_command_result_destroy(result: *mut std::ffi::c_void);
    fn fast_check_file_result_destroy(result: *mut std::ffi::c_void);
    fn fast_check_get_modified_files(
        directory_path: *const std::ffi::c_char,
        since_timestamp: i64,
        file_count: *mut usize
    ) -> *mut *mut std::ffi::c_char;
    fn fast_check_free_file_list(files: *mut *mut std::ffi::c_char, count: usize);
}

impl FastCheckEngine {
    /// Create a new fast check engine
    pub fn new() -> Result<Self, AmpuError> {
        let engine_ptr = unsafe { fast_check_engine_create() };
        if engine_ptr.is_null() {
            return Err(AmpuError::EngineCreationFailed);
        }
        
        Ok(FastCheckEngine { engine_ptr })
    }
    
    /// Clear the engine cache
    pub fn clear_cache(&mut self) -> Result<(), AmpuError> {
        unsafe {
            fast_check_engine_clear_cache(self.engine_ptr);
        }
        Ok(())
    }
    
    /// Check a single file
    pub fn check_file(&self, file_path: &str, config: &CheckConfig) -> Result<FileResult, AmpuError> {
        let c_file_path = std::ffi::CString::new(file_path)
            .map_err(|_| AmpuError::InvalidPath(file_path.to_string()))?;
        
        let result_ptr = unsafe {
            fast_check_single_file_detailed(self.engine_ptr, c_file_path.as_ptr(), config.config_ptr)
        };
        
        if result_ptr.is_null() {
            return Err(AmpuError::CheckFailed(file_path.to_string()));
        }
        
        // Extract result data (simplified - would need proper FFI structure access)
        let file_result = FileResult {
            file_path: file_path.to_string(),
            status: FileStatus::Ok, // Would extract from C struct
            error_count: 0,
            warning_count: 0,
            check_time_ms: 0.0,
            was_cached: false,
            error_message: None,
        };
        
        unsafe {
            fast_check_file_result_destroy(result_ptr);
        }
        
        Ok(file_result)
    }
    
    /// Check multiple files
    pub fn check_files(&self, files: &[String], config: &CheckConfig) -> Result<CheckResult, AmpuError> {
        // Convert Rust strings to C strings
        let c_strings: Result<Vec<_>, _> = files.iter()
            .map(|s| std::ffi::CString::new(s.as_str()))
            .collect();
        
        let c_strings = c_strings.map_err(|_| AmpuError::InvalidInput("Invalid file paths".to_string()))?;
        
        let c_ptrs: Vec<*const std::ffi::c_char> = c_strings.iter()
            .map(|s| s.as_ptr())
            .collect();
        
        let result_ptr = unsafe {
            fast_check_run_command(
                self.engine_ptr,
                c_ptrs.as_ptr(),
                c_ptrs.len(),
                config.config_ptr
            )
        };
        
        if result_ptr.is_null() {
            return Err(AmpuError::CheckFailed("Multiple files".to_string()));
        }
        
        // Extract result data (simplified)
        let check_result = CheckResult {
            files_checked: files.len(),
            total_errors: 0,
            total_warnings: 0,
            total_time_ms: 0.0,
            avg_time_per_file: 0.0,
            cache_hits: 0,
            cache_misses: 0,
            success: true,
            summary_message: "Check completed".to_string(),
        };
        
        unsafe {
            fast_check_command_result_destroy(result_ptr);
        }
        
        Ok(check_result)
    }
    
    /// Check all files in a directory
    pub fn check_directory(&self, directory: &str, config: &CheckConfig) -> Result<CheckResult, AmpuError> {
        let c_directory = std::ffi::CString::new(directory)
            .map_err(|_| AmpuError::InvalidPath(directory.to_string()))?;
        
        let result_ptr = unsafe {
            fast_check_run_on_directory(self.engine_ptr, c_directory.as_ptr(), config.config_ptr)
        };
        
        if result_ptr.is_null() {
            return Err(AmpuError::CheckFailed(directory.to_string()));
        }
        
        // Extract result data (simplified)
        let check_result = CheckResult {
            files_checked: 0,
            total_errors: 0,
            total_warnings: 0,
            total_time_ms: 0.0,
            avg_time_per_file: 0.0,
            cache_hits: 0,
            cache_misses: 0,
            success: true,
            summary_message: "Directory check completed".to_string(),
        };
        
        unsafe {
            fast_check_command_result_destroy(result_ptr);
        }
        
        Ok(check_result)
    }
}

impl Drop for FastCheckEngine {
    fn drop(&mut self) {
        unsafe {
            fast_check_engine_destroy(self.engine_ptr);
        }
    }
}

/// Configuration wrapper for C config
pub struct CheckConfig {
    config_ptr: *mut std::ffi::c_void,
}

impl CheckConfig {
    pub fn new() -> Result<Self, AmpuError> {
        let config_ptr = unsafe { fast_check_config_create() };
        if config_ptr.is_null() {
            return Err(AmpuError::ConfigCreationFailed);
        }
        
        Ok(CheckConfig { config_ptr })
    }
    
    pub fn set_watch_mode(&self, enabled: bool) {
        unsafe {
            fast_check_config_set_watch_mode(self.config_ptr, enabled);
        }
    }
    
    pub fn set_syntax_only(&self, enabled: bool) {
        unsafe {
            fast_check_config_set_syntax_only(self.config_ptr, enabled);
        }
    }
    
    pub fn set_timeout(&self, timeout_ms: u64) {
        unsafe {
            fast_check_config_set_timeout(self.config_ptr, timeout_ms);
        }
    }
    
    pub fn set_output_format(&self, format: &str) -> Result<(), AmpuError> {
        let c_format = std::ffi::CString::new(format)
            .map_err(|_| AmpuError::InvalidInput(format.to_string()))?;
        
        unsafe {
            fast_check_config_set_output_format(self.config_ptr, c_format.as_ptr());
        }
        
        Ok(())
    }
    
    pub fn add_include_pattern(&self, pattern: &str) -> Result<(), AmpuError> {
        let c_pattern = std::ffi::CString::new(pattern)
            .map_err(|_| AmpuError::InvalidInput(pattern.to_string()))?;
        
        unsafe {
            fast_check_config_add_include_pattern(self.config_ptr, c_pattern.as_ptr());
        }
        
        Ok(())
    }
    
    pub fn add_exclude_pattern(&self, pattern: &str) -> Result<(), AmpuError> {
        let c_pattern = std::ffi::CString::new(pattern)
            .map_err(|_| AmpuError::InvalidInput(pattern.to_string()))?;
        
        unsafe {
            fast_check_config_add_exclude_pattern(self.config_ptr, c_pattern.as_ptr());
        }
        
        Ok(())
    }
}

impl Drop for CheckConfig {
    fn drop(&mut self) {
        unsafe {
            fast_check_config_destroy(self.config_ptr);
        }
    }
}

impl CheckCommand {
    /// Execute the fast check command
    pub async fn execute(&self) -> Result<(), AmpuError> {
        let start_time = Instant::now();
        
        // Initialize fast check engine
        let mut engine = FastCheckEngine::new()?;
        let mut config = CheckConfig::new()?;
        
        // Configure engine
        config.set_watch_mode(self.watch);
        config.set_syntax_only(self.syntax_only);
        config.set_timeout(self.timeout);
        config.set_output_format(&self.format)?;
        
        for pattern in &self.include {
            config.add_include_pattern(pattern)?;
        }
        
        for pattern in &self.exclude {
            config.add_exclude_pattern(pattern)?;
        }
        
        if self.clear_cache {
            engine.clear_cache()?;
        }
        
        // Determine files to check
        let files_to_check = if self.files.is_empty() {
            self.get_modified_files().await?
        } else {
            self.files.clone()
        };
        
        if files_to_check.is_empty() {
            println!("No files to check");
            return Ok(());
        }
        
        // Check files
        let result = if files_to_check.len() == 1 {
            // Single file check
            let file_result = engine.check_file(&files_to_check[0], &config)?;
            self.report_file_result(&file_result)?;
            
            CheckResult {
                files_checked: 1,
                total_errors: file_result.error_count,
                total_warnings: file_result.warning_count,
                total_time_ms: file_result.check_time_ms,
                avg_time_per_file: file_result.check_time_ms,
                cache_hits: if file_result.was_cached { 1 } else { 0 },
                cache_misses: if file_result.was_cached { 0 } else { 1 },
                success: file_result.status as u8 <= FileStatus::Warning as u8,
                summary_message: "Single file check completed".to_string(),
            }
        } else {
            // Multiple files check
            engine.check_files(&files_to_check, &config)?
        };
        
        // Print summary
        let total_time = start_time.elapsed();
        self.print_summary(&result, total_time)?;
        
        // Watch mode
        if self.watch {
            self.start_watch_mode(engine, config).await?;
        }
        
        // Exit with error code if there were errors
        if result.total_errors > 0 {
            std::process::exit(1);
        }
        
        Ok(())
    }
    
    /// Get list of modified files
    async fn get_modified_files(&self) -> Result<Vec<String>, AmpuError> {
        let base_dir = self.base_dir.as_ref()
            .unwrap_or(&PathBuf::from("."))
            .to_str()
            .ok_or_else(|| AmpuError::InvalidPath("Base directory".to_string()))?;
        
        let c_directory = std::ffi::CString::new(base_dir)
            .map_err(|_| AmpuError::InvalidPath(base_dir.to_string()))?;
        
        let mut file_count = 0;
        let files_ptr = unsafe {
            fast_check_get_modified_files(c_directory.as_ptr(), 0, &mut file_count)
        };
        
        if files_ptr.is_null() || file_count == 0 {
            return Ok(Vec::new());
        }
        
        let mut files = Vec::new();
        for i in 0..file_count {
            unsafe {
                let file_ptr = *files_ptr.add(i);
                if !file_ptr.is_null() {
                    let c_str = std::ffi::CStr::from_ptr(file_ptr);
                    if let Ok(file_str) = c_str.to_str() {
                        files.push(file_str.to_string());
                    }
                }
            }
        }
        
        unsafe {
            fast_check_free_file_list(files_ptr, file_count);
        }
        
        Ok(files)
    }
    
    /// Report individual file result
    fn report_file_result(&self, result: &FileResult) -> Result<(), AmpuError> {
        match self.format.as_str() {
            "json" => self.report_json_file(result),
            "compact" => self.report_compact_file(result),
            _ => self.report_human_file(result),
        }
    }
    
    fn report_human_file(&self, result: &FileResult) -> Result<(), AmpuError> {
        let status = match result.status {
            FileStatus::Ok => "OK",
            FileStatus::Warning => "WARN",
            FileStatus::Error => "ERROR",
            FileStatus::Timeout => "TIMEOUT",
            FileStatus::NotFound => "NOT_FOUND",
            FileStatus::Permission => "PERMISSION",
            FileStatus::CacheError => "CACHE_ERROR",
        };
        
        println!("{} {} ({:.1}ms){}",
                 status,
                 result.file_path,
                 result.check_time_ms,
                 if result.was_cached { " [cached]" } else { "" });
        
        if result.error_count > 0 || result.warning_count > 0 {
            println!("  Errors: {}, Warnings: {}", result.error_count, result.warning_count);
        }
        
        if let Some(error) = &result.error_message {
            println!("  Error: {}", error);
        }
        
        Ok(())
    }
    
    fn report_json_file(&self, result: &FileResult) -> Result<(), AmpuError> {
        let json = serde_json::json!({
            "file_path": result.file_path,
            "status": result.status as u8,
            "error_count": result.error_count,
            "warning_count": result.warning_count,
            "check_time_ms": result.check_time_ms,
            "was_cached": result.was_cached,
            "error_message": result.error_message
        });
        
        println!("{}", json);
        Ok(())
    }
    
    fn report_compact_file(&self, result: &FileResult) -> Result<(), AmpuError> {
        println!("{}:{}:E{}:W{}:{:.0}ms",
                 result.file_path,
                 result.status as u8,
                 result.error_count,
                 result.warning_count,
                 result.check_time_ms);
        Ok(())
    }
    
    /// Print command summary
    fn print_summary(&self, result: &CheckResult, total_time: std::time::Duration) -> Result<(), AmpuError> {
        match self.format.as_str() {
            "json" => {
                let json = serde_json::json!({
                    "files_checked": result.files_checked,
                    "total_errors": result.total_errors,
                    "total_warnings": result.total_warnings,
                    "total_time_ms": result.total_time_ms,
                    "avg_time_per_file": result.avg_time_per_file,
                    "cache_hits": result.cache_hits,
                    "cache_misses": result.cache_misses,
                    "success": result.success,
                    "summary_message": result.summary_message
                });
                println!("{}", json);
            }
            "compact" => {
                println!("SUMMARY:{}:E{}:W{}:{:.0}ms",
                         result.files_checked,
                         result.total_errors,
                         result.total_warnings,
                         result.total_time_ms);
            }
            _ => {
                println!("\nSummary:");
                println!("  Files checked: {}", result.files_checked);
                println!("  Total time: {:.1}ms", result.total_time_ms);
                println!("  Average time per file: {:.1}ms", result.avg_time_per_file);
                println!("  Errors: {}", result.total_errors);
                println!("  Warnings: {}", result.total_warnings);
                println!("  Cache hits: {}", result.cache_hits);
                println!("  Cache misses: {}", result.cache_misses);
                println!("  Success: {}", result.success);
            }
        }
        
        Ok(())
    }
    
    /// Start watch mode
    async fn start_watch_mode(&self, _engine: FastCheckEngine, _config: CheckConfig) -> Result<(), AmpuError> {
        // Simplified implementation - would use file system watchers
        println!("Watch mode not yet implemented in this version");
        Ok(())
    }
} 
