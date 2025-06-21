#!/usr/bin/env python3
"""
Asthra LLM Training Data Preparation Pipeline
Version: 1.0
Created: January 2025

This script collects and processes all Asthra source files for LLM training,
implementing the comprehensive data preparation plan.
"""

import os
import sys
import json
import yaml
import hashlib
import argparse
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass, asdict
from datetime import datetime

# Add src to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

@dataclass
class DataSource:
    """Represents a single training data source file."""
    path: str
    type: str  # 'grammar', 'spec', 'manual', 'code', 'test'
    priority: int  # 1-5, higher = more important
    size_kb: float
    quality_score: float = 0.0
    content_hash: str = ""
    
class AsthraDatassetCollector:
    """Main data collection and processing pipeline."""
    
    def __init__(self, asthra_repo_path: str, output_path: str = "./data"):
        self.repo_path = Path(asthra_repo_path)
        self.output_path = Path(output_path)
        self.data_sources: List[DataSource] = []
        self.total_size_kb = 0.0
        self.quality_validator = QualityValidator(asthra_repo_path)
        
        # Ensure output directories exist
        self.raw_data_path = self.output_path / "raw"
        self.processed_data_path = self.output_path / "processed"
        self.quality_path = self.output_path / "quality"
        
        for path in [self.raw_data_path, self.processed_data_path, self.quality_path]:
            path.mkdir(parents=True, exist_ok=True)
    
    def collect_all_sources(self) -> Dict[str, List[DataSource]]:
        """Collect all training data sources."""
        print("🔍 Collecting training data sources...")
        
        sources = {
            'grammar': self._collect_grammar_sources(),
            'specification': self._collect_spec_sources(), 
            'user_manual': self._collect_manual_sources(),
            'examples': self._collect_example_sources(),
            'stdlib': self._collect_stdlib_sources(),
            'ai_docs': self._collect_ai_documentation(),
            'tests': self._collect_test_patterns()
        }
        
        # Calculate totals
        total_files = sum(len(source_list) for source_list in sources.values())
        self.total_size_kb = sum(source.size_kb for source_list in sources.values() for source in source_list)
        
        print(f"✅ Collected {total_files} files ({self.total_size_kb:.1f} KB total)")
        return sources
    
    def _collect_grammar_sources(self) -> List[DataSource]:
        """Extract grammar specification and parsing rules."""
        sources = []
        
        # Main grammar file
        grammar_file = self.repo_path / 'docs' / 'grammar.txt'
        if grammar_file.exists():
            sources.append(self._create_data_source(
                path=str(grammar_file),
                type='grammar',
                priority=5  # Highest priority
            ))
        
        print(f"  📋 Grammar: {len(sources)} files")
        return sources
    
    def _collect_spec_sources(self) -> List[DataSource]:
        """Extract technical specification documents."""
        spec_dir = self.repo_path / 'docs' / 'spec'
        sources = []
        
        if not spec_dir.exists():
            print(f"  ⚠️  Spec directory not found: {spec_dir}")
            return sources
        
        priority_files = {
            'grammar.md': 5,
            'overview.md': 5,
            'types.md': 4,
            'advanced-features.md': 4,
            'packages.md': 3,
            'imports.md': 3,
            'predeclared.md': 3,
            'design_rationale.md': 2
        }
        
        for file_path in spec_dir.glob('*.md'):
            priority = priority_files.get(file_path.name, 2)
            sources.append(self._create_data_source(
                path=str(file_path),
                type='spec',
                priority=priority
            ))
        
        print(f"  📚 Specification: {len(sources)} files")
        return sources
    
    def _collect_manual_sources(self) -> List[DataSource]:
        """Extract user manual documentation."""
        manual_dir = self.repo_path / 'docs' / 'user-manual'
        sources = []
        
        if not manual_dir.exists():
            print(f"  ⚠️  User manual directory not found: {manual_dir}")
            return sources
        
        priority_files = {
            'language-fundamentals.md': 5,
            'object-oriented.md': 4,
            'concurrency.md': 4,
            'ffi-interop.md': 4,
            'ai_generation_guidelines.md': 5,
            'memory-management.md': 4,
            'getting-started.md': 3,
            'building-projects.md': 3
        }
        
        for file_path in manual_dir.glob('*.md'):
            priority = priority_files.get(file_path.name, 2)
            sources.append(self._create_data_source(
                path=str(file_path),
                type='manual',
                priority=priority
            ))
        
        print(f"  📖 User Manual: {len(sources)} files")
        return sources
    
    def _collect_example_sources(self) -> List[DataSource]:
        """Extract example programs and code samples."""
        examples_dir = self.repo_path / 'examples'
        sources = []
        
        if not examples_dir.exists():
            print(f"  ⚠️  Examples directory not found: {examples_dir}")
            return sources
        
        priority_files = {
            'json_examples.asthra': 5,
            'ffi_complete_example.asthra': 4,
            'advanced_concurrency_example.asthra': 4,
            'hello_world.asthra': 3,
            'basic_types.asthra': 3
        }
        
        for file_path in examples_dir.glob('*.asthra'):
            priority = priority_files.get(file_path.name, 2)
            sources.append(self._create_data_source(
                path=str(file_path),
                type='code',
                priority=priority
            ))
        
        print(f"  💻 Examples: {len(sources)} files")
        return sources
    
    def _collect_stdlib_sources(self) -> List[DataSource]:
        """Extract standard library implementation."""
        stdlib_dir = self.repo_path / 'stdlib'
        sources = []
        
        if not stdlib_dir.exists():
            print(f"  ⚠️  Stdlib directory not found: {stdlib_dir}")
            return sources
        
        priority_modules = {
            'json.asthra': 4,
            'http': 3,
            'crypto': 3,
            'concurrent': 4,
            'fs': 3
        }
        
        # Collect .asthra files
        for file_path in stdlib_dir.rglob('*.asthra'):
            module_name = file_path.parent.name if file_path.parent != stdlib_dir else file_path.name
            priority = priority_modules.get(module_name, 2)
            sources.append(self._create_data_source(
                path=str(file_path),
                type='code',
                priority=priority
            ))
        
        print(f"  📦 Standard Library: {len(sources)} files")
        return sources
    
    def _collect_ai_documentation(self) -> List[DataSource]:
        """Extract AI-specific documentation and guidelines."""
        sources = []
        
        # AI generation guidelines
        ai_docs = [
            'docs/user-manual/ai_generation_guidelines.md',
            'docs/AI_CODE_GENERATION_CHAMPION_MASTER_PLAN.md',
            'docs/LLM_FINE_TUNING_MASTER_PLAN.md'
        ]
        
        for doc_path in ai_docs:
            full_path = self.repo_path / doc_path
            if full_path.exists():
                sources.append(self._create_data_source(
                    path=str(full_path),
                    type='ai_docs',
                    priority=5
                ))
        
        # Implementation plans with AI patterns
        plans_pattern = self.repo_path / 'docs' / '*IMPLEMENTATION_PLAN.md'
        for plan_file in self.repo_path.glob('docs/*IMPLEMENTATION_PLAN.md'):
            sources.append(self._create_data_source(
                path=str(plan_file),
                type='ai_docs',
                priority=3
            ))
        
        print(f"  🤖 AI Documentation: {len(sources)} files")
        return sources
    
    def _collect_test_patterns(self) -> List[DataSource]:
        """Extract test patterns for learning correct usage."""
        tests_dir = self.repo_path / 'tests'
        sources = []
        
        if not tests_dir.exists():
            print(f"  ⚠️  Tests directory not found: {tests_dir}")
            return sources
        
        # Collect representative test files (not all, to avoid noise)
        test_categories = ['parser', 'semantic', 'codegen', 'integration']
        max_files_per_category = 5
        
        for category in test_categories:
            category_dir = tests_dir / category
            if category_dir.exists():
                test_files = list(category_dir.glob('*.c'))[:max_files_per_category]
                for test_file in test_files:
                    sources.append(self._create_data_source(
                        path=str(test_file),
                        type='test',
                        priority=2
                    ))
        
        print(f"  🧪 Test Patterns: {len(sources)} files")
        return sources
    
    def _create_data_source(self, path: str, type: str, priority: int) -> DataSource:
        """Create a DataSource object with calculated metadata."""
        file_path = Path(path)
        size_kb = file_path.stat().st_size / 1024 if file_path.exists() else 0.0
        
        # Calculate content hash
        content_hash = ""
        if file_path.exists():
            with open(file_path, 'rb') as f:
                content_hash = hashlib.md5(f.read()).hexdigest()
        
        return DataSource(
            path=path,
            type=type,
            priority=priority,
            size_kb=size_kb,
            content_hash=content_hash
        )
    
    def process_sources(self, sources: Dict[str, List[DataSource]]) -> Dict[str, any]:
        """Process collected sources into training datasets."""
        print("\n🔄 Processing sources into training datasets...")
        
        # Copy raw files to organized structure
        self._organize_raw_files(sources)
        
        # Process into training formats
        datasets = {
            'instruction_tuning': self._create_instruction_dataset(sources),
            'code_completion': self._create_completion_dataset(sources),
            'explanation': self._create_explanation_dataset(sources),
            'validation': self._create_validation_dataset(sources)
        }
        
        # Generate quality reports
        quality_report = self._generate_quality_report(sources, datasets)
        
        return {
            'datasets': datasets,
            'quality_report': quality_report,
            'sources_summary': self._create_sources_summary(sources)
        }
    
    def _organize_raw_files(self, sources: Dict[str, List[DataSource]]):
        """Copy source files to organized raw data structure."""
        print("  📁 Organizing raw files...")
        
        for category, source_list in sources.items():
            category_dir = self.raw_data_path / category
            category_dir.mkdir(exist_ok=True)
            
            for source in source_list:
                src_path = Path(source.path)
                if src_path.exists():
                    dst_path = category_dir / src_path.name
                    
                    # Copy file
                    with open(src_path, 'r', encoding='utf-8', errors='ignore') as src_file:
                        content = src_file.read()
                    
                    with open(dst_path, 'w', encoding='utf-8') as dst_file:
                        dst_file.write(content)
        
        print(f"    ✅ Raw files organized in {self.raw_data_path}")
    
    def _create_instruction_dataset(self, sources: Dict[str, List[DataSource]]) -> List[Dict]:
        """Create instruction-tuning dataset."""
        print("  📝 Creating instruction tuning dataset...")
        
        instructions = []
        
        # Grammar and specification instruction pairs
        for source in sources.get('grammar', []) + sources.get('specification', []):
            content = self._read_file_content(source.path)
            if content:
                instructions.extend(self._extract_grammar_instructions(content, source))
        
        # Code examples as instruction pairs
        for source in sources.get('examples', []) + sources.get('stdlib', []):
            if source.path.endswith('.asthra'):
                content = self._read_file_content(source.path)
                if content:
                    instructions.extend(self._extract_code_instructions(content, source))
        
        # Save dataset
        output_file = self.processed_data_path / 'instruction_tuning.jsonl'
        with open(output_file, 'w') as f:
            for instruction in instructions:
                f.write(json.dumps(instruction) + '\n')
        
        print(f"    ✅ Created {len(instructions)} instruction pairs")
        return instructions
    
    def _create_completion_dataset(self, sources: Dict[str, List[DataSource]]) -> List[Dict]:
        """Create code completion dataset."""
        print("  🔄 Creating code completion dataset...")
        
        completions = []
        
        # Extract completion examples from code files
        for source in sources.get('examples', []) + sources.get('stdlib', []):
            if source.path.endswith('.asthra'):
                content = self._read_file_content(source.path)
                if content:
                    completions.extend(self._extract_completion_pairs(content, source))
        
        # Save dataset
        output_file = self.processed_data_path / 'code_completion.jsonl'
        with open(output_file, 'w') as f:
            for completion in completions:
                f.write(json.dumps(completion) + '\n')
        
        print(f"    ✅ Created {len(completions)} completion pairs")
        return completions
    
    def _create_explanation_dataset(self, sources: Dict[str, List[DataSource]]) -> List[Dict]:
        """Create code explanation dataset."""
        print("  📚 Creating explanation dataset...")
        
        explanations = []
        
        # Extract explanations from documentation
        for source in sources.get('user_manual', []) + sources.get('ai_docs', []):
            content = self._read_file_content(source.path)
            if content:
                explanations.extend(self._extract_explanations(content, source))
        
        # Save dataset
        output_file = self.processed_data_path / 'explanation.jsonl'
        with open(output_file, 'w') as f:
            for explanation in explanations:
                f.write(json.dumps(explanation) + '\n')
        
        print(f"    ✅ Created {len(explanations)} explanation pairs")
        return explanations
    
    def _create_validation_dataset(self, sources: Dict[str, List[DataSource]]) -> List[Dict]:
        """Create validation dataset for testing."""
        print("  ✅ Creating validation dataset...")
        
        # Use 10% of high-quality examples for validation
        high_quality_sources = []
        for category, source_list in sources.items():
            high_quality = [s for s in source_list if s.priority >= 4]
            high_quality_sources.extend(high_quality[:max(1, len(high_quality) // 10)])
        
        validation_data = []
        for source in high_quality_sources:
            content = self._read_file_content(source.path)
            if content:
                validation_data.append({
                    'source': source.path,
                    'type': source.type,
                    'content': content[:2000],  # Truncate for validation
                    'priority': source.priority
                })
        
        # Save dataset
        output_file = self.processed_data_path / 'validation.jsonl'
        with open(output_file, 'w') as f:
            for item in validation_data:
                f.write(json.dumps(item) + '\n')
        
        print(f"    ✅ Created {len(validation_data)} validation samples")
        return validation_data
    
    def _extract_grammar_instructions(self, content: str, source: DataSource) -> List[Dict]:
        """Extract instruction pairs from grammar files."""
        instructions = []
        
        # Simple pattern: extract grammar rules and create instruction pairs
        lines = content.split('\n')
        current_rule = ""
        
        for line in lines:
            line = line.strip()
            if line and not line.startswith('#'):
                if '<-' in line:  # PEG rule
                    parts = line.split('<-', 1)
                    if len(parts) == 2:
                        rule_name = parts[0].strip()
                        rule_definition = parts[1].strip()
                        
                        instructions.append({
                            'instruction': f"Define the Asthra grammar rule for {rule_name}",
                            'response': f"{rule_name} <- {rule_definition}",
                            'source': source.path,
                            'type': 'grammar'
                        })
        
        return instructions
    
    def _extract_code_instructions(self, content: str, source: DataSource) -> List[Dict]:
        """Extract instruction pairs from code files."""
        instructions = []
        
        # Extract functions and create instruction pairs
        lines = content.split('\n')
        in_function = False
        current_function = []
        function_signature = ""
        
        for line in lines:
            if 'fn ' in line and '(' in line:  # Function start
                if current_function:
                    # Save previous function
                    function_body = '\n'.join(current_function)
                    if function_signature and function_body:
                        instructions.append({
                            'instruction': f"Implement this Asthra function: {function_signature}",
                            'response': function_body,
                            'source': source.path,
                            'type': 'code'
                        })
                
                function_signature = line.strip()
                current_function = [line]
                in_function = True
            elif in_function:
                current_function.append(line)
                if line.strip() == '}' and len([l for l in current_function if '{' in l]) == len([l for l in current_function if '}' in l]):
                    in_function = False
        
        # Handle last function
        if current_function and function_signature:
            function_body = '\n'.join(current_function)
            instructions.append({
                'instruction': f"Implement this Asthra function: {function_signature}",
                'response': function_body,
                'source': source.path,
                'type': 'code'
            })
        
        return instructions
    
    def _extract_completion_pairs(self, content: str, source: DataSource) -> List[Dict]:
        """Extract code completion pairs."""
        completions = []
        
        lines = content.split('\n')
        
        # Create completion pairs by taking prefixes and suffixes
        for i in range(len(lines)):
            if len(lines[i].strip()) > 10:  # Meaningful lines
                prefix = '\n'.join(lines[max(0, i-2):i])
                current_line = lines[i]
                
                # Create completion pair
                if prefix.strip() and current_line.strip():
                    completions.append({
                        'prompt': prefix,
                        'completion': current_line,
                        'source': source.path,
                        'type': 'completion'
                    })
        
        return completions
    
    def _extract_explanations(self, content: str, source: DataSource) -> List[Dict]:
        """Extract explanation pairs from documentation."""
        explanations = []
        
        # Simple pattern: find code blocks followed by explanations
        lines = content.split('\n')
        current_code = []
        in_code_block = False
        
        for i, line in enumerate(lines):
            if line.strip().startswith('```asthra') or line.strip().startswith('```'):
                if not in_code_block:
                    in_code_block = True
                    current_code = []
                else:
                    in_code_block = False
                    if current_code:
                        code_text = '\n'.join(current_code)
                        
                        # Look for explanation in next few lines
                        explanation = ""
                        for j in range(i+1, min(i+5, len(lines))):
                            if lines[j].strip() and not lines[j].startswith('#'):
                                explanation = lines[j].strip()
                                break
                        
                        if explanation:
                            explanations.append({
                                'code': code_text,
                                'explanation': explanation,
                                'source': source.path,
                                'type': 'explanation'
                            })
            elif in_code_block:
                current_code.append(line)
        
        return explanations
    
    def _read_file_content(self, file_path: str) -> Optional[str]:
        """Safely read file content."""
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                return f.read()
        except Exception as e:
            print(f"    ⚠️  Error reading {file_path}: {e}")
            return None
    
    def _generate_quality_report(self, sources: Dict[str, List[DataSource]], datasets: Dict[str, List]) -> Dict:
        """Generate comprehensive quality report."""
        print("  📊 Generating quality report...")
        
        # Calculate quality scores
        for category, source_list in sources.items():
            for source in source_list:
                source.quality_score = self.quality_validator.calculate_quality_score(source)
        
        # Generate report
        report = {
            'timestamp': datetime.now().isoformat(),
            'summary': {
                'total_files': sum(len(source_list) for source_list in sources.values()),
                'total_size_kb': self.total_size_kb,
                'average_quality': sum(source.quality_score for source_list in sources.values() for source in source_list) / sum(len(source_list) for source_list in sources.values()) if sum(len(source_list) for source_list in sources.values()) > 0 else 0,
                'dataset_sizes': {name: len(data) for name, data in datasets.items()}
            },
            'by_category': {},
            'quality_distribution': self._calculate_quality_distribution(sources)
        }
        
        for category, source_list in sources.items():
            report['by_category'][category] = {
                'file_count': len(source_list),
                'total_size_kb': sum(s.size_kb for s in source_list),
                'average_quality': sum(s.quality_score for s in source_list) / len(source_list) if source_list else 0,
                'high_quality_files': len([s for s in source_list if s.quality_score >= 0.8])
            }
        
        # Save report
        report_file = self.quality_path / 'quality_report.json'
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"    ✅ Quality report saved to {report_file}")
        return report
    
    def _calculate_quality_distribution(self, sources: Dict[str, List[DataSource]]) -> Dict:
        """Calculate quality score distribution."""
        all_scores = [source.quality_score for source_list in sources.values() for source in source_list]
        
        if not all_scores:
            return {}
        
        return {
            'excellent': len([s for s in all_scores if s >= 0.9]),
            'good': len([s for s in all_scores if 0.7 <= s < 0.9]),
            'fair': len([s for s in all_scores if 0.5 <= s < 0.7]),
            'poor': len([s for s in all_scores if s < 0.5])
        }
    
    def _create_sources_summary(self, sources: Dict[str, List[DataSource]]) -> Dict:
        """Create summary of all sources."""
        summary = {
            'categories': {},
            'top_priority_files': [],
            'largest_files': []
        }
        
        all_sources = []
        for category, source_list in sources.items():
            summary['categories'][category] = len(source_list)
            all_sources.extend(source_list)
        
        # Top priority files
        summary['top_priority_files'] = [
            {'path': s.path, 'priority': s.priority, 'size_kb': s.size_kb}
            for s in sorted(all_sources, key=lambda x: x.priority, reverse=True)[:10]
        ]
        
        # Largest files
        summary['largest_files'] = [
            {'path': s.path, 'size_kb': s.size_kb, 'type': s.type}
            for s in sorted(all_sources, key=lambda x: x.size_kb, reverse=True)[:10]
        ]
        
        return summary


class QualityValidator:
    """Validates data quality for training."""
    
    def __init__(self, asthra_repo_path: str):
        self.repo_path = Path(asthra_repo_path)
        self.asthra_compiler = self.repo_path / 'build' / 'asthra'
    
    def calculate_quality_score(self, source: DataSource) -> float:
        """Calculate comprehensive quality score (0-1)."""
        scores = []
        
        # File existence and readability
        scores.append(self._validate_file_accessibility(source))
        
        # Content quality
        scores.append(self._validate_content_quality(source))
        
        # Syntax validation (for code files)
        if source.path.endswith('.asthra'):
            scores.append(self._validate_asthra_syntax(source))
        else:
            scores.append(1.0)  # Non-code files pass syntax check
        
        # AI training suitability
        scores.append(self._validate_ai_suitability(source))
        
        return sum(scores) / len(scores)
    
    def _validate_file_accessibility(self, source: DataSource) -> float:
        """Check if file is accessible and readable."""
        try:
            with open(source.path, 'r', encoding='utf-8') as f:
                content = f.read(100)  # Read first 100 chars
                return 1.0 if content else 0.5
        except:
            return 0.0
    
    def _validate_content_quality(self, source: DataSource) -> float:
        """Validate content quality."""
        try:
            with open(source.path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            # Basic quality checks
            if len(content) < 50:
                return 0.3  # Too short
            
            if len(content) > 100000:
                return 0.7  # Very long files might be harder to learn from
            
            # Check for reasonable structure
            lines = content.split('\n')
            non_empty_lines = [line for line in lines if line.strip()]
            
            if len(non_empty_lines) / len(lines) < 0.3:
                return 0.6  # Too much whitespace
            
            return 1.0
            
        except:
            return 0.0
    
    def _validate_asthra_syntax(self, source: DataSource) -> float:
        """Validate Asthra code syntax using compiler."""
        if not self.asthra_compiler.exists():
            return 0.8  # Cannot validate, assume good
        
        try:
            # Try to parse the file with Asthra compiler
            result = subprocess.run(
                [str(self.asthra_compiler), '--syntax-check', source.path],
                capture_output=True,
                timeout=10
            )
            return 1.0 if result.returncode == 0 else 0.5
        except:
            return 0.8  # Compilation test failed, but file might still be valid
    
    def _validate_ai_suitability(self, source: DataSource) -> float:
        """Check if content is suitable for AI training."""
        try:
            with open(source.path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            # Look for AI-friendly patterns
            ai_friendly_indicators = [
                'fn ',  # Function definitions
                'struct ',  # Struct definitions
                'impl ',  # Implementation blocks
                'let ',  # Variable declarations
                'pub ',  # Public declarations
                '// ',  # Comments
            ]
            
            score = 0.5  # Base score
            for indicator in ai_friendly_indicators:
                if indicator in content:
                    score += 0.1
            
            return min(score, 1.0)
            
        except:
            return 0.5


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description="Prepare Asthra training data for LLM fine-tuning")
    parser.add_argument('--asthra-repo', default='..', help='Path to Asthra repository')
    parser.add_argument('--output', default='./data', help='Output directory for processed data')
    parser.add_argument('--config', default='configs/devstral_config.yaml', help='Configuration file')
    parser.add_argument('--quality-threshold', type=float, default=0.7, help='Minimum quality score')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    print("🚀 Asthra LLM Training Data Preparation")
    print("=" * 50)
    print(f"Asthra repo: {args.asthra_repo}")
    print(f"Output directory: {args.output}")
    print(f"Quality threshold: {args.quality_threshold}")
    print()
    
    try:
        # Initialize collector
        collector = AsthraDatassetCollector(args.asthra_repo, args.output)
        
        # Collect sources
        sources = collector.collect_all_sources()
        
        # Process and create datasets
        results = collector.process_sources(sources)
        
        # Print summary
        print("\n📊 Data Preparation Summary")
        print("=" * 30)
        quality_report = results['quality_report']
        
        print(f"Total files processed: {quality_report['summary']['total_files']}")
        print(f"Total size: {quality_report['summary']['total_size_kb']:.1f} KB")
        print(f"Average quality score: {quality_report['summary']['average_quality']:.2f}")
        print()
        
        print("Dataset sizes:")
        for name, size in quality_report['summary']['dataset_sizes'].items():
            print(f"  {name}: {size} samples")
        print()
        
        print("Quality distribution:")
        dist = quality_report['quality_distribution']
        print(f"  Excellent (≥0.9): {dist.get('excellent', 0)} files")
        print(f"  Good (0.7-0.9): {dist.get('good', 0)} files")
        print(f"  Fair (0.5-0.7): {dist.get('fair', 0)} files")
        print(f"  Poor (<0.5): {dist.get('poor', 0)} files")
        
        print("\n✅ Data preparation completed successfully!")
        print(f"📁 Processed data available in: {args.output}/processed/")
        print(f"📊 Quality report available in: {args.output}/quality/quality_report.json")
        
    except Exception as e:
        print(f"\n❌ Error during data preparation: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main() 
