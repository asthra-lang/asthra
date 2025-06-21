#!/usr/bin/env python3
"""
Test script for the fine-tuned Asthra LLM model.
"""

from transformers import AutoTokenizer, AutoModelForCausalLM
from peft import PeftModel
import torch

def test_model():
    print("Loading trained model...")
    base_model = AutoModelForCausalLM.from_pretrained('microsoft/DialoGPT-small')
    model = PeftModel.from_pretrained(base_model, 'outputs/models/debug/final')
    tokenizer = AutoTokenizer.from_pretrained('microsoft/DialoGPT-small')
    
    # Set pad token
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token

    print("Testing generation...")
    prompt = "Asthra programming language"
    inputs = tokenizer.encode(prompt, return_tensors='pt')
    
    with torch.no_grad():
        outputs = model.generate(
            inputs, 
            max_length=50, 
            num_return_sequences=1, 
            temperature=0.7, 
            do_sample=True, 
            pad_token_id=tokenizer.eos_token_id
        )
    
    generated_text = tokenizer.decode(outputs[0], skip_special_tokens=True)
    print(f"Input: {prompt}")
    print(f"Generated: {generated_text}")
    print("✅ Model test successful!")

if __name__ == "__main__":
    test_model() 
