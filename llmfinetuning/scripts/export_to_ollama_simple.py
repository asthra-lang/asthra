#!/usr/bin/env python3
"""
Simplified Asthra LoRA Model to Ollama Export
This script merges the LoRA adapter with the base model and creates an Ollama Modelfile
"""

import argparse
import os
import sys
import shutil
from pathlib import Path
import logging

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
    logger.error(f"❌ Import error: {e}")
    logger.error("Install required packages: pip install torch transformers peft")
    sys.exit(1)

def merge_lora_model(lora_path: str, output_path: str) -> bool:
    """Merge LoRA adapter with base model."""
    try:
        logger.info(f"🔄 Loading LoRA config from: {lora_path}")
        
        # Load PEFT config to get base model
        peft_config = PeftConfig.from_pretrained(lora_path)
        base_model_name = peft_config.base_model_name_or_path
        
        logger.info(f"📦 Base model: {base_model_name}")
        logger.info(f"🔄 Loading base model...")
        
        # Load base model and tokenizer
        tokenizer = AutoTokenizer.from_pretrained(base_model_name)
        base_model = AutoModelForCausalLM.from_pretrained(
            base_model_name,
            torch_dtype=torch.float16,
            device_map="auto"
        )
        
        logger.info(f"🔄 Loading LoRA adapter...")
        
        # Load and merge LoRA adapter
        model = PeftModel.from_pretrained(base_model, lora_path)
        merged_model = model.merge_and_unload()
        
        logger.info(f"💾 Saving merged model to: {output_path}")
        
        # Save merged model - handle shared tensors properly
        os.makedirs(output_path, exist_ok=True)
        
        # Try to save with safe_serialization=False to avoid the shared tensor error
        try:
            merged_model.save_pretrained(output_path, safe_serialization=False)
        except Exception as e:
            logger.warning(f"Failed with safe_serialization=False: {e}")
            # Alternative: Try using torch.save directly for model weights
            try:
                import json
                # Save the model configuration
                config = merged_model.config
                config.save_pretrained(output_path)
                
                # Save the model weights using torch.save
                torch.save(merged_model.state_dict(), os.path.join(output_path, "pytorch_model.bin"))
                logger.info("✅ Saved model using torch.save fallback method")
            except Exception as e2:
                logger.error(f"Both save methods failed: {e2}")
                raise e2
        
        tokenizer.save_pretrained(output_path)
        
        logger.info("✅ Model merge completed successfully!")
        return True
        
    except Exception as e:
        logger.error(f"❌ Failed to merge LoRA model: {e}")
        return False

def create_ollama_modelfile(model_path: str, output_dir: str, model_name: str) -> str:
    """Create Ollama Modelfile for the merged model."""
    modelfile_path = os.path.join(output_dir, "Modelfile")
    
    # Note: Since we don't have GGUF, we'll create a Modelfile that can be manually converted
    modelfile_content = f"""# Asthra Programming Language Model
# This Modelfile is for the merged Asthra model
# 
# IMPORTANT: This model is in HuggingFace format, not GGUF
# You'll need to convert it to GGUF format before using with Ollama
# 
# Steps to use:
# 1. Convert to GGUF using llama.cpp tools
# 2. Update the FROM directive below to point to the GGUF file
# 3. Run: ollama create {model_name} -f Modelfile

# FROM {model_path}  # Update this path to point to your GGUF file
FROM ./asthra-gemma3-4b.gguf

# Set model parameters
PARAMETER temperature 0.3
PARAMETER top_p 0.9
PARAMETER top_k 40
PARAMETER num_ctx 4096

# System prompt optimized for Asthra programming
SYSTEM \"\"\"You are an expert Asthra programmer and AI assistant. Asthra is a systems programming language designed for clarity, safety, and AI-friendly code generation.

Key Asthra features you should use:
- Explicit syntax: clear visibility (pub/priv), explicit parameters (void), explicit returns
- Immutable by default: variables are immutable unless marked with 'mut'
- Result types: use Result.Ok() and Result.Err() for error handling
- Option types: use Option.Some() and Option.None() for nullable values
- Concurrency: use spawn for tasks, channels for communication
- Memory safety: automatic memory management with clear ownership
- FFI: seamless C interoperability

When generating Asthra code:
1. Use proper syntax: fn name(params) -> ReturnType
2. Handle errors with Result types
3. Use explicit type annotations
4. Follow immutable-by-default patterns
5. Include proper visibility modifiers (pub/priv)
6. Use structured concurrency when appropriate

Always generate idiomatic, safe, and performant Asthra code.\"\"\"

# Template for code generation
TEMPLATE \"\"\"{{ if .System }}{{ .System }}

{{ end }}{{ if .Prompt }}User: {{ .Prompt }}

{{ end }}Assistant: \"\"\"
"""
    
    logger.info(f"📝 Creating Ollama Modelfile: {modelfile_path}")
    with open(modelfile_path, 'w') as f:
        f.write(modelfile_content)
    
    # Also create a README with conversion instructions
    readme_path = os.path.join(output_dir, "README_CONVERSION.md")
    readme_content = f"""# Asthra Model Conversion Guide

Your Asthra model has been successfully merged and is ready for conversion to Ollama.

## Current Status
✅ LoRA adapter merged with base model  
📁 Merged model saved to: `{model_path}`  
📝 Ollama Modelfile created: `Modelfile`  

## Next Steps: Convert to GGUF Format

Since your system doesn't have llama-cpp-python installed, you'll need to manually convert the model to GGUF format:

### Option 1: Use llama.cpp (Recommended)

```bash
# Clone llama.cpp
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp
make

# Convert to GGUF
python convert-hf-to-gguf.py {model_path} --outfile {model_name}.gguf --outtype q4_0
```

### Option 2: Use Online Converters
- Some services offer HuggingFace to GGUF conversion
- Upload your merged model to HuggingFace Hub
- Use conversion services (search for "HuggingFace to GGUF converter")

### Option 3: Use Pre-built Tools
```bash
# If you have access to another system with llama-cpp-python
pip install llama-cpp-python
python -c "from llama_cpp import llama_cpp; llama_cpp.llama_convert_hf_to_gguf('{model_path}', '{model_name}.gguf')"
```

## Install in Ollama

Once you have the GGUF file:

```bash
# Update the Modelfile FROM directive to point to your GGUF file
# Then create the model in Ollama:
ollama create {model_name} -f Modelfile

# Test your model
ollama run {model_name} "Write a factorial function in Asthra"
```

## Model Information
- **Base Model**: google/gemma-3-4b-it
- **Adapter**: Asthra LoRA fine-tuning  
- **Model Size**: ~4.3B parameters
- **Recommended Quantization**: Q4_K_M (good balance of size and quality)

Your Asthra model is now ready for high-performance local inference with Ollama! 🚀
"""
    
    with open(readme_path, 'w') as f:
        f.write(readme_content)
    
    logger.info("✅ Ollama Modelfile and conversion guide created!")
    return modelfile_path

