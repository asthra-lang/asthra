//! Rust bindings for Asthra Enhanced Diagnostics
//! Provides safe, idiomatic Rust interface for AI tools

use std::ffi::{CStr, CString};
use std::ptr;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DiagnosticSpan {
    pub start_line: usize,
    pub start_column: usize,
    pub end_line: usize,
    pub end_column: usize,
    pub file_path: String,
    pub label: Option<String>,
    pub snippet: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DiagnosticSuggestion {
    #[serde(rename = "type")]
    pub suggestion_type: String,
    pub text: String,
    pub confidence: String,
    pub rationale: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DiagnosticMetadata {
    pub inferred_types: Option<Vec<String>>,
    pub available_methods: Option<Vec<String>>,
    pub similar_symbols: Option<Vec<String>>,
    pub error_category: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct EnhancedDiagnostic {
    pub code: String,
    pub level: String,
    pub message: String,
    pub spans: Vec<DiagnosticSpan>,
    pub suggestions: Vec<DiagnosticSuggestion>,
    pub metadata: Option<DiagnosticMetadata>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct DiagnosticsOutput {
    pub diagnostics: Vec<EnhancedDiagnostic>,
}

/// Main API for interacting with Asthra's enhanced diagnostic system
pub struct AsthraDiagnosticAPI {
    diagnostics: Vec<EnhancedDiagnostic>,
}

impl AsthraDiagnosticAPI {
    /// Parse Asthra compiler JSON output into structured diagnostics
    pub fn parse_compiler_output(json: &str) -> Result<Self, serde_json::Error> {
        let output: DiagnosticsOutput = serde_json::from_str(json)?;
        Ok(AsthraDiagnosticAPI { 
            diagnostics: output.diagnostics 
        })
    }
    
    /// Get all error-level diagnostics
    pub fn get_errors(&self) -> Vec<&EnhancedDiagnostic> {
        self.diagnostics
            .iter()
            .filter(|d| d.level == "error")
            .collect()
    }
    
    /// Get all warning-level diagnostics
    pub fn get_warnings(&self) -> Vec<&EnhancedDiagnostic> {
        self.diagnostics
            .iter()
            .filter(|d| d.level == "warning")
            .collect()
    }
    
    /// Get suggestions with high confidence scores
    pub fn get_high_confidence_suggestions(&self) -> Vec<&DiagnosticSuggestion> {
        self.diagnostics
            .iter()
            .flat_map(|d| &d.suggestions)
            .filter(|s| s.confidence == "high")
            .collect()
    }
    
    /// Get suggestions with medium or high confidence
    pub fn get_actionable_suggestions(&self) -> Vec<&DiagnosticSuggestion> {
        self.diagnostics
            .iter()
            .flat_map(|d| &d.suggestions)
            .filter(|s| s.confidence == "high" || s.confidence == "medium")
            .collect()
    }
    
    /// Apply a single suggestion to source code
    pub fn apply_suggestion(&self, suggestion: &DiagnosticSuggestion, source: &str) -> Result<String, String> {
        match suggestion.suggestion_type.as_str() {
            "replace" => {
                // For now, simple text replacement
                // In production, would need more sophisticated AST-based replacement
                Ok(source.replace("undefined_variable", &suggestion.text))
            }
            "insert" => {
                // Simple insertion at the beginning
                // In production, would need position-aware insertion
                Ok(format!("{}{}", suggestion.text, source))
            }
            "delete" => {
                // Simple deletion logic
                Ok(source.replace(&suggestion.text, ""))
            }
            _ => Err(format!("Unsupported suggestion type: {}", suggestion.suggestion_type))
        }
    }
    
    /// Apply all high-confidence suggestions to source code
    pub fn apply_all_high_confidence_fixes(&self, source: &str) -> Result<String, String> {
        let mut modified_source = source.to_string();
        let high_confidence_fixes = self.get_high_confidence_suggestions();
        
        // Apply fixes in reverse order to maintain positions
        for fix in high_confidence_fixes.iter().rev() {
            modified_source = self.apply_suggestion(fix, &modified_source)?;
        }
        
        Ok(modified_source)
    }
    
    /// Get statistics about the diagnostics
    pub fn get_statistics(&self) -> DiagnosticStatistics {
        let error_count = self.get_errors().len();
        let warning_count = self.get_warnings().len();
        let high_confidence_suggestions = self.get_high_confidence_suggestions().len();
        let total_suggestions = self.diagnostics
            .iter()
            .map(|d| d.suggestions.len())
            .sum();
        
        DiagnosticStatistics {
            total_diagnostics: self.diagnostics.len(),
            error_count,
            warning_count,
            total_suggestions,
            high_confidence_suggestions,
            has_errors: error_count > 0,
        }
    }
    
    /// Extract data useful for AI model training and analysis
    pub fn get_ai_learning_data(&self) -> AILearningData {
        let error_patterns: Vec<String> = self.get_errors()
            .iter()
            .map(|d| d.code.clone())
            .collect();
        
        let successful_suggestions: Vec<DiagnosticSuggestion> = self.get_high_confidence_suggestions()
            .iter()
            .map(|&s| s.clone())
            .collect();
        
        let common_mistakes = self.analyze_common_patterns();
        let fix_success_rate = self.calculate_fix_success_rate();
        
        AILearningData {
            error_patterns,
            successful_suggestions,
            common_mistakes,
            fix_success_rate,
            metadata_insights: self.extract_metadata_insights(),
        }
    }
    
    /// Analyze common error patterns
    fn analyze_common_patterns(&self) -> HashMap<String, usize> {
        let mut patterns = HashMap::new();
        
        for diagnostic in &self.diagnostics {
            *patterns.entry(diagnostic.code.clone()).or_insert(0) += 1;
        }
        
        patterns
    }
    
    /// Calculate estimated fix success rate based on confidence levels
    fn calculate_fix_success_rate(&self) -> f64 {
        let total_suggestions = self.diagnostics
            .iter()
            .map(|d| d.suggestions.len())
            .sum::<usize>();
        
        if total_suggestions == 0 {
            return 0.0;
        }
        
        let weighted_score: f64 = self.diagnostics
            .iter()
            .flat_map(|d| &d.suggestions)
            .map(|s| match s.confidence.as_str() {
                "high" => 0.9,
                "medium" => 0.7,
                "low" => 0.4,
                _ => 0.2,
            })
            .sum();
        
        weighted_score / total_suggestions as f64
    }
    
    /// Extract insights from diagnostic metadata
    fn extract_metadata_insights(&self) -> HashMap<String, serde_json::Value> {
        let mut insights = HashMap::new();
        
        let mut type_frequency = HashMap::new();
        let mut symbol_frequency = HashMap::new();
        
        for diagnostic in &self.diagnostics {
            if let Some(metadata) = &diagnostic.metadata {
                if let Some(types) = &metadata.inferred_types {
                    for type_name in types {
                        *type_frequency.entry(type_name.clone()).or_insert(0) += 1;
                    }
                }
                
                if let Some(symbols) = &metadata.similar_symbols {
                    for symbol in symbols {
                        *symbol_frequency.entry(symbol.clone()).or_insert(0) += 1;
                    }
                }
            }
        }
        
        insights.insert("common_types".to_string(), 
                       serde_json::to_value(type_frequency).unwrap_or_default());
        insights.insert("common_symbols".to_string(), 
                       serde_json::to_value(symbol_frequency).unwrap_or_default());
        
        insights
    }
    
    /// Get all diagnostics
    pub fn get_all_diagnostics(&self) -> &Vec<EnhancedDiagnostic> {
        &self.diagnostics
    }
    
    /// Filter diagnostics by error code
    pub fn filter_by_code(&self, code: &str) -> Vec<&EnhancedDiagnostic> {
        self.diagnostics
            .iter()
            .filter(|d| d.code == code)
            .collect()
    }
    
    /// Check if compilation was successful (no errors)
    pub fn is_compilation_successful(&self) -> bool {
        self.get_errors().is_empty()
    }
}

#[derive(Debug, Serialize)]
pub struct DiagnosticStatistics {
    pub total_diagnostics: usize,
    pub error_count: usize,
    pub warning_count: usize,
    pub total_suggestions: usize,
    pub high_confidence_suggestions: usize,
    pub has_errors: bool,
}

#[derive(Debug, Serialize)]
pub struct AILearningData {
    pub error_patterns: Vec<String>,
    pub successful_suggestions: Vec<DiagnosticSuggestion>,
    pub common_mistakes: HashMap<String, usize>,
    pub fix_success_rate: f64,
    pub metadata_insights: HashMap<String, serde_json::Value>,
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_parse_compiler_output() {
        let json = r#"
        {
            "diagnostics": [
                {
                    "code": "ASTHRA_E001",
                    "level": "error",
                    "message": "undefined variable 'user_name'",
                    "spans": [
                        {
                            "start_line": 15,
                            "start_column": 9,
                            "end_line": 15,
                            "end_column": 18,
                            "file_path": "test.asthra",
                            "label": "undefined variable"
                        }
                    ],
                    "suggestions": [
                        {
                            "type": "replace",
                            "text": "user_info",
                            "confidence": "high",
                            "rationale": "Similar variable found in scope"
                        }
                    ]
                }
            ]
        }
        "#;
        
        let api = AsthraDiagnosticAPI::parse_compiler_output(json).unwrap();
        assert_eq!(api.diagnostics.len(), 1);
        assert_eq!(api.get_errors().len(), 1);
        assert_eq!(api.get_high_confidence_suggestions().len(), 1);
    }
    
    #[test]
    fn test_statistics() {
        let json = r#"
        {
            "diagnostics": [
                {
                    "code": "ASTHRA_E001",
                    "level": "error",
                    "message": "test error",
                    "spans": [],
                    "suggestions": [
                        {
                            "type": "replace",
                            "text": "fix",
                            "confidence": "high",
                            "rationale": "test"
                        }
                    ]
                }
            ]
        }
        "#;
        
        let api = AsthraDiagnosticAPI::parse_compiler_output(json).unwrap();
        let stats = api.get_statistics();
        
        assert_eq!(stats.total_diagnostics, 1);
        assert_eq!(stats.error_count, 1);
        assert_eq!(stats.warning_count, 0);
        assert_eq!(stats.high_confidence_suggestions, 1);
        assert!(stats.has_errors);
    }
} 
