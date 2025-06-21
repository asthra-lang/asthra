#!/usr/bin/env python3
"""
Grammar Conformance Metrics Collector

Collects and analyzes grammar conformance metrics for monitoring and reporting.
Provides insights into compliance trends, fix effectiveness, and system performance.
"""

import json
import time
import os
import sys
import subprocess
from datetime import datetime, timedelta
from pathlib import Path
from dataclasses import dataclass, asdict
from typing import Dict, List, Optional, Tuple
import argparse

@dataclass
class ComplianceMetrics:
    """Grammar compliance metrics for a specific time period."""
    timestamp: str
    total_files_scanned: int
    total_violations: int
    violations_by_type: Dict[str, int]
    violations_by_priority: Dict[str, int]
    violations_by_directory: Dict[str, int]
    compliance_percentage: float
    files_with_violations: int
    clean_files: int

@dataclass
class PerformanceMetrics:
    """Performance metrics for grammar conformance tools."""
    timestamp: str
    validation_time_seconds: float
    fix_time_seconds: float
    files_processed: int
    fixes_applied: int
    fix_success_rate: float
    average_time_per_file: float

@dataclass
class TrendMetrics:
    """Trend analysis metrics over time."""
    period_start: str
    period_end: str
    violation_reduction: int
    compliance_improvement: float
    fix_effectiveness: float
    new_violations_introduced: int
    files_improved: int
    files_regressed: int

