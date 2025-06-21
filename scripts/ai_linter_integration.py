#!/usr/bin/env python3
"""
Asthra AI Linter Python Integration Helper

This module provides Python bindings and utilities for integrating the Asthra AI linter
into AI development workflows, CI/CD pipelines, and development tools.

Features:
- Easy access to AI linter JSON output
- Structured data classes for linting results
- Integration with popular development tools
- Batch processing capabilities
- Statistics and reporting
"""

import json
import subprocess
import sys
import os
from dataclasses import dataclass
from typing import List, Dict, Optional, Any, Union
from pathlib import Path
import tempfile

@dataclass
class AILintLocation:
    """Represents a source code location for an AI linting issue."""
    file: str
    start_line: int
    start_column: int
    end_line: int
    end_column: int

@dataclass
class AILintIssue:
    """Represents a single AI linting issue."""
    rule_id: str
    category: str
    severity: str
    title: str
    description: str
    ai_guidance: str
    location: AILintLocation
    confidence: float
    impact_score: float
    auto_fixable: bool

@dataclass
class AILintStatistics:
    """Represents statistics from AI linting analysis."""
    total_issues: int
    auto_fixable_count: int
    high_impact_count: int
    average_confidence: float
    average_impact: float
    by_category: Dict[str, int]
    by_severity: Dict[str, int]

@dataclass
class AILintResult:
    """Complete AI linting result with issues and statistics."""
    schema_version: str
    timestamp: str
    total_issues: int
    issues: List[AILintIssue]
    statistics: AILintStatistics

@dataclass
class AILintRule:
    """Represents an AI linting rule definition."""
    rule_id: str
    category: str
    default_severity: str
    title: str
    description: str
    ai_guidance: str
    impact_score: float
    auto_fixable: bool

@dataclass
class AILintFixSuggestion:
    """Represents an auto-fix suggestion."""
    rule_id: str
    ai_guidance: str
    location: AILintLocation
    confidence: float

