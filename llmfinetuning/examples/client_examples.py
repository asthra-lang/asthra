#!/usr/bin/env python3
"""
Client Examples for Asthra OpenAI-Compatible API
Demonstrates various ways to interact with the fine-tuned Asthra model
"""

import asyncio
import json
import os
import sys
import time
from typing import List, Dict, Any

# Example using OpenAI Python client
try:
    from openai import OpenAI
    HAS_OPENAI = True
except ImportError:
    print("⚠️  OpenAI client not available. Install with: pip install openai")
    HAS_OPENAI = False

# Example using requests
try:
    import requests
    HAS_REQUESTS = True
except ImportError:
    print("⚠️  Requests not available. Install with: pip install requests")
    HAS_REQUESTS = False

# Example using httpx for async
try:
    import httpx
    HAS_HTTPX = True
except ImportError:
    print("⚠️  httpx not available. Install with: pip install httpx")
    HAS_HTTPX = False

class AsthraiClient:
    """Custom client for Asthra API with Asthra-specific helpers."""
    
    def __init__(self, base_url: str = "http://localhost:8008"):
        self.base_url = base_url.rstrip('/')
        self.session = requests.Session() if HAS_REQUESTS else None
    
    def generate_code(self, prompt: str, **kwargs) -> str:
        """Generate Asthra code from a prompt."""
        messages = [
            {"role": "system", "content": "You are an expert Asthra programmer. Generate clean, idiomatic Asthra code."},
            {"role": "user", "content": prompt}
        ]
        
        response = self.chat_completion(messages, **kwargs)
        return response['choices'][0]['message']['content']
    
    def explain_code(self, code: str, **kwargs) -> str:
        """Explain Asthra code."""
        messages = [
            {"role": "system", "content": "You are an expert Asthra programmer. Explain the given code clearly."},
            {"role": "user", "content": f"Explain this Asthra code:\n\n```asthra\n{code}\n```"}
        ]
        
        response = self.chat_completion(messages, **kwargs)
        return response['choices'][0]['message']['content']
    
    def debug_code(self, code: str, error: str, **kwargs) -> str:
        """Help debug Asthra code."""
        messages = [
            {"role": "system", "content": "You are an expert Asthra programmer. Help debug code issues."},
            {"role": "user", "content": f"I have this Asthra code:\n\n```asthra\n{code}\n```\n\nAnd I'm getting this error:\n{error}\n\nHow can I fix it?"}
        ]
        
        response = self.chat_completion(messages, **kwargs)
        return response['choices'][0]['message']['content']
    
    def complete_code(self, partial_code: str, **kwargs) -> str:
        """Complete partial Asthra code."""
        response = self.text_completion(partial_code, **kwargs)
        return response['choices'][0]['text']
    
    def chat_completion(self, messages: List[Dict], **kwargs) -> Dict:
        """Send chat completion request."""
        if not self.session:
            raise RuntimeError("Requests not available")
        
        payload = {
            "model": "asthra-gemma3-4b",
            "messages": messages,
            "max_tokens": kwargs.get('max_tokens', 2048),
            "temperature": kwargs.get('temperature', 0.7),
            "top_p": kwargs.get('top_p', 0.9),
            "stream": kwargs.get('stream', False)
        }
        
        response = self.session.post(
            f"{self.base_url}/v1/chat/completions",
            json=payload,
            timeout=kwargs.get('timeout', 60)
        )
        response.raise_for_status()
        return response.json()
    
    def text_completion(self, prompt: str, **kwargs) -> Dict:
        """Send text completion request."""
        if not self.session:
            raise RuntimeError("Requests not available")
        
        payload = {
            "model": "asthra-gemma3-4b",
            "prompt": prompt,
            "max_tokens": kwargs.get('max_tokens', 2048),
            "temperature": kwargs.get('temperature', 0.7),
            "top_p": kwargs.get('top_p', 0.9),
            "stop": kwargs.get('stop', None)
        }
        
        response = self.session.post(
            f"{self.base_url}/v1/completions",
            json=payload,
            timeout=kwargs.get('timeout', 60)
        )
        response.raise_for_status()
        return response.json()