class GrammarMetricsCollector:
    """Main metrics collection and analysis class."""
    
    def __init__(self, output_dir: str = "metrics"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        self.timestamp = datetime.now().isoformat()
        
    def collect_compliance_metrics(self, scan_paths: List[str]) -> ComplianceMetrics:
        """Collect current grammar compliance metrics."""
        print("Collecting compliance metrics...")
        
        # Run comprehensive scan
        violations_by_type = {}
        violations_by_priority = {}
        violations_by_directory = {}
        total_violations = 0
        total_files = 0
        files_with_violations = 0
        
        for scan_path in scan_paths:
            if not os.path.exists(scan_path):
                print(f"Warning: Path {scan_path} does not exist")
                continue
                
            # Scan directory for violations
            result = self._run_validation_scan(scan_path)
            
            total_files += result['total_files']
            total_violations += result['total_violations']
            files_with_violations += result['files_with_violations']
            
            # Aggregate by type
            for vtype, count in result['by_type'].items():
                violations_by_type[vtype] = violations_by_type.get(vtype, 0) + count
            
            # Aggregate by priority
            for priority, count in result['by_priority'].items():
                violations_by_priority[priority] = violations_by_priority.get(priority, 0) + count
            
            # Store by directory
            violations_by_directory[scan_path] = result['total_violations']
        
        compliance_percentage = ((total_files - files_with_violations) / total_files * 100) if total_files > 0 else 100.0
        clean_files = total_files - files_with_violations
        
        return ComplianceMetrics(
            timestamp=self.timestamp,
            total_files_scanned=total_files,
            total_violations=total_violations,
            violations_by_type=violations_by_type,
            violations_by_priority=violations_by_priority,
            violations_by_directory=violations_by_directory,
            compliance_percentage=compliance_percentage,
            files_with_violations=files_with_violations,
            clean_files=clean_files
        )
    
    def collect_performance_metrics(self, test_files: List[str]) -> PerformanceMetrics:
        """Collect performance metrics for grammar tools."""
        print("Collecting performance metrics...")
        
        total_validation_time = 0.0
        total_fix_time = 0.0
        total_fixes_applied = 0
        successful_fixes = 0
        
        for test_file in test_files:
            if not os.path.exists(test_file):
                continue
                
            # Measure validation time
            start_time = time.time()
            validation_result = self._run_validation(test_file)
            validation_time = time.time() - start_time
            total_validation_time += validation_time
            
            # If violations found, measure fix time
            if validation_result['violations'] > 0:
                start_time = time.time()
                fix_result = self._run_fixes(test_file)
                fix_time = time.time() - start_time
                total_fix_time += fix_time
                
                total_fixes_applied += fix_result['fixes_applied']
                if fix_result['success']:
                    successful_fixes += 1
        
        files_processed = len(test_files)
        fix_success_rate = (successful_fixes / files_processed * 100) if files_processed > 0 else 100.0
        average_time_per_file = (total_validation_time + total_fix_time) / files_processed if files_processed > 0 else 0.0
        
        return PerformanceMetrics(
            timestamp=self.timestamp,
            validation_time_seconds=total_validation_time,
            fix_time_seconds=total_fix_time,
            files_processed=files_processed,
            fixes_applied=total_fixes_applied,
            fix_success_rate=fix_success_rate,
            average_time_per_file=average_time_per_file
        )
    
    def analyze_trends(self, days_back: int = 30) -> Optional[TrendMetrics]:
        """Analyze trends over the specified time period."""
        print(f"Analyzing trends over the last {days_back} days...")
        
        # Load historical metrics
        historical_metrics = self._load_historical_metrics(days_back)
        
        if len(historical_metrics) < 2:
            print("Insufficient historical data for trend analysis")
            return None
        
        # Compare first and last metrics
        oldest = historical_metrics[0]
        newest = historical_metrics[-1]
        
        violation_reduction = oldest.total_violations - newest.total_violations
        compliance_improvement = newest.compliance_percentage - oldest.compliance_percentage
        
        # Calculate fix effectiveness (violations fixed vs new violations)
        total_fixes = sum(m.total_violations for m in historical_metrics[:-1]) - newest.total_violations
        fix_effectiveness = (total_fixes / oldest.total_violations * 100) if oldest.total_violations > 0 else 100.0
        
        # Estimate new violations introduced
        new_violations_introduced = max(0, newest.total_violations - (oldest.total_violations - total_fixes))
        
        # File improvement analysis
        files_improved = max(0, newest.clean_files - oldest.clean_files)
        files_regressed = max(0, oldest.clean_files - newest.clean_files)
        
        return TrendMetrics(
            period_start=oldest.timestamp,
            period_end=newest.timestamp,
            violation_reduction=violation_reduction,
            compliance_improvement=compliance_improvement,
            fix_effectiveness=fix_effectiveness,
            new_violations_introduced=new_violations_introduced,
            files_improved=files_improved,
            files_regressed=files_regressed
        )
    
    def generate_report(self, compliance: ComplianceMetrics, performance: PerformanceMetrics, 
                       trends: Optional[TrendMetrics] = None) -> str:
        """Generate a comprehensive metrics report."""
        report = []
        report.append("# Grammar Conformance Metrics Report")
        report.append(f"**Generated**: {self.timestamp}")
        report.append("")
        
        # Compliance Summary
        report.append("## Compliance Summary")
        report.append(f"- **Total Files Scanned**: {compliance.total_files_scanned:,}")
        report.append(f"- **Total Violations**: {compliance.total_violations:,}")
        report.append(f"- **Compliance Percentage**: {compliance.compliance_percentage:.2f}%")
        report.append(f"- **Clean Files**: {compliance.clean_files:,}")
        report.append(f"- **Files with Violations**: {compliance.files_with_violations:,}")
        report.append("")
        
        # Violations by Type
        if compliance.violations_by_type:
            report.append("## Violations by Type")
            for vtype, count in sorted(compliance.violations_by_type.items(), key=lambda x: x[1], reverse=True):
                percentage = (count / compliance.total_violations * 100) if compliance.total_violations > 0 else 0
                report.append(f"- **{vtype}**: {count:,} ({percentage:.1f}%)")
            report.append("")
        
        # Violations by Priority
        if compliance.violations_by_priority:
            report.append("## Violations by Priority")
            priority_order = ['CRITICAL', 'HIGH', 'MEDIUM', 'LOW']
            for priority in priority_order:
                if priority in compliance.violations_by_priority:
                    count = compliance.violations_by_priority[priority]
                    percentage = (count / compliance.total_violations * 100) if compliance.total_violations > 0 else 0
                    report.append(f"- **{priority}**: {count:,} ({percentage:.1f}%)")
            report.append("")
        
        # Performance Metrics
        report.append("## Performance Metrics")
        report.append(f"- **Files Processed**: {performance.files_processed:,}")
        report.append(f"- **Total Validation Time**: {performance.validation_time_seconds:.2f}s")
        report.append(f"- **Total Fix Time**: {performance.fix_time_seconds:.2f}s")
        report.append(f"- **Fixes Applied**: {performance.fixes_applied:,}")
        report.append(f"- **Fix Success Rate**: {performance.fix_success_rate:.1f}%")
        report.append(f"- **Average Time per File**: {performance.average_time_per_file:.3f}s")
        report.append("")
        
        # Trend Analysis
        if trends:
            report.append("## Trend Analysis")
            report.append(f"- **Period**: {trends.period_start} to {trends.period_end}")
            report.append(f"- **Violation Reduction**: {trends.violation_reduction:,}")
            report.append(f"- **Compliance Improvement**: {trends.compliance_improvement:+.2f}%")
            report.append(f"- **Fix Effectiveness**: {trends.fix_effectiveness:.1f}%")
            report.append(f"- **New Violations Introduced**: {trends.new_violations_introduced:,}")
            report.append(f"- **Files Improved**: {trends.files_improved:,}")
            report.append(f"- **Files Regressed**: {trends.files_regressed:,}")
            report.append("")
        
        # Recommendations
        report.append("## Recommendations")
        if compliance.compliance_percentage >= 95:
            report.append("✅ **Excellent compliance** - maintain current standards")
        elif compliance.compliance_percentage >= 80:
            report.append("⚠️ **Good compliance** - focus on remaining violations")
        else:
            report.append("❌ **Poor compliance** - immediate action required")
        
        if performance.fix_success_rate >= 95:
            report.append("✅ **Excellent fix reliability** - automation working well")
        elif performance.fix_success_rate >= 80:
            report.append("⚠️ **Good fix reliability** - monitor for improvements")
        else:
            report.append("❌ **Poor fix reliability** - review automation tools")
        
        return "\n".join(report)
    
    def save_metrics(self, compliance: ComplianceMetrics, performance: PerformanceMetrics, 
                    trends: Optional[TrendMetrics] = None):
        """Save metrics to JSON files for historical tracking."""
        # Save individual metrics
        compliance_file = self.output_dir / f"compliance_{self.timestamp.replace(':', '-')}.json"
        with open(compliance_file, 'w') as f:
            json.dump(asdict(compliance), f, indent=2)
        
        performance_file = self.output_dir / f"performance_{self.timestamp.replace(':', '-')}.json"
        with open(performance_file, 'w') as f:
            json.dump(asdict(performance), f, indent=2)
        
        if trends:
            trends_file = self.output_dir / f"trends_{self.timestamp.replace(':', '-')}.json"
            with open(trends_file, 'w') as f:
                json.dump(asdict(trends), f, indent=2)
        
        # Update latest metrics
        latest_file = self.output_dir / "latest_metrics.json"
        latest_data = {
            'compliance': asdict(compliance),
            'performance': asdict(performance),
            'trends': asdict(trends) if trends else None,
            'generated_at': self.timestamp
        }
        with open(latest_file, 'w') as f:
            json.dump(latest_data, f, indent=2)
        
        print(f"Metrics saved to {self.output_dir}")
    
    def _run_validation_scan(self, path: str) -> Dict:
        """Run validation scan on a path and return results."""
        try:
            # Use the grammar conformance checker
            result = subprocess.run([
                'python3', 'scripts/grammar_conformance_checker.py', 
                '--scan', path, '--json'
            ], capture_output=True, text=True, timeout=300)
            
            if result.returncode == 0:
                return json.loads(result.stdout)
            else:
                print(f"Warning: Validation scan failed for {path}")
                return {'total_files': 0, 'total_violations': 0, 'files_with_violations': 0, 
                       'by_type': {}, 'by_priority': {}}
        except Exception as e:
            print(f"Error running validation scan: {e}")
            return {'total_files': 0, 'total_violations': 0, 'files_with_violations': 0, 
                   'by_type': {}, 'by_priority': {}}
    
    def _run_validation(self, file_path: str) -> Dict:
        """Run validation on a single file."""
        try:
            result = subprocess.run([
                'python3', 'scripts/validate_single_file.py', file_path
            ], capture_output=True, text=True, timeout=60)
            
            # Parse output for violation count
            violations = 0
            if "Total violations:" in result.stdout:
                for line in result.stdout.split('\n'):
                    if "Total violations:" in line:
                        violations = int(line.split(':')[1].strip())
                        break
            
            return {'violations': violations, 'success': result.returncode == 0}
        except Exception as e:
            print(f"Error validating {file_path}: {e}")
            return {'violations': 0, 'success': False}
    
    def _run_fixes(self, file_path: str) -> Dict:
        """Run fixes on a single file."""
        try:
            result = subprocess.run([
                'python3', 'scripts/fix_remaining_grammar_violations.py', file_path
            ], capture_output=True, text=True, timeout=120)
            
            # Parse output for fixes applied
            fixes_applied = 0
            if "Applied" in result.stdout and "fixes" in result.stdout:
                for line in result.stdout.split('\n'):
                    if "Applied" in line and "fixes" in line:
                        try:
                            fixes_applied = int(line.split()[1])
                        except:
                            pass
                        break
            
            return {'fixes_applied': fixes_applied, 'success': result.returncode == 0}
        except Exception as e:
            print(f"Error fixing {file_path}: {e}")
            return {'fixes_applied': 0, 'success': False}
    
    def _load_historical_metrics(self, days_back: int) -> List[ComplianceMetrics]:
        """Load historical compliance metrics."""
        metrics = []
        cutoff_date = datetime.now() - timedelta(days=days_back)
        
        for file_path in self.output_dir.glob("compliance_*.json"):
            try:
                with open(file_path, 'r') as f:
                    data = json.load(f)
                    metric_date = datetime.fromisoformat(data['timestamp'])
                    if metric_date >= cutoff_date:
                        metrics.append(ComplianceMetrics(**data))
            except Exception as e:
                print(f"Error loading {file_path}: {e}")
        
        return sorted(metrics, key=lambda m: m.timestamp)

def main():
    """Main function for command-line usage."""
    parser = argparse.ArgumentParser(description="Collect grammar conformance metrics")
    parser.add_argument('--scan-paths', nargs='+', default=['docs/', 'examples/', 'stdlib/'],
                       help='Paths to scan for compliance metrics')
    parser.add_argument('--test-files', nargs='+', 
                       default=['docs/spec/grammar.md', 'docs/spec/types.md'],
                       help='Files to use for performance testing')
    parser.add_argument('--output-dir', default='metrics',
                       help='Directory to save metrics')
    parser.add_argument('--trends-days', type=int, default=30,
                       help='Number of days for trend analysis')
    parser.add_argument('--report-only', action='store_true',
                       help='Generate report from latest metrics only')
    
    args = parser.parse_args()
    
    collector = GrammarMetricsCollector(args.output_dir)
    
    if args.report_only:
        # Load latest metrics and generate report
        latest_file = Path(args.output_dir) / "latest_metrics.json"
        if latest_file.exists():
            with open(latest_file, 'r') as f:
                data = json.load(f)
                compliance = ComplianceMetrics(**data['compliance'])
                performance = PerformanceMetrics(**data['performance'])
                trends = TrendMetrics(**data['trends']) if data['trends'] else None
                
                report = collector.generate_report(compliance, performance, trends)
                print(report)
        else:
            print("No latest metrics found. Run without --report-only first.")
        return
    
    # Collect new metrics
    compliance = collector.collect_compliance_metrics(args.scan_paths)
    performance = collector.collect_performance_metrics(args.test_files)
    trends = collector.analyze_trends(args.trends_days)
    
    # Generate and display report
    report = collector.generate_report(compliance, performance, trends)
    print(report)
    
    # Save metrics
    collector.save_metrics(compliance, performance, trends)
    
    # Save report
    report_file = Path(args.output_dir) / f"report_{collector.timestamp.replace(':', '-')}.md"
    with open(report_file, 'w') as f:
        f.write(report)
    
    print(f"\nReport saved to {report_file}")

if __name__ == "__main__":
    main() 
