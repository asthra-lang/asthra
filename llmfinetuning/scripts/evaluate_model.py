#!/usr/bin/env python3
"""
Asthra LLM Model Evaluation Script
Version: 1.0
Created: January 2025

Evaluate the fine-tuned Devstral-Small-2505 model on Asthra programming tasks.
"""

import os
import sys
import yaml
import torch
import argparse
import logging
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Any, Optional

# Add src to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from transformers import AutoModelForCausalLM, AutoTokenizer, pipeline
from peft import PeftModel
import json

def setup_logging(verbose: bool = False) -> logging.Logger:
    """Set up logging configuration."""
    log_level = logging.DEBUG if verbose else logging.INFO
    
    logging.basicConfig(
        level=log_level,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            logging.StreamHandler(),
            logging.FileHandler(f'evaluation_{datetime.now().strftime("%Y%m%d_%H%M%S")}.log')
        ]
    )
    
    return logging.getLogger('asthra_llm_evaluation')

def load_model_and_tokenizer(model_path: str, base_model_name: str, logger: logging.Logger):
    """Load the fine-tuned model and tokenizer."""
    logger.info(f"Loading model from {model_path}")
    
    # Load tokenizer
    tokenizer = AutoTokenizer.from_pretrained(model_path)
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token
    
    # Load base model
    base_model = AutoModelForCausalLM.from_pretrained(
        base_model_name,
        torch_dtype=torch.bfloat16,
        device_map="auto",
        trust_remote_code=True
    )
    
    # Load PEFT model if adapter exists
    adapter_path = Path(model_path) / "adapter_model.bin"
    if adapter_path.exists():
        logger.info("Loading LoRA adapter")
        model = PeftModel.from_pretrained(base_model, model_path)
    else:
        logger.info("No adapter found, using base model from path")
        model = AutoModelForCausalLM.from_pretrained(
            model_path,
            torch_dtype=torch.bfloat16,
            device_map="auto",
            trust_remote_code=True
        )
    
    return model, tokenizer

