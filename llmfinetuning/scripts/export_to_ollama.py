#!/usr/bin/env python3
"""
Export Asthra LoRA Model to Ollama GGUF Format
This script merges the LoRA adapter with the base model and converts to GGUF for Ollama
"""

import argparse
import os
import sys
import tempfile
import shutil
from pathlib import Path
import logging

# Add project root to path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

try:
    import torch
    from transformers import AutoModelForCausalLM, AutoTokenizer
    from peft import PeftModel, PeftConfig
    logger.info("✅ Core packages imported successfully")
except ImportError as e:
    logger.error(f"❌ Missing required packages: {e}")
    logger.error("Install with: pip install transformers torch peft")
    sys.exit(1)

def check_llama_cpp():
    """Check if llama.cpp is available for GGUF conversion."""
    try:
        import subprocess
        result = subprocess.run(['python', '-c', 'import llama_cpp'], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            return True
    except:
        pass
    
    # Check for llama.cpp convert script
    possible_paths = [
        "llama.cpp/convert_hf_to_gguf.py",
        "../llama.cpp/convert_hf_to_gguf.py",
        "convert_hf_to_gguf.py",
        os.path.expanduser("~/llama.cpp/convert_hf_to_gguf.py")
    ]
    
    for path in possible_paths:
        if os.path.exists(path):
            return path
    
    return False

def merge_lora_model(model_path: str, output_path: str):
    """Merge LoRA adapter with base model to create a full model."""
    logger.info(f"🔄 Merging LoRA model from: {model_path}")
    
    # Check if this is a PEFT adapter
    adapter_config_path = os.path.join(model_path, "adapter_config.json")
    if not os.path.exists(adapter_config_path):
        logger.error(f"❌ Not a PEFT adapter model: {model_path}")
        return False
    
    try:
        # Load PEFT config
        peft_config = PeftConfig.from_pretrained(model_path)
        base_model_name = peft_config.base_model_name_or_path
        logger.info(f"📦 Base model: {base_model_name}")
        
        # Load tokenizer
        logger.info("🔤 Loading tokenizer...")
        tokenizer = AutoTokenizer.from_pretrained(model_path, trust_remote_code=True)
        
        # Load base model
        logger.info(f"📦 Loading base model: {base_model_name}")
        base_model = AutoModelForCausalLM.from_pretrained(
            base_model_name,
            torch_dtype=torch.float16,
            device_map="auto",
            trust_remote_code=True
        )
        
        # Load and merge LoRA adapter
        logger.info("🔧 Loading LoRA adapter...")
        model = PeftModel.from_pretrained(base_model, model_path)
        
        logger.info("🔀 Merging LoRA with base model...")
        merged_model = model.merge_and_unload()
        
        # Save merged model
        logger.info(f"💾 Saving merged model to: {output_path}")
        os.makedirs(output_path, exist_ok=True)
        
        merged_model.save_pretrained(output_path, torch_dtype=torch.float16)
        tokenizer.save_pretrained(output_path)
        
        logger.info("✅ Model merge completed successfully!")
        return True
        
    except Exception as e:
        logger.error(f"❌ Failed to merge model: {e}")
        return False

def convert_to_gguf(model_path: str, output_path: str, quantization: str = "Q4_K_M"):
    """Convert HuggingFace model to GGUF format."""
    logger.info(f"🔄 Converting to GGUF format with {quantization} quantization")
    
    # Check for conversion tools
    converter = check_llama_cpp()
    if not converter:
        logger.error("❌ llama.cpp conversion tools not found!")
        logger.error("Please install llama.cpp:")
        logger.error("1. git clone https://github.com/ggerganov/llama.cpp")
        logger.error("2. cd llama.cpp && make")
        logger.error("3. pip install -r requirements.txt")
        return False
    
    try:
        import subprocess
        
        # Create output directory
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        
        if isinstance(converter, str):
            # Use llama.cpp convert script
            cmd = [
                'python', converter,
                model_path,
                '--outtype', quantization,
                '--outfile', output_path
            ]
        else:
            # Try using transformers-to-gguf (alternative method)
            cmd = [
                'python', '-m', 'transformers_to_gguf.convert',
                '--model', model_path,
                '--output', output_path,
                '--quantization', quantization
            ]
        
        logger.info(f"🔧 Running conversion command: {' '.join(cmd)}")
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=project_root)
        
        if result.returncode == 0:
            logger.info("✅ GGUF conversion completed successfully!")
            return True
        else:
            logger.error(f"❌ GGUF conversion failed: {result.stderr}")
            return False
            
    except Exception as e:
        logger.error(f"❌ Failed to convert to GGUF: {e}")
        return False

def create_ollama_modelfile(gguf_path: str, output_dir: str, model_name: str = "asthra-gemma3-4b"):
    """Create Ollama Modelfile for the converted model."""
    modelfile_path = os.path.join(output_dir, "Modelfile")
    
    modelfile_content = f"""FROM {gguf_path}

# Model parameters
PARAMETER temperature 0.7
PARAMETER top_p 0.9
PARAMETER top_k 40
PARAMETER repeat_penalty 1.1
PARAMETER num_ctx 4096

# System prompt for Asthra programming
SYSTEM \"\"\"You are an expert Asthra programmer. Asthra is a modern systems programming language designed for AI code generation excellence. 

Key Asthra features:
- Strong static typing with type inference
- Memory safety without garbage collection
- Concurrency with channels and async/await
- Pattern matching and algebraic data types
- Zero-cost abstractions
- Interoperability with C/C++

When writing Asthra code:
- Use `fn` for function declarations
- Use `let` and `mut` for variable declarations
- Use `->` for function return types
- Use `Result<T, E>` for error handling
- Use `Option<T>` for nullable values
- Follow Asthra naming conventions (snake_case)

Generate clean, idiomatic, and well-documented Asthra code.\"\"\"

# Template for code generation
TEMPLATE \"\"\"{{ if .System }}{{ .System }}

{{ end }}{{ if .Prompt }}User: {{ .Prompt }}
{{ end }}Assistant: \"\"\"
"""
    
    logger.info(f"📝 Creating Ollama Modelfile: {modelfile_path}")
    with open(modelfile_path, 'w') as f:
        f.write(modelfile_content)
    
    logger.info("✅ Ollama Modelfile created successfully!")
    return modelfile_path

