#!/usr/bin/env python3
"""
Test suite for Week 3: Language Bindings & Performance Optimization
Tests Python bindings for Asthra Enhanced Diagnostics
"""

import unittest
import json
import sys
import os
from typing import Dict, Any

# Add the bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../src/diagnostics/bindings'))

from python_bindings import (
    AsthraDiagnosticAPI, DiagnosticLevel, SuggestionType, ConfidenceLevel,
    DiagnosticSpan, DiagnosticSuggestion, DiagnosticMetadata, EnhancedDiagnostic,
    parse_asthra_output, auto_fix_code, extract_learning_data
)

class TestPythonBindings(unittest.TestCase):
    """Test suite for Python bindings"""
    
    def setUp(self):
        """Set up test data"""
        self.sample_json = """
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
                            "label": "undefined variable",
                            "snippet": "let email = user_name.email;"
                        }
                    ],
                    "suggestions": [
                        {
                            "type": "replace",
                            "text": "user_info",
                            "confidence": "high",
                            "rationale": "Similar variable 'user_info' found in scope"
                        }
                    ],
                    "metadata": {
                        "inferred_types": ["string"],
                        "similar_symbols": ["user_info", "user_data"],
                        "error_category": "semantic"
                    }
                },
                {
                    "code": "ASTHRA_W001",
                    "level": "warning",
                    "message": "unused variable 'temp'",
                    "spans": [
                        {
                            "start_line": 10,
                            "start_column": 5,
                            "end_line": 10,
                            "end_column": 9,
                            "file_path": "test.asthra",
                            "label": "unused variable"
                        }
                    ],
                    "suggestions": [
                        {
                            "type": "delete",
                            "text": "let temp = 42;",
                            "confidence": "medium",
                            "rationale": "Variable is never used"
                        }
                    ]
                }
            ]
        }
        """
        
        self.api = AsthraDiagnosticAPI(self.sample_json)
    
    def test_diagnostic_parsing(self):
        """Test parsing of diagnostic JSON"""
        self.assertEqual(len(self.api.diagnostics), 2)
        
        # Test first diagnostic
        error_diag = self.api.diagnostics[0]
        self.assertEqual(error_diag.code, "ASTHRA_E001")
        self.assertEqual(error_diag.level, DiagnosticLevel.ERROR)
        self.assertEqual(error_diag.message, "undefined variable 'user_name'")
        self.assertEqual(len(error_diag.spans), 1)
        self.assertEqual(len(error_diag.suggestions), 1)
        self.assertIsNotNone(error_diag.metadata)
        
        # Test span
        span = error_diag.spans[0]
        self.assertEqual(span.start_line, 15)
        self.assertEqual(span.start_column, 9)
        self.assertEqual(span.file_path, "test.asthra")
        
        # Test suggestion
        suggestion = error_diag.suggestions[0]
        self.assertEqual(suggestion.suggestion_type, SuggestionType.REPLACE)
        self.assertEqual(suggestion.text, "user_info")
        self.assertEqual(suggestion.confidence, ConfidenceLevel.HIGH)
    
    def test_error_filtering(self):
        """Test filtering of errors and warnings"""
        errors = self.api.get_errors()
        warnings = self.api.get_warnings()
        
        self.assertEqual(len(errors), 1)
        self.assertEqual(len(warnings), 1)
        self.assertEqual(errors[0].code, "ASTHRA_E001")
        self.assertEqual(warnings[0].code, "ASTHRA_W001")
    
    def test_suggestion_filtering(self):
        """Test filtering of suggestions by confidence"""
        high_confidence = self.api.get_high_confidence_fixes()
        actionable = self.api.get_actionable_suggestions()
        
        self.assertEqual(len(high_confidence), 1)
        self.assertEqual(len(actionable), 2)  # high + medium
        self.assertEqual(high_confidence[0].text, "user_info")
    
    def test_statistics(self):
        """Test diagnostic statistics"""
        stats = self.api.get_statistics()
        
        self.assertEqual(stats.total_diagnostics, 2)
        self.assertEqual(stats.error_count, 1)
        self.assertEqual(stats.warning_count, 1)
        self.assertEqual(stats.high_confidence_suggestions, 1)
        self.assertEqual(stats.medium_confidence_suggestions, 1)
        self.assertTrue(stats.has_errors)
        self.assertGreater(stats.fix_success_rate, 0.0)
    
    def test_ai_learning_data(self):
        """Test AI learning data extraction"""
        learning_data = self.api.get_ai_learning_data()
        
        self.assertIn("error_patterns", learning_data)
        self.assertIn("successful_suggestions", learning_data)
        self.assertIn("common_mistakes", learning_data)
        self.assertIn("fix_success_rate", learning_data)
        self.assertIn("metadata_insights", learning_data)
        
        self.assertEqual(learning_data["error_patterns"], ["ASTHRA_E001"])
        self.assertEqual(len(learning_data["successful_suggestions"]), 1)
        self.assertGreater(learning_data["fix_success_rate"], 0.0)
    
    def test_code_filtering(self):
        """Test filtering by error code"""
        e001_diagnostics = self.api.filter_by_code("ASTHRA_E001")
        w001_diagnostics = self.api.filter_by_code("ASTHRA_W001")
        
        self.assertEqual(len(e001_diagnostics), 1)
        self.assertEqual(len(w001_diagnostics), 1)
        self.assertEqual(e001_diagnostics[0].code, "ASTHRA_E001")
    
    def test_file_filtering(self):
        """Test filtering by file path"""
        test_file_diagnostics = self.api.filter_by_file("test.asthra")
        other_file_diagnostics = self.api.filter_by_file("other.asthra")
        
        self.assertEqual(len(test_file_diagnostics), 2)
        self.assertEqual(len(other_file_diagnostics), 0)
    
    def test_compilation_success(self):
        """Test compilation success detection"""
        self.assertFalse(self.api.is_compilation_successful())
        
        # Test with no errors
        warning_only_json = """
        {
            "diagnostics": [
                {
                    "code": "ASTHRA_W001",
                    "level": "warning",
                    "message": "unused variable",
                    "spans": [],
                    "suggestions": []
                }
            ]
        }
        """
        warning_api = AsthraDiagnosticAPI(warning_only_json)
        self.assertTrue(warning_api.is_compilation_successful())
    
    def test_lsp_export(self):
        """Test Language Server Protocol export"""
        lsp_diagnostics = self.api.export_for_language_server()
        
        self.assertEqual(len(lsp_diagnostics), 2)
        
        # Test first diagnostic
        lsp_diag = lsp_diagnostics[0]
        self.assertIn("range", lsp_diag)
        self.assertIn("severity", lsp_diag)
        self.assertIn("code", lsp_diag)
        self.assertIn("message", lsp_diag)
        self.assertEqual(lsp_diag["source"], "asthra")
        
        # Test range (LSP uses 0-based indexing)
        self.assertEqual(lsp_diag["range"]["start"]["line"], 14)  # 15 - 1
        self.assertEqual(lsp_diag["range"]["start"]["character"], 8)  # 9 - 1
    
    def test_apply_fixes(self):
        """Test applying fixes to source code"""
        source_code = "let email = undefined_variable.email;"
        fixed_code = self.api.apply_fixes(source_code, ConfidenceLevel.HIGH)
        
        # Should replace undefined_variable with user_info
        self.assertIn("user_info", fixed_code)
        self.assertNotIn("undefined_variable", fixed_code)
    
    def test_convenience_functions(self):
        """Test convenience functions"""
        # Test parse_asthra_output
        api = parse_asthra_output(self.sample_json)
        self.assertEqual(len(api.diagnostics), 2)
        
        # Test auto_fix_code
        source_code = "let email = undefined_variable.email;"
        fixed_code, success = auto_fix_code(source_code, self.sample_json)
        self.assertIn("user_info", fixed_code)
        self.assertFalse(success)  # Still has errors after fix
    
    def test_extract_learning_data(self):
        """Test extracting learning data from multiple outputs"""
        outputs = [self.sample_json, self.sample_json]  # Duplicate for testing
        combined_data = extract_learning_data(outputs)
        
        self.assertIn("error_patterns", combined_data)
        self.assertIn("average_fix_success_rate", combined_data)
        self.assertEqual(len(combined_data["error_patterns"]), 2)  # Duplicated
        self.assertGreater(combined_data["average_fix_success_rate"], 0.0)