class AsthraSyntaxEvaluator:
    """Evaluate syntax correctness of generated Asthra code."""
    
    def __init__(self, asthra_compiler_path: str):
        self.compiler_path = Path(asthra_compiler_path)
        self.temp_dir = Path("/tmp/asthra_eval")
        self.temp_dir.mkdir(exist_ok=True)
    
    def evaluate_syntax(self, code: str) -> Dict[str, Any]:
        """Evaluate if generated code has correct syntax."""
        # Create temporary file
        temp_file = self.temp_dir / f"test_{datetime.now().strftime('%H%M%S%f')}.asthra"
        
        try:
            with open(temp_file, 'w') as f:
                f.write(code)
            
            # Run syntax check
            result = subprocess.run(
                [str(self.compiler_path), '--syntax-check', str(temp_file)],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            return {
                'valid': result.returncode == 0,
                'error_message': result.stderr if result.returncode != 0 else None,
                'file_path': str(temp_file)
            }
            
        except subprocess.TimeoutExpired:
            return {
                'valid': False,
                'error_message': 'Compilation timeout',
                'file_path': str(temp_file)
            }
        except Exception as e:
            return {
                'valid': False,
                'error_message': str(e),
                'file_path': str(temp_file)
            }
        finally:
            # Cleanup
            if temp_file.exists():
                temp_file.unlink()

class AsthraBenchmarkSuite:
    """Comprehensive benchmark suite for Asthra code generation."""
    
    def __init__(self):
        self.test_cases = [
            {
                'name': 'basic_function',
                'prompt': 'Write a simple Asthra function that adds two integers:',
                'expected_keywords': ['fn', 'i32', 'return'],
                'category': 'basic'
            },
            {
                'name': 'struct_definition',
                'prompt': 'Define an Asthra struct called Person with name and age fields:',
                'expected_keywords': ['struct', 'Person', 'pub'],
                'category': 'basic'
            },
            {
                'name': 'error_handling',
                'prompt': 'Write an Asthra function that returns a Result type for safe division:',
                'expected_keywords': ['Result', 'Ok', 'Err', 'fn'],
                'category': 'advanced'
            },
            {
                'name': 'concurrency',
                'prompt': 'Create an Asthra function that spawns a concurrent task:',
                'expected_keywords': ['spawn', 'fn', 'async'],
                'category': 'advanced'
            },
            {
                'name': 'pattern_matching',
                'prompt': 'Write an Asthra match statement for Option type:',
                'expected_keywords': ['match', 'Some', 'None', 'Option'],
                'category': 'intermediate'
            },
            {
                'name': 'memory_management',
                'prompt': 'Create an Asthra function that safely manages a vector:',
                'expected_keywords': ['Vec', 'fn', 'mut', 'let'],
                'category': 'intermediate'
            }
        ]
    
    def run_benchmark(self, model, tokenizer, syntax_evaluator: Optional[AsthraSyntaxEvaluator], logger: logging.Logger) -> Dict[str, Any]:
        """Run complete benchmark suite."""
        logger.info("Running Asthra benchmark suite...")
        
        results = {
            'timestamp': datetime.now().isoformat(),
            'test_results': [],
            'summary': {
                'total_tests': len(self.test_cases),
                'syntax_valid': 0,
                'keyword_matches': 0,
                'by_category': {}
            }
        }
        
        # Create text generation pipeline
        generator = pipeline(
            "text-generation",
            model=model,
            tokenizer=tokenizer,
            torch_dtype=torch.bfloat16,
            device_map="auto"
        )
        
        for i, test_case in enumerate(self.test_cases):
            logger.info(f"Running test {i+1}/{len(self.test_cases)}: {test_case['name']}")
            
            # Generate code
            prompt = f"### Instruction:\n{test_case['prompt']}\n\n### Response:\n"
            
            try:
                outputs = generator(
                    prompt,
                    max_new_tokens=512,
                    temperature=0.7,
                    top_p=0.9,
                    do_sample=True,
                    num_return_sequences=1,
                    pad_token_id=tokenizer.eos_token_id
                )
                
                generated_text = outputs[0]['generated_text']
                # Extract just the response part
                response_start = generated_text.find("### Response:\n") + len("### Response:\n")
                generated_code = generated_text[response_start:].strip()
                
            except Exception as e:
                logger.error(f"Generation failed for {test_case['name']}: {e}")
                generated_code = ""
            
            # Evaluate syntax
            syntax_result = None
            if syntax_evaluator and generated_code:
                syntax_result = syntax_evaluator.evaluate_syntax(generated_code)
            
            # Check for expected keywords
            keyword_matches = 0
            for keyword in test_case['expected_keywords']:
                if keyword in generated_code:
                    keyword_matches += 1
            
            keyword_score = keyword_matches / len(test_case['expected_keywords']) if test_case['expected_keywords'] else 0
            
            # Record results
            test_result = {
                'name': test_case['name'],
                'category': test_case['category'],
                'prompt': test_case['prompt'],
                'generated_code': generated_code[:500],  # Truncate for storage
                'syntax_valid': syntax_result['valid'] if syntax_result else None,
                'syntax_error': syntax_result['error_message'] if syntax_result and not syntax_result['valid'] else None,
                'keyword_score': keyword_score,
                'expected_keywords': test_case['expected_keywords'],
                'keywords_found': [kw for kw in test_case['expected_keywords'] if kw in generated_code]
            }
            
            results['test_results'].append(test_result)
            
            # Update summary
            if syntax_result and syntax_result['valid']:
                results['summary']['syntax_valid'] += 1
            
            if keyword_score >= 0.5:  # At least 50% of keywords present
                results['summary']['keyword_matches'] += 1
            
            # Category tracking
            category = test_case['category']
            if category not in results['summary']['by_category']:
                results['summary']['by_category'][category] = {
                    'total': 0,
                    'syntax_valid': 0,
                    'keyword_matches': 0
                }
            
            results['summary']['by_category'][category]['total'] += 1
            if syntax_result and syntax_result['valid']:
                results['summary']['by_category'][category]['syntax_valid'] += 1
            if keyword_score >= 0.5:
                results['summary']['by_category'][category]['keyword_matches'] += 1
        
        # Calculate percentages
        total_tests = results['summary']['total_tests']
        results['summary']['syntax_success_rate'] = results['summary']['syntax_valid'] / total_tests if total_tests > 0 else 0
        results['summary']['keyword_success_rate'] = results['summary']['keyword_matches'] / total_tests if total_tests > 0 else 0
        
        for category_data in results['summary']['by_category'].values():
            category_total = category_data['total']
            category_data['syntax_success_rate'] = category_data['syntax_valid'] / category_total if category_total > 0 else 0
            category_data['keyword_success_rate'] = category_data['keyword_matches'] / category_total if category_total > 0 else 0
        
        return results

def generate_evaluation_report(results: Dict[str, Any], output_path: Path, logger: logging.Logger):
    """Generate comprehensive evaluation report."""
    logger.info(f"Generating evaluation report at {output_path}")
    
    # JSON report
    json_report = output_path / 'evaluation_results.json'
    with open(json_report, 'w') as f:
        json.dump(results, f, indent=2)
    
    # Human-readable report
    markdown_report = output_path / 'evaluation_report.md'
    
    with open(markdown_report, 'w') as f:
        f.write("# Asthra LLM Evaluation Report\n\n")
        f.write(f"**Generated:** {results['timestamp']}\n\n")
        
        # Summary
        summary = results['summary']
        f.write("## Summary\n\n")
        f.write(f"- **Total Tests:** {summary['total_tests']}\n")
        f.write(f"- **Syntax Success Rate:** {summary['syntax_success_rate']:.1%}\n")
        f.write(f"- **Keyword Success Rate:** {summary['keyword_success_rate']:.1%}\n\n")
        
        # By category
        f.write("## Results by Category\n\n")
        for category, data in summary['by_category'].items():
            f.write(f"### {category.title()}\n")
            f.write(f"- Tests: {data['total']}\n")
            f.write(f"- Syntax Success: {data['syntax_success_rate']:.1%}\n")
            f.write(f"- Keyword Success: {data['keyword_success_rate']:.1%}\n\n")
        
        # Individual test results
        f.write("## Individual Test Results\n\n")
        for test in results['test_results']:
            f.write(f"### {test['name']} ({test['category']})\n\n")
            f.write(f"**Prompt:** {test['prompt']}\n\n")
            f.write(f"**Syntax Valid:** {'✅' if test['syntax_valid'] else '❌'}\n")
            f.write(f"**Keyword Score:** {test['keyword_score']:.1%}\n")
            f.write(f"**Keywords Found:** {', '.join(test['keywords_found'])}\n\n")
            
            if test['syntax_error']:
                f.write(f"**Syntax Error:** `{test['syntax_error']}`\n\n")
            
            f.write("**Generated Code:**\n```asthra\n")
            f.write(test['generated_code'])
            f.write("\n```\n\n")
    
    logger.info(f"Reports saved:")
    logger.info(f"  - JSON: {json_report}")
    logger.info(f"  - Markdown: {markdown_report}")

def main():
    """Main evaluation function."""
    parser = argparse.ArgumentParser(description="Evaluate Asthra LLM model")
    parser.add_argument('--model-path', required=True, help='Path to fine-tuned model')
    parser.add_argument('--base-model', default='mistralai/Devstral-Small-2505', help='Base model name')
    parser.add_argument('--asthra-compiler', default='../build/asthra', help='Path to Asthra compiler')
    parser.add_argument('--output', default='./outputs/evaluation/', help='Output directory')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logging')
    
    args = parser.parse_args()
    
    # Set up logging
    logger = setup_logging(args.verbose)
    
    logger.info("🔍 Starting Asthra LLM Evaluation")
    logger.info("=" * 40)
    logger.info(f"Model path: {args.model_path}")
    logger.info(f"Base model: {args.base_model}")
    logger.info(f"Asthra compiler: {args.asthra_compiler}")
    logger.info(f"Output directory: {args.output}")
    
    try:
        # Create output directory
        output_path = Path(args.output)
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Load model and tokenizer
        model, tokenizer = load_model_and_tokenizer(args.model_path, args.base_model, logger)
        
        # Set up syntax evaluator
        syntax_evaluator = None
        if Path(args.asthra_compiler).exists():
            syntax_evaluator = AsthraSyntaxEvaluator(args.asthra_compiler)
            logger.info("Asthra compiler found - syntax evaluation enabled")
        else:
            logger.warning("Asthra compiler not found - syntax evaluation disabled")
        
        # Run benchmark suite
        benchmark = AsthraBenchmarkSuite()
        results = benchmark.run_benchmark(model, tokenizer, syntax_evaluator, logger)
        
        # Generate reports
        generate_evaluation_report(results, output_path, logger)
        
        # Print summary
        summary = results['summary']
        logger.info("\n📊 Evaluation Summary")
        logger.info("=" * 25)
        logger.info(f"Total tests: {summary['total_tests']}")
        logger.info(f"Syntax success rate: {summary['syntax_success_rate']:.1%}")
        logger.info(f"Keyword success rate: {summary['keyword_success_rate']:.1%}")
        
        # Category breakdown
        logger.info("\nBy category:")
        for category, data in summary['by_category'].items():
            logger.info(f"  {category}: {data['syntax_success_rate']:.1%} syntax, {data['keyword_success_rate']:.1%} keywords")
        
        logger.info(f"\n✅ Evaluation completed! Reports saved to {output_path}")
        
    except Exception as e:
        logger.error(f"❌ Evaluation failed: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main() 
