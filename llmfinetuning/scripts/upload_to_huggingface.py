#!/usr/bin/env python3
"""
Upload Asthra LoRA adapter to HuggingFace Hub
"""

import argparse
import os
import sys
from pathlib import Path

try:
    from huggingface_hub import HfApi, Repository, login, create_repo
    from huggingface_hub.utils import RepositoryNotFoundError
    print("✅ HuggingFace Hub library available")
except ImportError:
    print("❌ Please install huggingface_hub: pip install huggingface_hub")
    sys.exit(1)

def upload_lora_adapter(
    model_path: str,
    repo_name: str,
    private: bool = False,
    commit_message: str = "Upload Asthra LoRA adapter"
):
    """Upload LoRA adapter to HuggingFace Hub."""
    
    print(f"🚀 Uploading Asthra LoRA adapter to HuggingFace Hub")
    print("=" * 60)
    print(f"📁 Model Path: {model_path}")
    print(f"📦 Repository: {repo_name}")
    print(f"🔒 Private: {private}")
    
    # Validate model path
    if not os.path.exists(model_path):
        print(f"❌ Model path not found: {model_path}")
        return False
    
    # Check for required LoRA files
    required_files = ['adapter_model.safetensors', 'adapter_config.json']
    missing_files = []
    
    for file in required_files:
        if not os.path.exists(os.path.join(model_path, file)):
            missing_files.append(file)
    
    if missing_files:
        print(f"❌ Missing required LoRA files: {missing_files}")
        return False
    
    print("✅ LoRA adapter files found")
    
    try:
        # Initialize HF API
        api = HfApi()
        
        # Check if user is logged in
        try:
            user = api.whoami()
            username = user['name']
            print(f"✅ Logged in as: {username}")
        except Exception:
            print("❌ Not logged in to HuggingFace")
            print("Please run: huggingface-cli login")
            return False
        
        # Create repository name with username
        full_repo_name = f"{username}/{repo_name}"
        
        # Create repository
        try:
            repo_url = create_repo(
                repo_id=full_repo_name,
                private=private,
                repo_type="model"
            )
            print(f"✅ Created repository: {repo_url}")
        except Exception as e:
            if "already exists" in str(e):
                print(f"ℹ️  Repository already exists: {full_repo_name}")
            else:
                print(f"❌ Failed to create repository: {e}")
                return False
        
        # Create README.md for the model
        readme_content = create_model_readme(model_path)
        readme_path = os.path.join(model_path, "README.md")
        with open(readme_path, 'w') as f:
            f.write(readme_content)
        
        # Upload files
        print("📤 Uploading files...")
        
        files_to_upload = [
            'adapter_model.safetensors',
            'adapter_config.json',
            'README.md'
        ]
        
        # Check for optional files
        optional_files = ['training_args.json', 'trainer_state.json']
        for file in optional_files:
            if os.path.exists(os.path.join(model_path, file)):
                files_to_upload.append(file)
        
        for file in files_to_upload:
            file_path = os.path.join(model_path, file)
            if os.path.exists(file_path):
                print(f"  📁 Uploading {file}...")
                api.upload_file(
                    path_or_fileobj=file_path,
                    path_in_repo=file,
                    repo_id=full_repo_name,
                    commit_message=f"{commit_message} - {file}"
                )
        
        print("✅ Upload completed successfully!")
        print(f"🌐 Model available at: https://huggingface.co/{full_repo_name}")
        
        # Clean up temporary README
        if os.path.exists(readme_path):
            os.remove(readme_path)
        
        return True
        
    except Exception as e:
        print(f"❌ Upload failed: {e}")
        return False