def test_merged_model(model_path: str) -> bool:
    """Test the merged model with a simple generation."""
    try:
        logger.info("🧪 Testing merged model...")
        
        from transformers import pipeline
        
        # Create a text generation pipeline
        generator = pipeline(
            "text-generation",
            model=model_path,
            tokenizer=model_path,
            torch_dtype=torch.float16,
            device_map="auto"
        )
        
        # Test prompt
        prompt = "Write a simple Asthra function:"
        
        # Generate
        output = generator(
            prompt,
            max_length=100,
            num_return_sequences=1,
            temperature=0.3,
            do_sample=True,
            pad_token_id=generator.tokenizer.eos_token_id
        )
        
        generated_text = output[0]['generated_text']
        logger.info(f"✅ Model test successful! Generated: {generated_text[:100]}...")
        return True
        
    except Exception as e:
        logger.error(f"⚠️  Model test failed: {e}")
        logger.info("Don't worry - this doesn't mean the merge failed, just that testing is difficult")
        return False

def main():
    """Main function to export Asthra model to Ollama."""
    parser = argparse.ArgumentParser(description="Export Asthra LoRA Model to Ollama (Simplified)")
    parser.add_argument('--model-path', required=True, help='Path to the LoRA model directory')
    parser.add_argument('--output-dir', default='./outputs/ollama/', help='Output directory for Ollama files')
    parser.add_argument('--model-name', default='asthra-gemma3-4b', help='Name for the Ollama model')
    parser.add_argument('--test-model', action='store_true', help='Test the merged model')
    
    args = parser.parse_args()
    
    logger.info("🚀 Starting Asthra Model Export to Ollama (Simplified)")
    logger.info("=" * 60)
    logger.info(f"Model Path: {args.model_path}")
    logger.info(f"Output Dir: {args.output_dir}")
    logger.info(f"Model Name: {args.model_name}")
    
    # Validate input
    if not os.path.exists(args.model_path):
        logger.error(f"❌ Model path not found: {args.model_path}")
        sys.exit(1)
    
    # Create output directories
    os.makedirs(args.output_dir, exist_ok=True)
    merged_model_path = os.path.join(args.output_dir, "merged_model")
    
    # Step 1: Merge LoRA with base model
    logger.info("📝 Step 1: Merging LoRA adapter with base model...")
    if not merge_lora_model(args.model_path, merged_model_path):
        logger.error("❌ Failed to merge LoRA model")
        sys.exit(1)
    
    # Step 2: Test model (optional)
    if args.test_model:
        logger.info("📝 Step 2: Testing merged model...")
        test_merged_model(merged_model_path)
    
    # Step 3: Create Ollama Modelfile
    logger.info("📝 Step 3: Creating Ollama Modelfile...")
    modelfile_path = create_ollama_modelfile(merged_model_path, args.output_dir, args.model_name)
    
    # Success summary
    logger.info("\n🎉 SUCCESS! Your Asthra model export is ready!")
    logger.info("=" * 60)
    logger.info(f"📁 Merged Model: {merged_model_path}")
    logger.info(f"📝 Modelfile: {modelfile_path}")
    logger.info(f"📖 Instructions: {os.path.join(args.output_dir, 'README_CONVERSION.md')}")
    logger.info("")
    logger.info("⚠️  NEXT STEPS:")
    logger.info("1. Convert the merged model to GGUF format (see README_CONVERSION.md)")
    logger.info("2. Update the Modelfile FROM directive")
    logger.info(f"3. Run: ollama create {args.model_name} -f Modelfile")
    logger.info(f"4. Test: ollama run {args.model_name}")
    logger.info("")
    logger.info("🚀 Your Asthra AI assistant will be ready for local use!")

if __name__ == '__main__':
    main() 
