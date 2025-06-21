# Asthra Model to Ollama Export Guide

This guide helps you convert your trained Asthra LoRA model to GGUF format for use with Ollama.

## 🎯 Overview

The conversion process involves:
1. **Merging** your LoRA adapter with the base model (Gemma 3 4B)
2. **Converting** to GGUF format for efficient inference
3. **Creating** an Ollama Modelfile with Asthra-specific prompts
4. **Installing** the model in Ollama for local use

## 📋 Prerequisites

### 1. Install Ollama
```bash
# macOS
brew install ollama

# Linux
curl -fsSL https://ollama.ai/install.sh | sh

# Windows
# Download from https://ollama.ai/download
```

### 2. Install Dependencies
```bash
# Install GGUF conversion tools
pip install -r requirements-ollama.txt

# Alternative: Install llama.cpp manually for more control
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp
make
pip install -r requirements.txt
```

## 🚀 Quick Start (2 Steps)

### Step 1: Convert Your Model
```bash
# Basic conversion (recommended)
python scripts/export_to_ollama.py --model-path ./outputs/asthra-gemma3-4b-v1

# Advanced: Custom quantization and output
python scripts/export_to_ollama.py \
  --model-path ./outputs/asthra-gemma3-4b-v1 \
  --output-dir ./outputs/ollama/ \
  --model-name asthra-gemma3-4b \
  --quantization Q4_K_M
```

### Step 2: Use Your Model
```bash
# Chat with your Asthra model
ollama run asthra-gemma3-4b

# Generate specific Asthra code
ollama run asthra-gemma3-4b "Write a concurrent web server in Asthra"
```

## ⚙️ Advanced Options

### Quantization Types
Choose based on your needs:

| Quantization | Size | Quality | Use Case |
|--------------|------|---------|----------|
| `Q4_0` | Smallest | Good | Mobile/Low memory |
| `Q4_K_M` | Small | Better | **Recommended** |
| `Q5_K_M` | Medium | Best | High quality |
| `Q8_0` | Large | Excellent | Maximum quality |

```bash
# High quality (larger file)
python scripts/export_to_ollama.py \
  --model-path ./outputs/asthra-gemma3-4b-v1 \
  --quantization Q5_K_M

# Mobile-friendly (smaller file)
python scripts/export_to_ollama.py \
  --model-path ./outputs/asthra-gemma3-4b-v1 \
  --quantization Q4_0
```

### Manual Installation
If automatic installation fails:

```bash
# 1. Export without installing
python scripts/export_to_ollama.py \
  --model-path ./outputs/asthra-gemma3-4b-v1 \
  --skip-install

# 2. Manually install in Ollama
cd outputs/ollama/
ollama create asthra-gemma3-4b -f Modelfile
```

### Merge Only (No GGUF)
For debugging or using with other tools:

```bash
# Only merge LoRA, skip GGUF conversion
python scripts/export_to_ollama.py \
  --model-path ./outputs/asthra-gemma3-4b-v1 \
  --merge-only

# Merged HuggingFace model will be in outputs/ollama/merged_model/
```

## 🎯 Usage Examples

### Basic Chat
```bash
$ ollama run asthra-gemma3-4b
>>> Write a simple HTTP server in Asthra

package main;

import "asthra/net";
import "asthra/io";

pub fn main(none) -> Result<void, string> {
    let listener = TcpListener::bind("127.0.0.1:8080")?;
    println!("Server running on http://localhost:8080");
    
    for stream in listener.incoming() {
        match stream {
            Result.Ok(stream) => handle_client(stream)?,
            Result.Err(e) => eprintln!("Connection error: {}", e),
        }
    }
    Result.Ok(none)
}

priv fn handle_client(stream: TcpStream) -> Result<void, string> {
    let response = "HTTP/1.1 200 OK\r\n\r\nHello from Asthra!";
    stream.write_all(response.as_bytes())?;
    Result.Ok(none)
}
```

### One-Shot Generation
```bash
# Generate a specific function
ollama run asthra-gemma3-4b "Write a factorial function with error handling"

# Generate with custom parameters
ollama run asthra-gemma3-4b --temperature 0.3 "Write a concurrent prime number generator"
```

### API Usage
```bash
# Using Ollama's REST API
curl http://localhost:11434/api/generate -d '{
  "model": "asthra-gemma3-4b",
  "prompt": "Write a binary search function in Asthra",
  "stream": false
}'
```

## 🔧 Troubleshooting

### Common Issues

#### 1. "llama.cpp not found"
```bash
# Solution: Install llama-cpp-python
pip install llama-cpp-python

# Or clone llama.cpp manually
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp && make
```

#### 2. "Out of memory during merge"
```bash
# Solution: Use smaller quantization or add swap
python scripts/export_to_ollama.py \
  --model-path ./outputs/asthra-gemma3-4b-v1 \
  --quantization Q4_0  # Smaller memory usage
```

#### 3. "Ollama create failed"
```bash
# Check Ollama status
ollama ps

# Restart Ollama service
ollama serve

# Check available space
df -h
```

#### 4. "Model generates poor code"
The model might need the Asthra system prompt. Check your Modelfile contains:
```
SYSTEM "You are an expert Asthra programmer..."
```

### Performance Tips

#### 1. Choose Right Quantization
- **Development**: Use Q4_K_M (balanced)
- **Production**: Use Q5_K_M (higher quality)
- **Mobile**: Use Q4_0 (smallest)

#### 2. GPU Acceleration
```bash
# Enable GPU in Ollama (if available)
export OLLAMA_HOST=0.0.0.0:11434
export OLLAMA_CUDA_ENABLED=1
ollama serve
```

#### 3. Memory Management
```bash
# Limit context length for lower memory usage
ollama run asthra-gemma3-4b --parameter num_ctx 2048
```

## 📊 File Sizes & Requirements

| Quantization | File Size | RAM Required | Quality |
|--------------|-----------|--------------|---------|
| Q4_0 | ~2.5 GB | 4 GB | Good |
| Q4_K_M | ~2.7 GB | 4 GB | **Recommended** |
| Q5_K_M | ~3.2 GB | 5 GB | Better |
| Q8_0 | ~4.3 GB | 6 GB | Best |

## 🎉 Success!

After successful conversion, your Asthra model is ready for:

✅ **Local development** with Ollama  
✅ **IDE integration** with Ollama extensions  
✅ **API usage** via Ollama's REST API  
✅ **Custom applications** using Ollama client libraries  
✅ **Offline usage** without internet connection  

Your Asthra model now runs efficiently on your local machine with all the benefits of GGUF format: faster loading, lower memory usage, and CPU-friendly inference!

## 📚 Next Steps

1. **Test your model**: Try different Asthra code generation tasks
2. **Integrate with IDE**: Install Ollama extensions for your editor
3. **Build applications**: Use Ollama's API in your projects  
4. **Share your model**: Export and share your GGUF files
5. **Fine-tune further**: Train additional LoRA adapters for specialized tasks

🚀 **Happy coding with your local Asthra AI assistant!** 
