#!/usr/bin/env python3
"""
Test script for Asthra OpenAI-Compatible API Server
Quick verification that the server is working correctly
"""

import asyncio
import json
import sys
import time
from pathlib import Path

import httpx
import requests

def test_health_endpoint(base_url: str = "http://localhost:8000"):
    """Test the health endpoint."""
    print("🔍 Testing health endpoint...")
    try:
        response = requests.get(f"{base_url}/health", timeout=5)
        if response.status_code == 200:
            print("✅ Health endpoint working")
            return True
        else:
            print(f"❌ Health endpoint failed: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ Health endpoint error: {e}")
        return False

def test_models_endpoint(base_url: str = "http://localhost:8000"):
    """Test the models endpoint."""
    print("🔍 Testing models endpoint...")
    try:
        response = requests.get(f"{base_url}/v1/models", timeout=5)
        if response.status_code == 200:
            data = response.json()
            models = [model['id'] for model in data.get('data', [])]
            print(f"✅ Models endpoint working, available models: {models}")
            return True, models
        else:
            print(f"❌ Models endpoint failed: {response.status_code}")
            return False, []
    except Exception as e:
        print(f"❌ Models endpoint error: {e}")
        return False, []

def test_completion_endpoint(base_url: str = "http://localhost:8000", model: str = "asthra-gemma3-4b"):
    """Test the completion endpoint."""
    print("🔍 Testing completion endpoint...")
    try:
        payload = {
            "model": model,
            "prompt": "pub fn main() {",
            "max_tokens": 50,
            "temperature": 0.7
        }
        
        response = requests.post(
            f"{base_url}/v1/completions",
            json=payload,
            headers={"Content-Type": "application/json"},
            timeout=30
        )
        
        if response.status_code == 200:
            data = response.json()
            completion = data.get('choices', [{}])[0].get('text', '')
            print(f"✅ Completion endpoint working")
            print(f"   Generated: {completion[:100]}...")
            return True
        else:
            print(f"❌ Completion endpoint failed: {response.status_code}")
            print(f"   Response: {response.text}")
            return False
    except Exception as e:
        print(f"❌ Completion endpoint error: {e}")
        return False

def test_chat_completion_endpoint(base_url: str = "http://localhost:8000", model: str = "asthra-gemma3-4b"):
    """Test the chat completion endpoint."""
    print("🔍 Testing chat completion endpoint...")
    try:
        payload = {
            "model": model,
            "messages": [
                {"role": "user", "content": "Write a simple hello world program in Asthra"}
            ],
            "max_tokens": 100,
            "temperature": 0.7
        }
        
        response = requests.post(
            f"{base_url}/v1/chat/completions",
            json=payload,
            headers={"Content-Type": "application/json"},
            timeout=30
        )
        
        if response.status_code == 200:
            data = response.json()
            message = data.get('choices', [{}])[0].get('message', {}).get('content', '')
            print(f"✅ Chat completion endpoint working")
            print(f"   Generated: {message[:100]}...")
            return True
        else:
            print(f"❌ Chat completion endpoint failed: {response.status_code}")
            print(f"   Response: {response.text}")
            return False
    except Exception as e:
        print(f"❌ Chat completion endpoint error: {e}")
        return False

async def test_streaming_endpoint(base_url: str = "http://localhost:8000", model: str = "asthra-gemma3-4b"):
    """Test the streaming chat completion endpoint."""
    print("🔍 Testing streaming endpoint...")
    try:
        payload = {
            "model": model,
            "messages": [
                {"role": "user", "content": "Write a simple function in Asthra"}
            ],
            "max_tokens": 50,
            "temperature": 0.7,
            "stream": True
        }
        
        async with httpx.AsyncClient(timeout=30) as client:
            async with client.stream(
                "POST",
                f"{base_url}/v1/chat/completions",
                json=payload,
                headers={"Content-Type": "application/json"},
            ) as response:
                if response.status_code == 200:
                    print("✅ Streaming endpoint working")
                    chunk_count = 0
                    async for chunk in response.aiter_text():
                        if chunk.strip() and not chunk.startswith("data: [DONE]"):
                            chunk_count += 1
                            if chunk_count <= 3:  # Show first few chunks
                                print(f"   Chunk {chunk_count}: {chunk[:50]}...")
                    print(f"   Received {chunk_count} chunks")
                    return True
                else:
                    print(f"❌ Streaming endpoint failed: {response.status_code}")
                    return False
    except Exception as e:
        print(f"❌ Streaming endpoint error: {e}")
        return False

def test_openai_client_compatibility(base_url: str = "http://localhost:8000"):
    """Test using the official OpenAI client."""
    print("🔍 Testing OpenAI client compatibility...")
    try:
        import openai
        
        client = openai.OpenAI(
            base_url=f"{base_url}/v1",
            api_key="not-required"
        )
        
        response = client.chat.completions.create(
            model="asthra-gemma3-4b",
            messages=[
                {"role": "user", "content": "Write a comment in Asthra"}
            ],
            max_tokens=30
        )
        
        content = response.choices[0].message.content
        print("✅ OpenAI client compatibility working")
        print(f"   Generated: {content[:100]}...")
        return True
        
    except ImportError:
        print("⚠️  OpenAI client not installed, skipping test")
        return True
    except Exception as e:
        print(f"❌ OpenAI client compatibility error: {e}")
        return False

def main():
    """Run all tests."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Test Asthra OpenAI-Compatible Server")
    parser.add_argument('--url', default='http://localhost:8000', help='Server base URL')
    parser.add_argument('--model', default='asthra-gemma3-4b', help='Model name to test')
    args = parser.parse_args()
    
    print("🚀 Testing Asthra OpenAI-Compatible API Server")
    print("=" * 50)
    print(f"Server URL: {args.url}")
    print(f"Model: {args.model}")
    print()
    
    # Test basic endpoints
    health_ok = test_health_endpoint(args.url)
    if not health_ok:
        print("\n❌ Server health check failed. Is the server running?")
        print(f"   Start server with: python servers/fastapi_server.py --model /path/to/model")
        sys.exit(1)
    
    models_ok, available_models = test_models_endpoint(args.url)
    if not models_ok:
        print("\n❌ Models endpoint failed")
        sys.exit(1)
    
    # Use first available model if specified model not found
    test_model = args.model
    if available_models and test_model not in available_models:
        test_model = available_models[0]
        print(f"⚠️  Model '{args.model}' not found, using '{test_model}'")
    
    print()
    
    # Test generation endpoints
    completion_ok = test_completion_endpoint(args.url, test_model)
    chat_ok = test_chat_completion_endpoint(args.url, test_model)
    
    print()
    
    # Test streaming
    streaming_ok = asyncio.run(test_streaming_endpoint(args.url, test_model))
    
    print()
    
    # Test OpenAI client compatibility
    openai_ok = test_openai_client_compatibility(args.url)
    
    print()
    print("=" * 50)
    
    # Summary
    all_tests = [health_ok, models_ok, completion_ok, chat_ok, streaming_ok, openai_ok]
    passed = sum(all_tests)
    total = len(all_tests)
    
    if passed == total:
        print(f"🎉 All tests passed! ({passed}/{total})")
        print("✅ Your Asthra OpenAI-compatible server is working perfectly!")
        print()
        print("🎯 Next steps:")
        print("   • Configure your AI editor with this server")
        print("   • See examples/client_examples.py for more usage examples")
        print("   • Check servers/README.md for editor integration guides")
    else:
        print(f"⚠️  Some tests failed ({passed}/{total} passed)")
        print("   Check the error messages above for details")
        sys.exit(1)

if __name__ == '__main__':
    main() 
