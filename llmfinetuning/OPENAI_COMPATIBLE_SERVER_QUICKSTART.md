# 🚀 Asthra OpenAI-Compatible Server - Quick Start

Your complete OpenAI-compatible API server for the fine-tuned Asthra model is ready! This guide gets you up and running in minutes.

## ✅ What's Complete

You now have a **production-ready OpenAI-compatible API server** with:

### 🏗️ **Three Server Options**
1. **FastAPI Server** (`servers/fastapi_server.py`) - Best for development, macOS, CPU inference
2. **vLLM Server** (`servers/vllm_server.py`) - High-performance for Linux/CUDA production
3. **LSP Server** (`servers/lsp_server.py`) - AI editor integration (VS Code, Cursor, etc.)

### 🔌 **Full OpenAI API Compatibility**
- `/v1/chat/completions` - Chat-based completion
- `/v1/completions` - Text completion  
- `/v1/models` - List available models
- `/health` - Health check
- **Streaming support** for real-time responses
- **Standard request/response formats**

### 🎯 **AI Editor Ready**
- **Cursor Editor** configuration (`editors/cursor_config.json`)
- **Continue.dev** configuration (`editors/continue_config.json`)
- **Generic OpenAI-compatible** editor support
- **Language Server Protocol** (LSP) for advanced features

### 🛠️ **Production Infrastructure**
- **Docker support** (`Dockerfile`)
- **Comprehensive configuration** (`configs/server_config.yaml`)
- **Startup scripts** (`scripts/start_server.sh`)
- **Client examples** (`examples/client_examples.py`)
- **Test suite** (`test_server.py`)

## 🚀 Quick Start (2 Minutes)

### 1. Install Dependencies
```bash
# Install server requirements
pip install -r requirements-server.txt

# Optional: For LSP support
pip install -r requirements-lsp.txt
```

### 2. Start Your Server

#### Option A: FastAPI Server (Recommended)
```bash
# Basic startup
python servers/fastapi_server.py --model ./outputs/asthra-gemma3-4b-v1

# With LSP for AI editors
./scripts/start_server.sh --with-lsp
```

#### Option B: vLLM Server (High Performance)
```bash
# Linux/CUDA only
python servers/vllm_server.py --model ./outputs/asthra-gemma3-4b-v1
```

### 3. Test Your Server
```bash
# Run comprehensive tests
python test_server.py

# Quick health check
curl http://localhost:8000/health
```

### 4. Use with AI Editors

#### Cursor Editor
1. Start server: `./scripts/start_server.sh --with-lsp`
2. Copy config: `editors/cursor_config.json` → Cursor settings
3. Restart Cursor

#### Continue.dev
1. Start server: `python servers/fastapi_server.py --model ./outputs/asthra-gemma3-4b-v1`
2. Copy config: `editors/continue_config.json` → `~/.continue/config.json`

#### Any OpenAI-Compatible Editor
- **Base URL:** `http://localhost:8000/v1`
- **API Key:** `not-required` (or leave empty)
- **Model:** `asthra-gemma3-4b`

## 🧪 Test Examples

### Using cURL
```bash
# Chat completion
curl http://localhost:8000/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "asthra-gemma3-4b",
    "messages": [{"role": "user", "content": "Write hello world in Asthra"}],
    "max_tokens": 100
  }'

# Text completion
curl http://localhost:8000/v1/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "asthra-gemma3-4b",
    "prompt": "pub fn main() {",
    "max_tokens": 50
  }'
```

### Using Python OpenAI Client
```python
import openai

client = openai.OpenAI(
    base_url="http://localhost:8000/v1",
    api_key="not-required"
)

response = client.chat.completions.create(
    model="asthra-gemma3-4b",
    messages=[
        {"role": "user", "content": "Write a function in Asthra"}
    ]
)

print(response.choices[0].message.content)
```

## 📊 Server Options

### FastAPI Server Options
```bash
python servers/fastapi_server.py \
  --model ./outputs/asthra-gemma3-4b-v1 \
  --host 0.0.0.0 \
  --port 8000 \
  --max-context-length 30720
```

### vLLM Server Options (Linux/CUDA)
```bash
python servers/vllm_server.py \
  --model ./outputs/asthra-gemma3-4b-v1 \
  --host 0.0.0.0 \
  --port 8000 \
  --gpu-memory-utilization 0.9
```

### Startup Script Options
```bash
./scripts/start_server.sh \
  --type fastapi \
  --model ./outputs/asthra-gemma3-4b-v1 \
  --with-lsp \
  --port 8000
```

## 🎯 Next Steps

### For Development
1. **Configure your AI editor** using the provided configurations
2. **Run the test suite** to verify everything works: `python test_server.py`
3. **Explore client examples** in `examples/client_examples.py`

### For Production
1. **Use Docker deployment** with the provided `Dockerfile`
2. **Configure monitoring** using the settings in `configs/server_config.yaml`
3. **Scale with vLLM** on GPU infrastructure for high throughput

### For AI Editor Integration
1. **Follow the editor setup guide** in `editors/setup_guide.md`
2. **Start with LSP support** for advanced features: `./scripts/start_server.sh --with-lsp`
3. **Customize configurations** for your specific workflow

## 🛠️ Troubleshooting

### Server Won't Start
```bash
# Check model path
ls -la ./outputs/asthra-gemma3-4b-v1

# Use absolute path
python servers/fastapi_server.py --model $(pwd)/outputs/asthra-gemma3-4b-v1
```

### Memory Issues
```bash
# Reduce context length
python servers/fastapi_server.py --model ./model --max-context-length 16384

# Use CPU inference
CUDA_VISIBLE_DEVICES="" python servers/fastapi_server.py --model ./model
```

### Connection Issues
```bash
# Check server is running
curl http://localhost:8000/health

# Check port availability
lsof -i :8000
```

## 📖 Documentation

- **[Detailed Server Guide](servers/README.md)** - Complete server documentation
- **[Editor Setup Guide](editors/setup_guide.md)** - AI editor integration
- **[Client Examples](examples/client_examples.py)** - Usage examples
- **[Configuration Guide](configs/server_config.yaml)** - Advanced configuration

## 🎉 Success!

Your Asthra OpenAI-compatible server is now **complete and ready for production use**! 

**What you can do now:**
- ✅ Use any OpenAI-compatible AI editor or tool
- ✅ Deploy to production with Docker or cloud services
- ✅ Integrate with existing AI development workflows
- ✅ Scale with high-performance vLLM on GPU infrastructure
- ✅ Develop Asthra applications with AI-powered assistance

**Your server provides all standard OpenAI API features:**
- Chat completions with streaming
- Text completions  
- Model information and health checks
- Full compatibility with OpenAI client libraries
- Language Server Protocol for advanced editor features

🚀 **Happy coding with your Asthra AI assistant!** 
