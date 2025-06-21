"""
Python bindings for Asthra Enhanced Diagnostics
Provides Pythonic interface for AI tools and language servers
"""

import json
import re
from typing import List, Dict, Optional, Any, Union, Tuple
from dataclasses import dataclass, field
from pathlib import Path
from enum import Enum

class DiagnosticLevel(Enum):
    """Diagnostic severity levels"""
    ERROR = "error"
    WARNING = "warning"
    HELP = "help"
    NOTE = "note"

class SuggestionType(Enum):
    """Types of diagnostic suggestions"""
    INSERT = "insert"
    DELETE = "delete"
    REPLACE = "replace"

class ConfidenceLevel(Enum):
    """Confidence levels for suggestions"""
    HIGH = "high"
    MEDIUM = "medium"
    LOW = "low"

@dataclass
class DiagnosticSpan:
    """Represents a source code location span"""
    start_line: int
    start_column: int
    end_line: int
    end_column: int
    file_path: str
    label: Optional[str] = None
    snippet: Optional[str] = None
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DiagnosticSpan':
        """Create DiagnosticSpan from dictionary"""
        return cls(
            start_line=data['start_line'],
            start_column=data['start_column'],
            end_line=data['end_line'],
            end_column=data['end_column'],
            file_path=data['file_path'],
            label=data.get('label'),
            snippet=data.get('snippet')
        )

@dataclass
class DiagnosticSuggestion:
    """Represents a suggested fix for a diagnostic"""
    suggestion_type: SuggestionType
    text: str
    confidence: ConfidenceLevel
    rationale: str
    span: Optional[DiagnosticSpan] = None
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DiagnosticSuggestion':
        """Create DiagnosticSuggestion from dictionary"""
        return cls(
            suggestion_type=SuggestionType(data['type']),
            text=data['text'],
            confidence=ConfidenceLevel(data['confidence']),
            rationale=data['rationale'],
            span=DiagnosticSpan.from_dict(data['span']) if 'span' in data else None
        )

@dataclass
class DiagnosticMetadata:
    """Additional metadata for diagnostics"""
    inferred_types: List[str] = field(default_factory=list)
    available_methods: List[str] = field(default_factory=list)
    similar_symbols: List[str] = field(default_factory=list)
    error_category: Optional[str] = None
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'DiagnosticMetadata':
        """Create DiagnosticMetadata from dictionary"""
        return cls(
            inferred_types=data.get('inferred_types', []),
            available_methods=data.get('available_methods', []),
            similar_symbols=data.get('similar_symbols', []),
            error_category=data.get('error_category')
        )

@dataclass
class EnhancedDiagnostic:
    """Enhanced diagnostic with structured information"""
    code: str
    level: DiagnosticLevel
    message: str
    spans: List[DiagnosticSpan] = field(default_factory=list)
    suggestions: List[DiagnosticSuggestion] = field(default_factory=list)
    metadata: Optional[DiagnosticMetadata] = None
    related_info: List[str] = field(default_factory=list)
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'EnhancedDiagnostic':
        """Create EnhancedDiagnostic from dictionary"""
        spans = [DiagnosticSpan.from_dict(span_data) for span_data in data.get('spans', [])]
        suggestions = [DiagnosticSuggestion.from_dict(sugg_data) for sugg_data in data.get('suggestions', [])]
        metadata = DiagnosticMetadata.from_dict(data['metadata']) if 'metadata' in data else None
        
        return cls(
            code=data['code'],
            level=DiagnosticLevel(data['level']),
            message=data['message'],
            spans=spans,
            suggestions=suggestions,
            metadata=metadata,
            related_info=data.get('related_info', [])
        )
    
    def is_error(self) -> bool:
        """Check if this diagnostic is an error"""
        return self.level == DiagnosticLevel.ERROR
    
    def is_warning(self) -> bool:
        """Check if this diagnostic is a warning"""
        return self.level == DiagnosticLevel.WARNING
    
    def has_high_confidence_suggestions(self) -> bool:
        """Check if this diagnostic has high-confidence suggestions"""
        return any(s.confidence == ConfidenceLevel.HIGH for s in self.suggestions)
    
    def get_primary_span(self) -> Optional[DiagnosticSpan]:
        """Get the primary (first) span of this diagnostic"""
        return self.spans[0] if self.spans else None

