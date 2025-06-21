#!/usr/bin/env python3
"""
Asthra Test Trend Analysis Tool

This tool provides comprehensive test trend analysis for the unified testing framework,
supporting the Phase 5 tooling objectives of the Testing Framework Standardization Plan.

Features:
- Historical test result analysis
- Performance trend tracking
- Failure pattern identification
- Category-wise metrics
- CI/CD integration support
- Multiple output formats (JSON, HTML, CSV)

Usage:
    python3 scripts/test_trend_analysis.py --since 2024-01-01 --categories parser,semantic
    python3 scripts/test_trend_analysis.py --format html --output trend-report.html
"""

import argparse
import json
import csv
import os
import sys
import datetime
from pathlib import Path
from typing import Dict, List, Any, Optional
import sqlite3
import glob
import re

class TestTrendAnalyzer:
    """Comprehensive test trend analysis for Asthra testing framework."""
    
    def __init__(self, results_dir: str = "build/test-results"):
        self.results_dir = Path(results_dir)
        self.db_path = self.results_dir / "test_history.db"
        self.setup_database()
    
    def setup_database(self):
        """Initialize SQLite database for test history tracking."""
        self.results_dir.mkdir(parents=True, exist_ok=True)
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Create test runs table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS test_runs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                format TEXT,
                parallel BOOLEAN,
                timeout INTEGER,
                total_categories INTEGER,
                passed_categories INTEGER,
                failed_categories INTEGER,
                total_tests INTEGER,
                passed_tests INTEGER,
                failed_tests INTEGER,
                total_duration INTEGER,
                pass_rate REAL
            )
        ''')
        
        # Create category results table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS category_results (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                run_id INTEGER,
                category TEXT NOT NULL,
                status TEXT NOT NULL,
                tests_run INTEGER,
                tests_passed INTEGER,
                tests_failed INTEGER,
                duration INTEGER,
                start_time TEXT,
                end_time TEXT,
                FOREIGN KEY (run_id) REFERENCES test_runs (id)
            )
        ''')
        
        # Create performance metrics table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS performance_metrics (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                run_id INTEGER,
                category TEXT,
                metric_name TEXT,
                metric_value REAL,
                timestamp TEXT,
                FOREIGN KEY (run_id) REFERENCES test_runs (id)
            )
        ''')
        
        conn.commit()
        conn.close()
    
    def import_test_results(self, results_file: str) -> int:
        """Import test results from JSON file into database."""
        with open(results_file, 'r') as f:
            data = json.load(f)
        
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Extract run metadata
        test_run = data.get('test_run', {})
        timestamp = test_run.get('start_time', datetime.datetime.now().isoformat())
        
        # Calculate aggregated metrics
        categories = test_run.get('categories', [])
        total_categories = len(categories)
        passed_categories = sum(1 for c in categories if c.get('status') == 'PASSED')
        failed_categories = total_categories - passed_categories
        total_tests = sum(c.get('tests_run', 0) for c in categories)
        passed_tests = sum(c.get('tests_passed', 0) for c in categories)
        failed_tests = sum(c.get('tests_failed', 0) for c in categories)
        total_duration = sum(c.get('duration', 0) for c in categories)
        pass_rate = (passed_tests / total_tests * 100) if total_tests > 0 else 0
        
        # Insert test run
        cursor.execute('''
            INSERT INTO test_runs (
                timestamp, format, parallel, timeout,
                total_categories, passed_categories, failed_categories,
                total_tests, passed_tests, failed_tests,
                total_duration, pass_rate
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            timestamp, test_run.get('format', 'standard'),
            test_run.get('parallel', False), test_run.get('timeout', 30),
            total_categories, passed_categories, failed_categories,
            total_tests, passed_tests, failed_tests,
            total_duration, pass_rate
        ))
        
        run_id = cursor.lastrowid
        
        # Insert category results
        for category in categories:
            cursor.execute('''
                INSERT INTO category_results (
                    run_id, category, status, tests_run,
                    tests_passed, tests_failed, duration,
                    start_time, end_time
                ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''', (
                run_id, category.get('category', ''),
                category.get('status', ''), category.get('tests_run', 0),
                category.get('tests_passed', 0), category.get('tests_failed', 0),
                category.get('duration', 0), category.get('start_time', ''),
                category.get('end_time', '')
            ))
        
        conn.commit()
        conn.close()
        
        return run_id
    
    def analyze_trends(self, since: str = None, categories: List[str] = None) -> Dict[str, Any]:
        """Analyze test trends with optional filtering."""
        conn = sqlite3.connect(self.db_path)
        
        # Build WHERE clause
        where_conditions = []
        params = []
        
        if since:
            where_conditions.append("timestamp >= ?")
            params.append(since)
        
        if categories:
            category_placeholders = ','.join('?' * len(categories))
            where_conditions.append(f"category IN ({category_placeholders})")
            params.extend(categories)
        
        where_clause = " AND ".join(where_conditions) if where_conditions else "1=1"
        
        # Overall trends
        cursor = conn.cursor()
        cursor.execute(f'''
            SELECT 
                COUNT(*) as total_runs,
                AVG(pass_rate) as avg_pass_rate,
                MIN(pass_rate) as min_pass_rate,
                MAX(pass_rate) as max_pass_rate,
                AVG(total_duration) as avg_duration,
                MIN(total_duration) as min_duration,
                MAX(total_duration) as max_duration
            FROM test_runs 
            WHERE {where_clause}
        ''', params)
        
        overall_stats = dict(zip([col[0] for col in cursor.description], cursor.fetchone() or []))
        
        # Category-wise trends
        cursor.execute(f'''
            SELECT 
                category,
                COUNT(*) as runs,
                AVG(CASE WHEN status = 'PASSED' THEN 1.0 ELSE 0.0 END) * 100 as success_rate,
                AVG(duration) as avg_duration,
                AVG(tests_run) as avg_tests,
                SUM(tests_failed) as total_failures
            FROM category_results 
            WHERE {where_clause}
            GROUP BY category
            ORDER BY success_rate DESC
        ''', params)
        
        category_trends = []
        for row in cursor.fetchall():
            category_trends.append(dict(zip([col[0] for col in cursor.description], row)))
        
        # Recent failure patterns
        cursor.execute(f'''
            SELECT 
                category,
                timestamp,
                tests_failed,
                duration
            FROM category_results cr
            JOIN test_runs tr ON cr.run_id = tr.id
            WHERE cr.status = 'FAILED' AND {where_clause}
            ORDER BY timestamp DESC
            LIMIT 20
        ''', params)
        
        recent_failures = []
        for row in cursor.fetchall():
            recent_failures.append(dict(zip([col[0] for col in cursor.description], row)))
        
        # Performance trends over time
        cursor.execute(f'''
            SELECT 
                DATE(timestamp) as date,
                AVG(pass_rate) as daily_pass_rate,
                AVG(total_duration) as daily_duration,
                COUNT(*) as daily_runs
            FROM test_runs 
            WHERE {where_clause}
            GROUP BY DATE(timestamp)
            ORDER BY date DESC
            LIMIT 30
        ''', params)
        
        daily_trends = []
        for row in cursor.fetchall():
            daily_trends.append(dict(zip([col[0] for col in cursor.description], row)))
        
        conn.close()
        
        return {
            'analysis_timestamp': datetime.datetime.now().isoformat(),
            'filters': {
                'since': since,
                'categories': categories
            },
            'overall_statistics': overall_stats,
            'category_trends': category_trends,
            'recent_failures': recent_failures,
            'daily_trends': daily_trends,
            'recommendations': self._generate_recommendations(overall_stats, category_trends, recent_failures)
        }
    
    def _generate_recommendations(self, overall_stats: Dict, category_trends: List[Dict], 
                                recent_failures: List[Dict]) -> List[str]:
        """Generate actionable recommendations based on trend analysis."""
        recommendations = []
        
        # Overall performance recommendations
        avg_pass_rate = overall_stats.get('avg_pass_rate', 0)
        if avg_pass_rate < 80:
            recommendations.append(
                f"⚠️ Overall pass rate is {avg_pass_rate:.1f}%. Consider investigating test stability."
            )
        elif avg_pass_rate > 95:
            recommendations.append(
                f"✅ Excellent pass rate of {avg_pass_rate:.1f}%. Consider adding more edge case tests."
            )
        
        # Category-specific recommendations
        if category_trends:
            worst_category = min(category_trends, key=lambda x: x.get('success_rate', 100))
            if worst_category.get('success_rate', 100) < 70:
                recommendations.append(
                    f"🔍 Category '{worst_category['category']}' has low success rate "
                    f"({worst_category['success_rate']:.1f}%). Needs attention."
                )
            
            # Performance recommendations
            slow_categories = [c for c in category_trends if c.get('avg_duration', 0) > 60]
            if slow_categories:
                slow_names = [c['category'] for c in slow_categories[:3]]
                recommendations.append(
                    f"⏱️ Slow categories detected: {', '.join(slow_names)}. Consider optimization."
                )
        
        # Failure pattern analysis
        if recent_failures:
            failure_categories = {}
            for failure in recent_failures:
                cat = failure['category']
                failure_categories[cat] = failure_categories.get(cat, 0) + 1
            
            frequent_failures = sorted(failure_categories.items(), key=lambda x: x[1], reverse=True)
            if frequent_failures and frequent_failures[0][1] > 3:
                recommendations.append(
                    f"🚨 Category '{frequent_failures[0][0]}' has {frequent_failures[0][1]} "
                    "recent failures. Investigate immediately."
                )
        
        if not recommendations:
            recommendations.append("✅ No significant issues detected. Test suite is performing well.")
        
        return recommendations
    
    def generate_html_report(self, analysis: Dict[str, Any], output_file: str):
        """Generate comprehensive HTML trend report."""
        html_content = f'''
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Asthra Test Trend Analysis</title>
    <style>
        body {{ font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 40px; }}
        .header {{ background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); 
                  color: white; padding: 30px; border-radius: 10px; margin-bottom: 30px; }}
        .metric-grid {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); 
                       gap: 20px; margin-bottom: 30px; }}
        .metric-card {{ background: white; border: 1px solid #ddd; border-radius: 8px; 
                       padding: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .metric-value {{ font-size: 2em; font-weight: bold; color: #333; }}
        .metric-label {{ color: #666; margin-bottom: 10px; }}
        .recommendations {{ background: #f8f9fa; border-left: 4px solid #007bff; 
                          padding: 20px; margin: 20px 0; }}
        .category-table {{ width: 100%; border-collapse: collapse; margin: 20px 0; }}
        .category-table th, .category-table td {{ padding: 12px; text-align: left; 
                                                 border-bottom: 1px solid #ddd; }}
        .category-table th {{ background-color: #f8f9fa; }}
        .success-rate {{ padding: 4px 8px; border-radius: 4px; color: white; }}
        .success-high {{ background-color: #28a745; }}
        .success-medium {{ background-color: #ffc107; color: #212529; }}
        .success-low {{ background-color: #dc3545; }}
        .chart-container {{ margin: 30px 0; }}
    </style>
</head>
<body>
    <div class="header">
        <h1>🧪 Asthra Test Trend Analysis</h1>
        <p>Generated: {analysis['analysis_timestamp']}</p>
        {f"<p>Filters: Since {analysis['filters']['since']}" if analysis['filters']['since'] else ""}
        {f", Categories: {', '.join(analysis['filters']['categories'])}" if analysis['filters']['categories'] else ""}</p>
    </div>

    <div class="metric-grid">
        <div class="metric-card">
            <div class="metric-label">Total Test Runs</div>
            <div class="metric-value">{analysis['overall_statistics'].get('total_runs', 0)}</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Average Pass Rate</div>
            <div class="metric-value">{analysis['overall_statistics'].get('avg_pass_rate', 0):.1f}%</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Average Duration</div>
            <div class="metric-value">{analysis['overall_statistics'].get('avg_duration', 0):.0f}s</div>
        </div>
    </div>

    <div class="recommendations">
        <h3>📋 Recommendations</h3>
        <ul>
            {''.join(f'<li>{rec}</li>' for rec in analysis['recommendations'])}
        </ul>
    </div>

    <h3>📊 Category Performance Trends</h3>
    <table class="category-table">
        <thead>
            <tr>
                <th>Category</th>
                <th>Success Rate</th>
                <th>Avg Duration (s)</th>
                <th>Avg Tests</th>
                <th>Total Failures</th>
            </tr>
        </thead>
        <tbody>
'''
        
        for category in analysis['category_trends']:
            success_rate = category.get('success_rate', 0)
            success_class = 'success-high' if success_rate >= 90 else 'success-medium' if success_rate >= 70 else 'success-low'
            
            html_content += f'''
            <tr>
                <td><strong>{category['category']}</strong></td>
                <td><span class="success-rate {success_class}">{success_rate:.1f}%</span></td>
                <td>{category.get('avg_duration', 0):.1f}</td>
                <td>{category.get('avg_tests', 0):.1f}</td>
                <td>{category.get('total_failures', 0)}</td>
            </tr>
'''
        
        html_content += '''
        </tbody>
    </table>

    <h3>🚨 Recent Failures</h3>
    <table class="category-table">
        <thead>
            <tr>
                <th>Category</th>
                <th>Timestamp</th>
                <th>Failed Tests</th>
                <th>Duration (s)</th>
            </tr>
        </thead>
        <tbody>
'''
        
        for failure in analysis['recent_failures'][:10]:
            html_content += f'''
            <tr>
                <td>{failure['category']}</td>
                <td>{failure['timestamp']}</td>
                <td>{failure['tests_failed']}</td>
                <td>{failure['duration']}</td>
            </tr>
'''
        
        html_content += '''
        </tbody>
    </table>

    <div style="margin-top: 50px; text-align: center; color: #666;">
        <p>Generated by Asthra Testing Framework Trend Analysis Tool</p>
    </div>
</body>
</html>
'''
        
        with open(output_file, 'w') as f:
            f.write(html_content)
    
    def auto_import_recent_results(self):
        """Automatically import recent test results from the results directory."""
        pattern = str(self.results_dir / "results-*.json")
        result_files = glob.glob(pattern)
        
        imported_count = 0
        for result_file in result_files:
            try:
                self.import_test_results(result_file)
                imported_count += 1
                print(f"✅ Imported: {result_file}")
            except Exception as e:
                print(f"❌ Failed to import {result_file}: {e}")
        
        return imported_count