def example_openai_client():
    """Example using the official OpenAI Python client."""
    if not HAS_OPENAI:
        print("❌ OpenAI client not available")
        return
    
    print("🚀 Testing with OpenAI Python Client")
    print("=" * 50)
    
    # Initialize client pointing to local server
    client = OpenAI(
        base_url="http://localhost:8008/v1",
        api_key="not-required"  # Our server doesn't require API key
    )
    
    try:
        # Test chat completion
        print("💬 Chat Completion Example:")
        response = client.chat.completions.create(
            model="asthra-gemma3-4b",
            messages=[
                {"role": "system", "content": "You are an expert Asthra programmer."},
                {"role": "user", "content": "Write a Hello World program in Asthra"}
            ],
            max_tokens=512,
            temperature=0.7
        )
        
        print("Response:", response.choices[0].message.content)
        print()
        
        # Test text completion
        print("📝 Text Completion Example:")
        response = client.completions.create(
            model="asthra-gemma3-4b",
            prompt="// Asthra function to calculate factorial\npub fn factorial(n: i32) -> i32 {",
            max_tokens=256,
            temperature=0.5,
            stop=["}"]
        )
        
        print("Completion:", response.choices[0].text)
        print()
        
        # Test streaming
        print("🌊 Streaming Example:")
        stream = client.chat.completions.create(
            model="asthra-gemma3-4b",
            messages=[
                {"role": "user", "content": "Explain Asthra's memory management system"}
            ],
            max_tokens=512,
            stream=True
        )
        
        print("Streaming response: ", end="", flush=True)
        for chunk in stream:
            if chunk.choices[0].delta.content is not None:
                print(chunk.choices[0].delta.content, end="", flush=True)
        print("\n")
        
    except Exception as e:
        print(f"❌ Error: {e}")

def example_requests_client():
    """Example using requests library."""
    if not HAS_REQUESTS:
        print("❌ Requests library not available")
        return
    
    print("🌐 Testing with Requests Library")
    print("=" * 50)
    
    base_url = "http://localhost:8008"
    
    try:
        # Test server health
        print("🏥 Health Check:")
        response = requests.get(f"{base_url}/health", timeout=5)
        print("Health status:", response.json())
        print()
        
        # Test list models
        print("📋 List Models:")
        response = requests.get(f"{base_url}/v1/models", timeout=5)
        models = response.json()
        print("Available models:", [model['id'] for model in models['data']])
        print()
        
        # Test chat completion
        print("💬 Chat Completion:")
        payload = {
            "model": "asthra-gemma3-4b",
            "messages": [
                {"role": "user", "content": "Create an Asthra struct for a Point with x and y coordinates"}
            ],
            "max_tokens": 512,
            "temperature": 0.7
        }
        
        response = requests.post(f"{base_url}/v1/chat/completions", json=payload, timeout=30)
        result = response.json()
        print("Response:", result['choices'][0]['message']['content'])
        print()
        
    except Exception as e:
        print(f"❌ Error: {e}")

async def example_async_client():
    """Example using async httpx client."""
    if not HAS_HTTPX:
        print("❌ httpx library not available")
        return
    
    print("⚡ Testing with Async httpx Client")
    print("=" * 50)
    
    base_url = "http://localhost:8008"
    
    async with httpx.AsyncClient(timeout=60.0) as client:
        try:
            # Test multiple concurrent requests
            print("🔄 Concurrent Requests:")
            
            prompts = [
                "Write an Asthra function to reverse a string",
                "Create an Asthra enum for HTTP status codes",
                "Implement an Asthra trait for Comparable objects"
            ]
            
            tasks = []
            for i, prompt in enumerate(prompts):
                payload = {
                    "model": "asthra-gemma3-4b",
                    "messages": [{"role": "user", "content": prompt}],
                    "max_tokens": 256,
                    "temperature": 0.7
                }
                
                task = client.post(f"{base_url}/v1/chat/completions", json=payload)
                tasks.append((i, task))
            
            # Wait for all responses
            for i, task in tasks:
                response = await task
                result = response.json()
                print(f"Response {i+1}:", result['choices'][0]['message']['content'][:100] + "...")
                print()
                
        except Exception as e:
            print(f"❌ Error: {e}")