class TestPerformance(unittest.TestCase):
    """Performance tests for the diagnostic system"""
    
    def test_large_diagnostic_set_performance(self):
        """Test performance with large diagnostic sets"""
        import time
        
        # Generate large diagnostic set
        large_diagnostics = {
            "diagnostics": []
        }
        
        for i in range(1000):
            diagnostic = {
                "code": f"ASTHRA_E{i:03d}",
                "level": "error",
                "message": f"Test error {i}",
                "spans": [
                    {
                        "start_line": i + 1,
                        "start_column": 1,
                        "end_line": i + 1,
                        "end_column": 10,
                        "file_path": f"test{i}.asthra",
                        "label": "test error"
                    }
                ],
                "suggestions": [
                    {
                        "type": "replace",
                        "text": f"fix_{i}",
                        "confidence": "high",
                        "rationale": f"Fix for error {i}"
                    }
                ]
            }
            large_diagnostics["diagnostics"].append(diagnostic)
        
        json_str = json.dumps(large_diagnostics)
        
        # Test parsing performance
        start_time = time.time()
        api = AsthraDiagnosticAPI(json_str)
        parse_time = time.time() - start_time
        
        self.assertLess(parse_time, 1.0)  # Should parse in under 1 second
        self.assertEqual(len(api.diagnostics), 1000)
        
        # Test filtering performance
        start_time = time.time()
        errors = api.get_errors()
        filter_time = time.time() - start_time
        
        self.assertLess(filter_time, 0.1)  # Should filter in under 100ms
        self.assertEqual(len(errors), 1000)
        
        # Test statistics performance
        start_time = time.time()
        stats = api.get_statistics()
        stats_time = time.time() - start_time
        
        self.assertLess(stats_time, 0.5)  # Should calculate stats in under 500ms
        self.assertEqual(stats.total_diagnostics, 1000)