def create_model_readme(model_path: str) -> str:
    """Create a README.md for the model repository."""
    
    # Read adapter config to get base model info
    import json
    config_path = os.path.join(model_path, 'adapter_config.json')
    
    try:
        with open(config_path, 'r') as f:
            config = json.load(f)
        base_model = config.get('base_model_name_or_path', 'google/gemma-3-4b-it')
        lora_rank = config.get('r', 'Unknown')
        lora_alpha = config.get('lora_alpha', 'Unknown')
        target_modules = config.get('target_modules', [])
    except:
        base_model = 'google/gemma-3-4b-it'
        lora_rank = 'Unknown'
        lora_alpha = 'Unknown'
        target_modules = []
    
    readme = f"""---
license: apache-2.0
base_model: {base_model}
tags:
- asthra
- programming-language
- code-generation
- lora
- peft
language:
- en
library_name: peft
---

# Asthra Programming Language LoRA Adapter

This is a LoRA (Low-Rank Adaptation) adapter trained on the Asthra programming language for code generation.

## Model Details

- **Base Model**: `{base_model}`
- **Adapter Type**: LoRA (Low-Rank Adaptation)
- **LoRA Rank**: {lora_rank}
- **LoRA Alpha**: {lora_alpha}
- **Target Modules**: {', '.join(target_modules) if target_modules else 'Various attention and MLP layers'}
- **Language**: Asthra programming language
- **License**: Apache 2.0

## About Asthra

Asthra is a systems programming language designed for:
- **Clarity**: Explicit syntax with clear visibility modifiers
- **Safety**: Immutable by default, Result/Option types for error handling
- **Concurrency**: Built-in support for structured concurrency
- **AI-Friendly**: Designed for easy code generation and analysis

## Usage

### Loading the Model

```python
from transformers import AutoModelForCausalLM, AutoTokenizer
from peft import PeftModel

# Load base model
base_model = AutoModelForCausalLM.from_pretrained("{base_model}")
tokenizer = AutoTokenizer.from_pretrained("{base_model}")

# Load LoRA adapter
model = PeftModel.from_pretrained(base_model, "YOUR_USERNAME/asthra-gemma3-lora")

# Generate Asthra code
prompt = "Write a factorial function in Asthra:"
inputs = tokenizer(prompt, return_tensors="pt")
outputs = model.generate(**inputs, max_length=200, temperature=0.3)
generated_text = tokenizer.decode(outputs[0], skip_special_tokens=True)
print(generated_text)
```

### With Pipeline

```python
from transformers import pipeline
from peft import PeftModel, PeftConfig

# Load config and model
config = PeftConfig.from_pretrained("YOUR_USERNAME/asthra-gemma3-lora")
model = AutoModelForCausalLM.from_pretrained(config.base_model_name_or_path)
model = PeftModel.from_pretrained(model, "YOUR_USERNAME/asthra-gemma3-lora")

# Create pipeline
generator = pipeline("text-generation", model=model, tokenizer=tokenizer)

# Generate code
result = generator("Write a concurrent web server in Asthra:", 
                  max_length=300, 
                  temperature=0.3)
print(result[0]['generated_text'])
```

## Example Outputs

The model generates idiomatic Asthra code with proper:
- Function syntax: `fn name(params) -> ReturnType`
- Error handling: `Result.Ok()` and `Result.Err()`
- Type annotations: explicit types for variables and parameters
- Concurrency: channels and spawn for concurrent programming
- Memory safety: automatic memory management

## Training Details

This LoRA adapter was fine-tuned specifically on Asthra programming language examples to improve code generation quality and adherence to Asthra language patterns.

## License

This model is released under the Apache 2.0 license.
"""
    
    return readme

def main():
    """Main function to upload LoRA adapter."""
    parser = argparse.ArgumentParser(description="Upload Asthra LoRA adapter to HuggingFace Hub")
    parser.add_argument('--model-path', required=True, help='Path to the LoRA adapter directory')
    parser.add_argument('--repo-name', default='asthra-gemma3-lora', help='Repository name on HuggingFace')
    parser.add_argument('--private', action='store_true', help='Make repository private')
    parser.add_argument('--commit-message', default='Upload Asthra LoRA adapter', help='Commit message')
    
    args = parser.parse_args()
    
    success = upload_lora_adapter(
        model_path=args.model_path,
        repo_name=args.repo_name,
        private=args.private,
        commit_message=args.commit_message
    )
    
    if success:
        print("\n🎉 SUCCESS! Your Asthra LoRA adapter is now available on HuggingFace Hub!")
        print(f"📚 Share it with: https://huggingface.co/YOUR_USERNAME/{args.repo_name}")
    else:
        print("\n❌ Upload failed. Please check the error messages above.")
        sys.exit(1)

if __name__ == '__main__':
    main() 