def example_custom_client():
    """Example using custom Asthra client."""
    print("🎯 Testing with Custom Asthra Client")
    print("=" * 50)
    
    client = AsthraiClient("http://localhost:8008")
    
    try:
        # Generate code
        print("🔧 Code Generation:")
        code = client.generate_code(
            "Create an Asthra function that implements binary search",
            max_tokens=512,
            temperature=0.5
        )
        print("Generated code:", code)
        print()
        
        # Explain code
        print("📖 Code Explanation:")
        explanation = client.explain_code(
            """
            pub fn fibonacci(n: i32) -> i32 {
                if n <= 1 {
                    return n;
                }
                return fibonacci(n - 1) + fibonacci(n - 2);
            }
            """,
            max_tokens=256
        )
        print("Explanation:", explanation)
        print()
        
        # Debug code
        print("🐛 Code Debugging:")
        debug_help = client.debug_code(
            "let x = 5;\nlet y = x + z;",
            "Undefined variable 'z'",
            max_tokens=256
        )
        print("Debug help:", debug_help)
        print()
        
    except Exception as e:
        print(f"❌ Error: {e}")

def benchmark_performance():
    """Simple performance benchmark."""
    if not HAS_REQUESTS:
        print("❌ Cannot run benchmark without requests")
        return
    
    print("⚡ Performance Benchmark")
    print("=" * 50)
    
    base_url = "http://localhost:8008"
    test_prompt = "Write a simple Asthra function"
    
    times = []
    
    for i in range(5):
        print(f"Request {i+1}/5...", end=" ", flush=True)
        
        start_time = time.time()
        
        payload = {
            "model": "asthra-gemma3-4b",
            "messages": [{"role": "user", "content": test_prompt}],
            "max_tokens": 128,
            "temperature": 0.7
        }
        
        try:
            response = requests.post(f"{base_url}/v1/chat/completions", json=payload, timeout=30)
            response.raise_for_status()
            
            end_time = time.time()
            duration = end_time - start_time
            times.append(duration)
            
            print(f"{duration:.2f}s")
            
        except Exception as e:
            print(f"Error: {e}")
    
    if times:
        avg_time = sum(times) / len(times)
        min_time = min(times)
        max_time = max(times)
        
        print()
        print(f"📊 Results:")
        print(f"Average: {avg_time:.2f}s")
        print(f"Min: {min_time:.2f}s") 
        print(f"Max: {max_time:.2f}s")
        print(f"Requests/min: {60/avg_time:.1f}")

def main():
    """Run all examples."""
    print("🚀 Asthra OpenAI-Compatible API Client Examples")
    print("=" * 60)
    print()
    
    # Check if server is running
    try:
        if HAS_REQUESTS:
            response = requests.get("http://localhost:8008/health", timeout=5)
            if response.status_code == 200:
                print("✅ Server is running and healthy")
            else:
                print("⚠️  Server responded but may have issues")
        else:
            print("⚠️  Cannot check server health without requests library")
    except:
        print("❌ Server not running. Start with:")
        print("   python servers/fastapi_server.py --model ./outputs/asthra-gemma3-4b-v1 --port 8008")
        print("   or")
        print("   python servers/vllm_server.py --model ./outputs/asthra-gemma3-4b-v1 --port 8008")
        return
    
    print()
    
    # Run examples
    example_openai_client()
    print()
    
    example_requests_client()
    print()
    
    example_custom_client()
    print()
    
    # Run async example
    if HAS_HTTPX:
        asyncio.run(example_async_client())
        print()
    
    # Performance benchmark
    benchmark_performance()

if __name__ == '__main__':
    main() 
