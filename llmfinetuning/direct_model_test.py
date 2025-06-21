#!/usr/bin/env python3
"""
Direct test of the trained Asthra model without server
"""

import os
import sys
import torch
from pathlib import Path

# Add project root to path
project_root = Path(__file__).parent
sys.path.insert(0, str(project_root))

try:
    from transformers import AutoModelForCausalLM, AutoTokenizer
    from peft import PeftModel, PeftConfig
    print("✅ All required packages imported successfully")
except ImportError as e:
    print(f"❌ Import error: {e}")
    sys.exit(1)

def load_asthra_model():
    """Load the Asthra LoRA model."""
    model_path = "./outputs/asthra-gemma3-4b-v1"
    
    print(f"🔍 Loading Asthra model from: {model_path}")
    
    # Load PEFT config
    print("📦 Loading PEFT config...")
    peft_config = PeftConfig.from_pretrained(model_path)
    base_model_name = peft_config.base_model_name_or_path
    print(f"📋 Base model: {base_model_name}")
    
    # Load tokenizer
    print("🔤 Loading tokenizer...")
    tokenizer = AutoTokenizer.from_pretrained(
        model_path,
        trust_remote_code=True
    )
    
    # Check device
    device = torch.device("cuda" if torch.cuda.is_available() else "mps" if torch.backends.mps.is_available() else "cpu")
    print(f"🖥️  Using device: {device}")
    
    # Load base model
    print(f"📦 Loading base model: {base_model_name}...")
    
    model_kwargs = {
        "trust_remote_code": True,
        "torch_dtype": torch.float16 if device.type == "cuda" else torch.float32,
        "device_map": "auto" if device.type == "cuda" else None,
        "low_cpu_mem_usage": True
    }
    
    base_model = AutoModelForCausalLM.from_pretrained(
        base_model_name,
        **model_kwargs
    )
    
    # Load adapter
    print("🔧 Loading LoRA adapter...")
    model = PeftModel.from_pretrained(base_model, model_path)
    
    if device.type != "cuda":
        model = model.to(device)
    
    model.eval()
    
    print("✅ Asthra model loaded successfully!")
    return model, tokenizer, device

def generate_asthra_code(model, tokenizer, device, prompt, max_tokens=200):
    """Generate Asthra code from a prompt."""
    print(f"💭 Prompt: {prompt}")
    
    # Tokenize input
    inputs = tokenizer(prompt, return_tensors="pt").to(device)
    
    # Generate
    with torch.no_grad():
        outputs = model.generate(
            **inputs,
            max_new_tokens=max_tokens,
            temperature=0.7,
            do_sample=True,
            pad_token_id=tokenizer.pad_token_id,
            eos_token_id=tokenizer.eos_token_id,
            repetition_penalty=1.1
        )
    
    # Decode
    generated_text = tokenizer.decode(outputs[0], skip_special_tokens=True)
    
    # Extract only the new text (remove the prompt)
    new_text = generated_text[len(prompt):]
    
    return new_text.strip()

def main():
    """Main test function."""
    print("🚀 Direct Asthra Model Test")
    print("=" * 50)
    
    # Load model
    model, tokenizer, device = load_asthra_model()
    
    # Test prompts
    test_prompts = [
        "Write a function in Asthra that calculates factorial:",
        "Create an Asthra program that implements a binary search:",
        "Write Asthra code for a simple linked list:",
        "Implement a concurrent Asthra program with channels:",
        "Create an Asthra function that sorts an array:"
    ]
    
    print("\n🧪 Running generation tests...\n")
    
    for i, prompt in enumerate(test_prompts, 1):
        print(f"📝 Test {i}/{len(test_prompts)}")
        print(f"❓ {prompt}")
        
        try:
            result = generate_asthra_code(model, tokenizer, device, prompt, max_tokens=150)
            print(f"✅ Generated Asthra code:")
            print(f"```asthra")
            print(result)
            print(f"```")
        except Exception as e:
            print(f"❌ Error: {e}")
        
        print("-" * 50)
    
    print("\n🎉 Direct model test completed!")

if __name__ == "__main__":
    main() 