def install_ollama_model(modelfile_path: str, model_name: str):
    """Install the model in Ollama."""
    try:
        import subprocess
        
        # Check if Ollama is installed
        result = subprocess.run(['ollama', 'version'], capture_output=True, text=True)
        if result.returncode != 0:
            logger.error("❌ Ollama not found! Please install Ollama first:")
            logger.error("Visit: https://ollama.ai/download")
            return False
        
        logger.info(f"🔧 Installing model in Ollama as '{model_name}'...")
        
        # Create the model in Ollama
        cmd = ['ollama', 'create', model_name, '-f', modelfile_path]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            logger.info("✅ Model installed in Ollama successfully!")
            logger.info(f"🚀 You can now use: ollama run {model_name}")
            return True
        else:
            logger.error(f"❌ Failed to install in Ollama: {result.stderr}")
            return False
            
    except Exception as e:
        logger.error(f"❌ Failed to install in Ollama: {e}")
        return False

def main():
    """Main function to export Asthra model to Ollama."""
    parser = argparse.ArgumentParser(description="Export Asthra LoRA Model to Ollama GGUF Format")
    parser.add_argument('--model-path', required=True, help='Path to the LoRA model directory')
    parser.add_argument('--output-dir', default='./outputs/ollama/', help='Output directory for Ollama files')
    parser.add_argument('--model-name', default='asthra-gemma3-4b', help='Name for the Ollama model')
    parser.add_argument('--quantization', default='Q4_K_M', 
                       choices=['Q4_0', 'Q4_1', 'Q5_0', 'Q5_1', 'Q8_0', 'Q4_K_M', 'Q5_K_M'],
                       help='GGUF quantization type')
    parser.add_argument('--merge-only', action='store_true', help='Only merge LoRA, skip GGUF conversion')
    parser.add_argument('--skip-install', action='store_true', help='Skip Ollama installation step')
    
    args = parser.parse_args()
    
    logger.info("🚀 Starting Asthra Model Export to Ollama")
    logger.info("=" * 50)
    logger.info(f"Model Path: {args.model_path}")
    logger.info(f"Output Dir: {args.output_dir}")
    logger.info(f"Model Name: {args.model_name}")
    logger.info(f"Quantization: {args.quantization}")
    
    # Validate input
    if not os.path.exists(args.model_path):
        logger.error(f"❌ Model path not found: {args.model_path}")
        sys.exit(1)
    
    # Create output directories
    os.makedirs(args.output_dir, exist_ok=True)
    merged_model_path = os.path.join(args.output_dir, "merged_model")
    gguf_path = os.path.join(args.output_dir, f"{args.model_name}.gguf")
    
    # Step 1: Merge LoRA with base model
    logger.info("📝 Step 1: Merging LoRA adapter with base model...")
    if not merge_lora_model(args.model_path, merged_model_path):
        logger.error("❌ Failed to merge LoRA model")
        sys.exit(1)
    
    if args.merge_only:
        logger.info(f"✅ Merge completed! Merged model saved to: {merged_model_path}")
        return
    
    # Step 2: Convert to GGUF
    logger.info("📝 Step 2: Converting to GGUF format...")
    if not convert_to_gguf(merged_model_path, gguf_path, args.quantization):
        logger.error("❌ Failed to convert to GGUF")
        logger.error("You can use the merged model directly or install llama.cpp for GGUF conversion")
        sys.exit(1)
    
    # Step 3: Create Ollama Modelfile
    logger.info("📝 Step 3: Creating Ollama Modelfile...")
    modelfile_path = create_ollama_modelfile(gguf_path, args.output_dir, args.model_name)
    
    # Step 4: Install in Ollama (optional)
    if not args.skip_install:
        logger.info("📝 Step 4: Installing in Ollama...")
        if install_ollama_model(modelfile_path, args.model_name):
            logger.info("\n🎉 SUCCESS! Your Asthra model is ready in Ollama!")
            logger.info(f"🚀 Usage: ollama run {args.model_name}")
            logger.info(f"🚀 Chat: ollama run {args.model_name} 'Write a factorial function in Asthra'")
        else:
            logger.info(f"⚠️  Manual installation required:")
            logger.info(f"   cd {args.output_dir}")
            logger.info(f"   ollama create {args.model_name} -f Modelfile")
    else:
        logger.info("✅ Export completed! Files ready for manual Ollama installation:")
        logger.info(f"📁 GGUF Model: {gguf_path}")
        logger.info(f"📁 Modelfile: {modelfile_path}")
        logger.info(f"🚀 Install with: cd {args.output_dir} && ollama create {args.model_name} -f Modelfile")
    
    # Cleanup merged model to save space
    if os.path.exists(merged_model_path):
        logger.info("🧹 Cleaning up temporary merged model...")
        shutil.rmtree(merged_model_path)
    
    logger.info("\n✅ Asthra model export completed successfully!")

if __name__ == '__main__':
    main()