class AsthraiLinter:
    """Python interface to the Asthra AI linter."""
    
    def __init__(self, asthra_path: Optional[str] = None):
        """Initialize the AI linter interface.
        
        Args:
            asthra_path: Path to the Asthra compiler executable. If None, searches PATH.
        """
        self.asthra_path = asthra_path or self._find_asthra_executable()
        if not self.asthra_path:
            raise RuntimeError("Asthra compiler not found. Please install or specify path.")
    
    def _find_asthra_executable(self) -> Optional[str]:
        """Find the Asthra compiler executable in PATH."""
        import shutil
        return shutil.which("asthra") or shutil.which("./asthra")
    
    def lint_file(self, file_path: str, output_format: str = "json") -> AILintResult:
        """Lint a single Asthra source file.
        
        Args:
            file_path: Path to the Asthra source file
            output_format: Output format ("json" or "text")
            
        Returns:
            AILintResult with issues and statistics
        """
        if not os.path.exists(file_path):
            raise FileNotFoundError(f"Source file not found: {file_path}")
        
        cmd = [self.asthra_path, "lint", "--ai-output", "--format", output_format, file_path]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=False)
            
            if output_format == "json":
                return self._parse_json_result(result.stdout)
            else:
                return self._parse_text_result(result.stdout)
                
        except subprocess.SubprocessError as e:
            raise RuntimeError(f"Failed to run AI linter: {e}")
    
    def lint_directory(self, directory_path: str, pattern: str = "*.asthra") -> List[AILintResult]:
        """Lint all Asthra files in a directory.
        
        Args:
            directory_path: Path to directory containing Asthra files
            pattern: File pattern to match (default: "*.asthra")
            
        Returns:
            List of AILintResult for each file
        """
        directory = Path(directory_path)
        if not directory.exists():
            raise FileNotFoundError(f"Directory not found: {directory_path}")
        
        results = []
        for file_path in directory.rglob(pattern):
            if file_path.is_file():
                try:
                    result = self.lint_file(str(file_path))
                    results.append(result)
                except Exception as e:
                    print(f"Warning: Failed to lint {file_path}: {e}", file=sys.stderr)
        
        return results
    
    def get_rule_catalog(self) -> List[AILintRule]:
        """Get the catalog of available AI linting rules.
        
        Returns:
            List of AILintRule definitions
        """
        cmd = [self.asthra_path, "lint", "--list-rules", "--format", "json"]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            data = json.loads(result.stdout)
            
            rules = []
            for rule_data in data.get("rules", []):
                rule = AILintRule(
                    rule_id=rule_data["rule_id"],
                    category=rule_data["category"],
                    default_severity=rule_data["default_severity"],
                    title=rule_data["title"],
                    description=rule_data["description"],
                    ai_guidance=rule_data["ai_guidance"],
                    impact_score=rule_data["impact_score"],
                    auto_fixable=rule_data["auto_fixable"]
                )
                rules.append(rule)
            
            return rules
            
        except (subprocess.SubprocessError, json.JSONDecodeError) as e:
            raise RuntimeError(f"Failed to get rule catalog: {e}")
    
    def get_fix_suggestions(self, file_path: str) -> List[AILintFixSuggestion]:
        """Get auto-fix suggestions for a file.
        
        Args:
            file_path: Path to the Asthra source file
            
        Returns:
            List of AILintFixSuggestion
        """
        cmd = [self.asthra_path, "lint", "--fix-suggestions", "--format", "json", file_path]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            data = json.loads(result.stdout)
            
            suggestions = []
            for suggestion_data in data.get("fix_suggestions", []):
                location = AILintLocation(
                    file=suggestion_data["location"]["file"],
                    start_line=suggestion_data["location"]["start_line"],
                    start_column=suggestion_data["location"]["start_column"],
                    end_line=suggestion_data["location"]["end_line"],
                    end_column=suggestion_data["location"]["end_column"]
                )
                
                suggestion = AILintFixSuggestion(
                    rule_id=suggestion_data["rule_id"],
                    ai_guidance=suggestion_data["ai_guidance"],
                    location=location,
                    confidence=suggestion_data["confidence"]
                )
                suggestions.append(suggestion)
            
            return suggestions
            
        except (subprocess.SubprocessError, json.JSONDecodeError) as e:
            raise RuntimeError(f"Failed to get fix suggestions: {e}")
    
    def _parse_json_result(self, json_output: str) -> AILintResult:
        """Parse JSON output from the AI linter."""
        try:
            data = json.loads(json_output)
            
            # Parse issues
            issues = []
            for issue_data in data.get("issues", []):
                location = AILintLocation(
                    file=issue_data["location"]["file"],
                    start_line=issue_data["location"]["start_line"],
                    start_column=issue_data["location"]["start_column"],
                    end_line=issue_data["location"]["end_line"],
                    end_column=issue_data["location"]["end_column"]
                )
                
                issue = AILintIssue(
                    rule_id=issue_data["rule_id"],
                    category=issue_data["category"],
                    severity=issue_data["severity"],
                    title=issue_data["title"],
                    description=issue_data["description"],
                    ai_guidance=issue_data["ai_guidance"],
                    location=location,
                    confidence=issue_data["confidence"],
                    impact_score=issue_data["impact_score"],
                    auto_fixable=issue_data["auto_fixable"]
                )
                issues.append(issue)
            
            # Parse statistics
            stats_data = data.get("statistics", {})
            statistics = AILintStatistics(
                total_issues=stats_data.get("total_issues", 0),
                auto_fixable_count=stats_data.get("auto_fixable_count", 0),
                high_impact_count=stats_data.get("high_impact_count", 0),
                average_confidence=stats_data.get("average_confidence", 0.0),
                average_impact=stats_data.get("average_impact", 0.0),
                by_category=stats_data.get("by_category", {}),
                by_severity=stats_data.get("by_severity", {})
            )
            
            return AILintResult(
                schema_version=data.get("schema_version", "1.0"),
                timestamp=data.get("timestamp", ""),
                total_issues=data.get("total_issues", 0),
                issues=issues,
                statistics=statistics
            )
            
        except json.JSONDecodeError as e:
            raise RuntimeError(f"Failed to parse AI linter JSON output: {e}")
    
    def _parse_text_result(self, text_output: str) -> AILintResult:
        """Parse text output from the AI linter (simplified)."""
        # This is a simplified implementation for text output
        # In practice, you'd want more sophisticated parsing
        lines = text_output.strip().split('\n')
        issue_count = len([line for line in lines if line.strip() and not line.startswith('#')])
        
        # Create a minimal result for text output
        statistics = AILintStatistics(
            total_issues=issue_count,
            auto_fixable_count=0,
            high_impact_count=0,
            average_confidence=0.0,
            average_impact=0.0,
            by_category={},
            by_severity={}
        )
        
        return AILintResult(
            schema_version="1.0",
            timestamp="",
            total_issues=issue_count,
            issues=[],  # Text parsing would need more work
            statistics=statistics
        )