@dataclass
class DiagnosticStatistics:
    """Statistics about a set of diagnostics"""
    total_diagnostics: int
    error_count: int
    warning_count: int
    help_count: int
    note_count: int
    total_suggestions: int
    high_confidence_suggestions: int
    medium_confidence_suggestions: int
    low_confidence_suggestions: int
    has_errors: bool
    fix_success_rate: float

class AsthraDiagnosticAPI:
    """
    Python interface to Asthra's enhanced diagnostic system.
    Enables AI tools to understand and fix compilation errors automatically.
    """
    
    def __init__(self, json_output: str):
        """Parse Asthra compiler JSON output."""
        self.diagnostics = self._parse_diagnostics(json_output)
    
    def _parse_diagnostics(self, json_output: str) -> List[EnhancedDiagnostic]:
        """Parse JSON output into diagnostic objects."""
        try:
            data = json.loads(json_output)
            diagnostics = []
            
            for diag_data in data.get("diagnostics", []):
                diagnostic = EnhancedDiagnostic.from_dict(diag_data)
                diagnostics.append(diagnostic)
            
            return diagnostics
        except json.JSONDecodeError as e:
            raise ValueError(f"Invalid JSON output: {e}")
    
    def get_errors(self) -> List[EnhancedDiagnostic]:
        """Get all error diagnostics."""
        return [d for d in self.diagnostics if d.is_error()]
    
    def get_warnings(self) -> List[EnhancedDiagnostic]:
        """Get all warning diagnostics."""
        return [d for d in self.diagnostics if d.is_warning()]
    
    def get_high_confidence_fixes(self) -> List[DiagnosticSuggestion]:
        """Get suggestions with high confidence scores."""
        fixes = []
        for diagnostic in self.diagnostics:
            fixes.extend([s for s in diagnostic.suggestions if s.confidence == ConfidenceLevel.HIGH])
        return fixes
    
    def get_actionable_suggestions(self) -> List[DiagnosticSuggestion]:
        """Get suggestions with medium or high confidence."""
        suggestions = []
        for diagnostic in self.diagnostics:
            suggestions.extend([
                s for s in diagnostic.suggestions 
                if s.confidence in [ConfidenceLevel.HIGH, ConfidenceLevel.MEDIUM]
            ])
        return suggestions
    
    def apply_fixes(self, source_code: str, confidence_threshold: ConfidenceLevel = ConfidenceLevel.HIGH) -> str:
        """Apply fixes to source code based on confidence threshold."""
        modified_code = source_code
        applicable_fixes = []
        
        for diagnostic in self.diagnostics:
            for suggestion in diagnostic.suggestions:
                if self._meets_confidence_threshold(suggestion.confidence, confidence_threshold):
                    applicable_fixes.append(suggestion)
        
        # Apply fixes in reverse order to maintain line numbers
        for fix in reversed(applicable_fixes):
            modified_code = self._apply_single_fix(modified_code, fix)
        
        return modified_code
    
    def _meets_confidence_threshold(self, confidence: ConfidenceLevel, threshold: ConfidenceLevel) -> bool:
        """Check if confidence meets threshold."""
        confidence_order = {ConfidenceLevel.LOW: 1, ConfidenceLevel.MEDIUM: 2, ConfidenceLevel.HIGH: 3}
        return confidence_order[confidence] >= confidence_order[threshold]
    
    def _apply_single_fix(self, source_code: str, suggestion: DiagnosticSuggestion) -> str:
        """Apply a single suggestion to source code."""
        if suggestion.suggestion_type == SuggestionType.REPLACE:
            # Simple replacement logic (would need more sophisticated implementation)
            return source_code.replace("undefined_variable", suggestion.text)
        elif suggestion.suggestion_type == SuggestionType.INSERT:
            return source_code + suggestion.text
        elif suggestion.suggestion_type == SuggestionType.DELETE:
            return source_code.replace(suggestion.text, "")
        else:
            return source_code
    
    def get_statistics(self) -> DiagnosticStatistics:
        """Get comprehensive statistics about the diagnostics."""
        error_count = len(self.get_errors())
        warning_count = len(self.get_warnings())
        help_count = len([d for d in self.diagnostics if d.level == DiagnosticLevel.HELP])
        note_count = len([d for d in self.diagnostics if d.level == DiagnosticLevel.NOTE])
        
        all_suggestions = []
        for diagnostic in self.diagnostics:
            all_suggestions.extend(diagnostic.suggestions)
        
        high_confidence = len([s for s in all_suggestions if s.confidence == ConfidenceLevel.HIGH])
        medium_confidence = len([s for s in all_suggestions if s.confidence == ConfidenceLevel.MEDIUM])
        low_confidence = len([s for s in all_suggestions if s.confidence == ConfidenceLevel.LOW])
        
        fix_success_rate = self._calculate_fix_success_rate()
        
        return DiagnosticStatistics(
            total_diagnostics=len(self.diagnostics),
            error_count=error_count,
            warning_count=warning_count,
            help_count=help_count,
            note_count=note_count,
            total_suggestions=len(all_suggestions),
            high_confidence_suggestions=high_confidence,
            medium_confidence_suggestions=medium_confidence,
            low_confidence_suggestions=low_confidence,
            has_errors=error_count > 0,
            fix_success_rate=fix_success_rate
        )
    
    def _calculate_fix_success_rate(self) -> float:
        """Calculate estimated fix success rate based on confidence levels."""
        all_suggestions = []
        for diagnostic in self.diagnostics:
            all_suggestions.extend(diagnostic.suggestions)
        
        if not all_suggestions:
            return 0.0
        
        confidence_weights = {
            ConfidenceLevel.HIGH: 0.9,
            ConfidenceLevel.MEDIUM: 0.7,
            ConfidenceLevel.LOW: 0.4
        }
        
        weighted_score = sum(confidence_weights[s.confidence] for s in all_suggestions)
        return weighted_score / len(all_suggestions)
    
    def get_ai_learning_data(self) -> Dict[str, Any]:
        """Extract data useful for AI model training."""
        error_patterns = [d.code for d in self.get_errors()]
        successful_suggestions = self.get_high_confidence_fixes()
        common_mistakes = self._analyze_common_patterns()
        fix_success_rate = self._calculate_fix_success_rate()
        metadata_insights = self._extract_metadata_insights()
        
        return {
            "error_patterns": error_patterns,
            "successful_suggestions": [
                {
                    "type": s.suggestion_type.value,
                    "text": s.text,
                    "confidence": s.confidence.value,
                    "rationale": s.rationale
                } for s in successful_suggestions
            ],
            "common_mistakes": common_mistakes,
            "fix_success_rate": fix_success_rate,
            "metadata_insights": metadata_insights,
            "statistics": self.get_statistics().__dict__
        }
    
    def _analyze_common_patterns(self) -> Dict[str, int]:
        """Analyze common error patterns."""
        patterns = {}
        for diagnostic in self.diagnostics:
            patterns[diagnostic.code] = patterns.get(diagnostic.code, 0) + 1
        return patterns
    
    def _extract_metadata_insights(self) -> Dict[str, Any]:
        """Extract insights from diagnostic metadata."""
        type_frequency = {}
        symbol_frequency = {}
        category_frequency = {}
        
        for diagnostic in self.diagnostics:
            if diagnostic.metadata:
                # Count type frequencies
                for type_name in diagnostic.metadata.inferred_types:
                    type_frequency[type_name] = type_frequency.get(type_name, 0) + 1
                
                # Count symbol frequencies
                for symbol in diagnostic.metadata.similar_symbols:
                    symbol_frequency[symbol] = symbol_frequency.get(symbol, 0) + 1
                
                # Count category frequencies
                if diagnostic.metadata.error_category:
                    category = diagnostic.metadata.error_category
                    category_frequency[category] = category_frequency.get(category, 0) + 1
        
        return {
            "common_types": type_frequency,
            "common_symbols": symbol_frequency,
            "error_categories": category_frequency
        }
    
    def filter_by_code(self, code: str) -> List[EnhancedDiagnostic]:
        """Filter diagnostics by error code."""
        return [d for d in self.diagnostics if d.code == code]
    
    def filter_by_file(self, file_path: str) -> List[EnhancedDiagnostic]:
        """Filter diagnostics by file path."""
        return [
            d for d in self.diagnostics 
            if any(span.file_path == file_path for span in d.spans)
        ]
    
    def is_compilation_successful(self) -> bool:
        """Check if compilation was successful (no errors)."""
        return len(self.get_errors()) == 0
    
    def get_all_diagnostics(self) -> List[EnhancedDiagnostic]:
        """Get all diagnostics."""
        return self.diagnostics.copy()
    
    def export_for_language_server(self) -> List[Dict[str, Any]]:
        """Export diagnostics in Language Server Protocol format."""
        lsp_diagnostics = []
        
        for diagnostic in self.diagnostics:
            for span in diagnostic.spans:
                lsp_diagnostic = {
                    "range": {
                        "start": {
                            "line": span.start_line - 1,  # LSP uses 0-based lines
                            "character": span.start_column - 1
                        },
                        "end": {
                            "line": span.end_line - 1,
                            "character": span.end_column - 1
                        }
                    },
                    "severity": self._diagnostic_level_to_lsp_severity(diagnostic.level),
                    "code": diagnostic.code,
                    "message": diagnostic.message,
                    "source": "asthra"
                }
                
                if diagnostic.suggestions:
                    lsp_diagnostic["codeActions"] = [
                        {
                            "title": f"Apply: {s.text}",
                            "kind": "quickfix",
                            "edit": {
                                "changes": {
                                    span.file_path: [
                                        {
                                            "range": lsp_diagnostic["range"],
                                            "newText": s.text
                                        }
                                    ]
                                }
                            }
                        } for s in diagnostic.suggestions
                        if s.confidence in [ConfidenceLevel.HIGH, ConfidenceLevel.MEDIUM]
                    ]
                
                lsp_diagnostics.append(lsp_diagnostic)
        
        return lsp_diagnostics
    
    def _diagnostic_level_to_lsp_severity(self, level: DiagnosticLevel) -> int:
        """Convert diagnostic level to LSP severity."""
        mapping = {
            DiagnosticLevel.ERROR: 1,    # Error
            DiagnosticLevel.WARNING: 2,  # Warning
            DiagnosticLevel.HELP: 3,     # Information
            DiagnosticLevel.NOTE: 4      # Hint
        }
        return mapping.get(level, 3)