class TestErrorHandling(unittest.TestCase):
    """Test error handling and edge cases"""
    
    def test_invalid_json(self):
        """Test handling of invalid JSON"""
        with self.assertRaises(ValueError):
            AsthraDiagnosticAPI("invalid json")
    
    def test_empty_diagnostics(self):
        """Test handling of empty diagnostic sets"""
        empty_json = '{"diagnostics": []}'
        api = AsthraDiagnosticAPI(empty_json)
        
        self.assertEqual(len(api.diagnostics), 0)
        self.assertEqual(len(api.get_errors()), 0)
        self.assertTrue(api.is_compilation_successful())
        
        stats = api.get_statistics()
        self.assertEqual(stats.total_diagnostics, 0)
        self.assertFalse(stats.has_errors)
    
    def test_malformed_diagnostic(self):
        """Test handling of malformed diagnostic data"""
        malformed_json = """
        {
            "diagnostics": [
                {
                    "code": "ASTHRA_E001",
                    "level": "invalid_level",
                    "message": "test"
                }
            ]
        }
        """
        
        with self.assertRaises(ValueError):
            AsthraDiagnosticAPI(malformed_json)
    
    def test_missing_fields(self):
        """Test handling of missing required fields"""
        minimal_json = """
        {
            "diagnostics": [
                {
                    "code": "ASTHRA_E001",
                    "level": "error",
                    "message": "test error"
                }
            ]
        }
        """
        
        api = AsthraDiagnosticAPI(minimal_json)
        self.assertEqual(len(api.diagnostics), 1)
        
        diagnostic = api.diagnostics[0]
        self.assertEqual(len(diagnostic.spans), 0)
        self.assertEqual(len(diagnostic.suggestions), 0)
        self.assertIsNone(diagnostic.metadata)

def run_performance_benchmarks():
    """Run performance benchmarks"""
    print("Running performance benchmarks...")
    
    # Benchmark 1: Large diagnostic set parsing
    import time
    
    large_set = {"diagnostics": []}
    for i in range(10000):
        large_set["diagnostics"].append({
            "code": f"ASTHRA_E{i:04d}",
            "level": "error",
            "message": f"Error {i}",
            "spans": [],
            "suggestions": []
        })
    
    json_str = json.dumps(large_set)
    
    start_time = time.time()
    api = AsthraDiagnosticAPI(json_str)
    parse_time = time.time() - start_time
    
    print(f"Parsed 10,000 diagnostics in {parse_time:.3f} seconds")
    
    # Benchmark 2: Statistics calculation
    start_time = time.time()
    stats = api.get_statistics()
    stats_time = time.time() - start_time
    
    print(f"Calculated statistics in {stats_time:.3f} seconds")
    
    # Benchmark 3: AI learning data extraction
    start_time = time.time()
    learning_data = api.get_ai_learning_data()
    learning_time = time.time() - start_time
    
    print(f"Extracted AI learning data in {learning_time:.3f} seconds")
    
    print("Performance benchmarks completed!")

if __name__ == "__main__":
    # Run unit tests
    unittest.main(argv=[''], exit=False, verbosity=2)
    
    # Run performance benchmarks
    print("\n" + "="*50)
    run_performance_benchmarks() 