def main():
    parser = argparse.ArgumentParser(
        description="Asthra Test Trend Analysis Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  python3 scripts/test_trend_analysis.py --auto-import
  python3 scripts/test_trend_analysis.py --since 2024-01-01 --categories parser,semantic
  python3 scripts/test_trend_analysis.py --format html --output build/trend-report.html
  python3 scripts/test_trend_analysis.py --import results.json --analyze
        '''
    )
    
    parser.add_argument('--results-dir', default='build/test-results',
                       help='Test results directory (default: build/test-results)')
    parser.add_argument('--since', help='Analyze trends since date (YYYY-MM-DD)')
    parser.add_argument('--categories', help='Comma-separated list of categories to analyze')
    parser.add_argument('--format', choices=['json', 'html', 'csv'], default='json',
                       help='Output format (default: json)')
    parser.add_argument('--output', help='Output file (default: stdout for json)')
    parser.add_argument('--import', dest='import_file', help='Import test results from JSON file')
    parser.add_argument('--auto-import', action='store_true', 
                       help='Automatically import all recent results')
    parser.add_argument('--analyze', action='store_true', help='Perform trend analysis')
    
    args = parser.parse_args()
    
    analyzer = TestTrendAnalyzer(args.results_dir)
    
    # Import results if requested
    if args.import_file:
        run_id = analyzer.import_test_results(args.import_file)
        print(f"✅ Imported test results as run ID: {run_id}")
    
    if args.auto_import:
        count = analyzer.auto_import_recent_results()
        print(f"✅ Imported {count} result files")
    
    # Perform analysis if requested or if no import operations
    if args.analyze or (not args.import_file and not args.auto_import):
        categories = args.categories.split(',') if args.categories else None
        analysis = analyzer.analyze_trends(since=args.since, categories=categories)
        
        if args.format == 'json':
            output = json.dumps(analysis, indent=2)
            if args.output:
                with open(args.output, 'w') as f:
                    f.write(output)
                print(f"✅ JSON analysis saved to: {args.output}")
            else:
                print(output)
        
        elif args.format == 'html':
            output_file = args.output or 'test-trend-report.html'
            analyzer.generate_html_report(analysis, output_file)
            print(f"✅ HTML report generated: {output_file}")
        
        elif args.format == 'csv':
            output_file = args.output or 'test-trends.csv'
            with open(output_file, 'w', newline='') as csvfile:
                if analysis['category_trends']:
                    fieldnames = analysis['category_trends'][0].keys()
                    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                    writer.writeheader()
                    writer.writerows(analysis['category_trends'])
            print(f"✅ CSV data saved to: {output_file}")

if __name__ == '__main__':
    main() 