class AILintReporter:
    """Generates reports from AI linting results."""
    
    @staticmethod
    def generate_summary(results: Union[AILintResult, List[AILintResult]]) -> Dict[str, Any]:
        """Generate a summary report from linting results.
        
        Args:
            results: Single result or list of results
            
        Returns:
            Dictionary with summary statistics
        """
        if isinstance(results, AILintResult):
            results = [results]
        
        total_issues = sum(r.total_issues for r in results)
        total_files = len(results)
        
        # Aggregate statistics
        all_issues = []
        for result in results:
            all_issues.extend(result.issues)
        
        if not all_issues:
            return {
                "total_files": total_files,
                "total_issues": 0,
                "average_issues_per_file": 0.0,
                "categories": {},
                "severities": {},
                "auto_fixable_percentage": 0.0,
                "high_impact_percentage": 0.0
            }
        
        # Calculate aggregated statistics
        categories = {}
        severities = {}
        auto_fixable = 0
        high_impact = 0
        
        for issue in all_issues:
            categories[issue.category] = categories.get(issue.category, 0) + 1
            severities[issue.severity] = severities.get(issue.severity, 0) + 1
            if issue.auto_fixable:
                auto_fixable += 1
            if issue.impact_score >= 0.7:
                high_impact += 1
        
        return {
            "total_files": total_files,
            "total_issues": total_issues,
            "average_issues_per_file": total_issues / total_files if total_files > 0 else 0.0,
            "categories": categories,
            "severities": severities,
            "auto_fixable_percentage": (auto_fixable / total_issues * 100) if total_issues > 0 else 0.0,
            "high_impact_percentage": (high_impact / total_issues * 100) if total_issues > 0 else 0.0
        }
    
    @staticmethod
    def generate_markdown_report(results: Union[AILintResult, List[AILintResult]]) -> str:
        """Generate a Markdown report from linting results."""
        summary = AILintReporter.generate_summary(results)
        
        if isinstance(results, AILintResult):
            results = [results]
        
        report = []
        report.append("# AI Linting Report")
        report.append("")
        report.append("## Summary")
        report.append("")
        report.append(f"- **Files analyzed**: {summary['total_files']}")
        report.append(f"- **Total issues**: {summary['total_issues']}")
        report.append(f"- **Average issues per file**: {summary['average_issues_per_file']:.1f}")
        report.append(f"- **Auto-fixable**: {summary['auto_fixable_percentage']:.1f}%")
        report.append(f"- **High impact**: {summary['high_impact_percentage']:.1f}%")
        report.append("")
        
        if summary['categories']:
            report.append("## Issues by Category")
            report.append("")
            for category, count in sorted(summary['categories'].items()):
                report.append(f"- **{category.title()}**: {count}")
            report.append("")
        
        if summary['severities']:
            report.append("## Issues by Severity")
            report.append("")
            for severity, count in sorted(summary['severities'].items()):
                report.append(f"- **{severity.title()}**: {count}")
            report.append("")
        
        # Detailed issues
        all_issues = []
        for result in results:
            all_issues.extend(result.issues)
        
        if all_issues:
            report.append("## Detailed Issues")
            report.append("")
            
            for issue in sorted(all_issues, key=lambda x: (x.location.file, x.location.start_line)):
                report.append(f"### {issue.title}")
                report.append("")
                report.append(f"- **File**: `{issue.location.file}`")
                report.append(f"- **Location**: Line {issue.location.start_line}, Column {issue.location.start_column}")
                report.append(f"- **Category**: {issue.category}")
                report.append(f"- **Severity**: {issue.severity}")
                report.append(f"- **Confidence**: {issue.confidence:.0%}")
                report.append(f"- **Impact**: {issue.impact_score:.0%}")
                report.append(f"- **Auto-fixable**: {'Yes' if issue.auto_fixable else 'No'}")
                report.append("")
                report.append(f"**Description**: {issue.description}")
                report.append("")
                report.append(f"**AI Guidance**: {issue.ai_guidance}")
                report.append("")
        
        return "\n".join(report)

def main():
    """Command-line interface for the AI linter integration."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Asthra AI Linter Python Integration")
    parser.add_argument("path", help="File or directory to lint")
    parser.add_argument("--format", choices=["json", "text", "markdown"], default="json",
                       help="Output format")
    parser.add_argument("--output", "-o", help="Output file (default: stdout)")
    parser.add_argument("--asthra-path", help="Path to Asthra compiler")
    
    args = parser.parse_args()
    
    try:
        linter = AsthraiLinter(args.asthra_path)
        
        if os.path.isfile(args.path):
            result = linter.lint_file(args.path)
            results = [result]
        elif os.path.isdir(args.path):
            results = linter.lint_directory(args.path)
        else:
            print(f"Error: Path not found: {args.path}", file=sys.stderr)
            sys.exit(1)
        
        # Generate output
        if args.format == "json":
            if len(results) == 1:
                output = json.dumps(results[0].__dict__, indent=2, default=str)
            else:
                output = json.dumps([r.__dict__ for r in results], indent=2, default=str)
        elif args.format == "markdown":
            output = AILintReporter.generate_markdown_report(results)
        else:  # text
            summary = AILintReporter.generate_summary(results)
            output = f"AI Linting Summary: {summary['total_issues']} issues in {summary['total_files']} files"
        
        # Write output
        if args.output:
            with open(args.output, 'w') as f:
                f.write(output)
            print(f"Report written to {args.output}")
        else:
            print(output)
    
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main() 