# Convenience functions for common use cases
def parse_asthra_output(json_output: str) -> AsthraDiagnosticAPI:
    """Parse Asthra compiler output and return API instance."""
    return AsthraDiagnosticAPI(json_output)

def auto_fix_code(source_code: str, compiler_output: str, 
                  confidence_threshold: ConfidenceLevel = ConfidenceLevel.HIGH) -> Tuple[str, bool]:
    """Automatically apply fixes to source code."""
    api = AsthraDiagnosticAPI(compiler_output)
    fixed_code = api.apply_fixes(source_code, confidence_threshold)
    success = api.is_compilation_successful()
    return fixed_code, success

def extract_learning_data(compiler_outputs: List[str]) -> Dict[str, Any]:
    """Extract learning data from multiple compiler outputs."""
    combined_data = {
        "error_patterns": [],
        "successful_suggestions": [],
        "common_mistakes": {},
        "fix_success_rates": [],
        "metadata_insights": {"common_types": {}, "common_symbols": {}, "error_categories": {}}
    }
    
    for output in compiler_outputs:
        api = AsthraDiagnosticAPI(output)
        learning_data = api.get_ai_learning_data()
        
        combined_data["error_patterns"].extend(learning_data["error_patterns"])
        combined_data["successful_suggestions"].extend(learning_data["successful_suggestions"])
        combined_data["fix_success_rates"].append(learning_data["fix_success_rate"])
        
        # Merge common mistakes
        for mistake, count in learning_data["common_mistakes"].items():
            combined_data["common_mistakes"][mistake] = combined_data["common_mistakes"].get(mistake, 0) + count
        
        # Merge metadata insights
        for category, data in learning_data["metadata_insights"].items():
            if isinstance(data, dict):
                for key, value in data.items():
                    combined_data["metadata_insights"][category][key] = \
                        combined_data["metadata_insights"][category].get(key, 0) + value
    
    # Calculate average fix success rate
    if combined_data["fix_success_rates"]:
        combined_data["average_fix_success_rate"] = sum(combined_data["fix_success_rates"]) / len(combined_data["fix_success_rates"])
    else:
        combined_data["average_fix_success_rate"] = 0.0
    
    return combined_data 
