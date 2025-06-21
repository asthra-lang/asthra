# Asthra OpenAI-Compatible API Servers

This directory contains OpenAI-compatible API servers for serving your fine-tuned Asthra models. These servers provide standard OpenAI API endpoints that work with existing AI tools and editors.

## 🚀 Quick Start

### 1. Install Dependencies

```bash
# Install server dependencies
pip install -r ../requirements-server.txt

# For LSP support (optional)
pip install -r ../requirements-lsp.txt
```

### 2. Start a Server

#### FastAPI Server (Recommended)
```bash
# Basic usage
python fastapi_server.py --model /path/to/asthra-gemma3-4b

# With custom host/port
python fastapi_server.py --model ./outputs/asthra-gemma3-4b --host 0.0.0.0 --port 8000

# Start with LSP support
../scripts/start_server.sh --with-lsp
```

#### vLLM Server (High Performance - Linux/CUDA only)
```bash
python vllm_server.py --model /path/to/asthra-gemma3-4b --port 8001
```

#### Using the Startup Script (Recommended)
```bash
# Start FastAPI server with LSP
../scripts/start_server.sh --with-lsp

# Start vLLM server
../scripts/start_server.sh --type vllm --model ./outputs/asthra-gemma3-4b

# Start only LSP server (assumes model server running on port 8000)
../scripts/start_server.sh --lsp-only
```

## 📋 Available Servers

### 1. FastAPI Server (`fastapi_server.py`)
- **Best for:** Development, macOS, CPU inference
- **Features:** Full OpenAI API compatibility, streaming, custom generation
- **Requirements:** `torch`, `transformers`, `fastapi`

### 2. vLLM Server (`vllm_server.py`)  
- **Best for:** Production, Linux with CUDA GPUs
- **Features:** High-performance inference, batching, quantization
- **Requirements:** `vllm`, CUDA-compatible GPU

### 3. LSP Server (`lsp_server.py`)
- **Best for:** AI editor integration (Cursor, VS Code, etc.)
- **Features:** Language Server Protocol, hover info, completions
- **Requirements:** `pygls`, running model server

## 🔌 API Endpoints

All servers provide standard OpenAI-compatible endpoints:

### Chat Completions
```bash
POST /v1/chat/completions
```

Example:
```python
import openai

client = openai.OpenAI(
    base_url="http://localhost:8000/v1",
    api_key="not-required"
)

response = client.chat.completions.create(
    model="asthra-gemma3-4b",
    messages=[
        {"role": "user", "content": "Write a hello world program in Asthra"}
    ]
)
```

### Text Completions
```bash
POST /v1/completions
```

### Model Info
```bash
GET /v1/models
```

### Health Check
```bash
GET /health
```

## 🛠️ Configuration

### FastAPI Server Options
```bash
python fastapi_server.py --help
```

Key options:
- `--model`: Path to fine-tuned model
- `--host`: Server host (default: 0.0.0.0)
- `--port`: Server port (default: 8000)
- `--max-context-length`: Maximum context length (default: 30720)

### vLLM Server Options
```bash
python vllm_server.py --help
```

Key options:
- `--model`: Path to fine-tuned model
- `--host`: Server host
- `--port`: Server port
- `--gpu-memory-utilization`: GPU memory usage (0.0-1.0)
- `--max-model-len`: Maximum model length

## 🎯 Using with AI Editors

### Cursor Editor
1. Start server with LSP: `../scripts/start_server.sh --with-lsp`
2. Install the provided configuration: `../editors/cursor_config.json`
3. Restart Cursor

### Continue.dev
1. Start server: `python fastapi_server.py --model ./outputs/asthra-gemma3-4b`
2. Copy configuration: `../editors/continue_config.json`
3. Follow setup guide: `../editors/setup_guide.md`

### Generic OpenAI-Compatible Editors
Configure with:
- **Base URL:** `http://localhost:8000/v1`
- **API Key:** `not-required` (or leave empty)
- **Model:** `asthra-gemma3-4b`

## 📊 Performance Tips

### FastAPI Server
- Use `torch.compile()` for faster inference (PyTorch 2.0+)
- Enable half-precision with `--dtype float16` on GPU
- Adjust `--max-context-length` based on available memory

### vLLM Server
- Set `--gpu-memory-utilization 0.9` for maximum GPU usage
- Use `--tensor-parallel-size N` for multi-GPU setups
- Enable quantization with `--quantization awq` or `--quantization gptq`

### Memory Management
- Monitor memory usage with `nvidia-smi` (GPU) or `htop` (CPU)
- Reduce batch size if running out of memory
- Use CPU offloading for large models on limited GPU memory

## 🐛 Troubleshooting

### Common Issues

#### "Model not found"
```bash
# Check if model path exists
ls -la /path/to/your/model

# Use absolute path
python fastapi_server.py --model $(pwd)/outputs/asthra-gemma3-4b
```

#### "CUDA out of memory"
```bash
# Reduce context length
python fastapi_server.py --model ./model --max-context-length 16384

# Use CPU inference
CUDA_VISIBLE_DEVICES="" python fastapi_server.py --model ./model
```

#### "Connection refused" 
```bash
# Check if server is running
curl http://localhost:8000/health

# Check port availability
lsof -i :8000
```

#### vLLM import errors on macOS
```bash
# vLLM doesn't support macOS - use FastAPI server instead
python fastapi_server.py --model ./model
```

### Logs and Debugging

Enable debug logging:
```bash
# Set log level
export PYTHONPATH=.:$PYTHONPATH
python fastapi_server.py --model ./model --log-level debug

# Check server logs
tail -f server.log
```

## 🧪 Testing

### Basic API Test
```bash
# Test health endpoint
curl http://localhost:8000/health

# Test completion
curl http://localhost:8000/v1/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "asthra-gemma3-4b",
    "prompt": "pub fn main() {",
    "max_tokens": 100
  }'
```

### Using Client Examples
```bash
# Run comprehensive tests
python ../examples/client_examples.py
```

## 📖 Related Documentation

- **[Client Examples](../examples/client_examples.py)** - Complete usage examples
- **[Server Configuration](../configs/server_config.yaml)** - Advanced configuration
- **[Editor Setup](../editors/setup_guide.md)** - AI editor integration
- **[Docker Deployment](../Dockerfile)** - Container deployment
- **[Model Training](../docs/training_guide.md)** - Fine-tuning your model

## 🤝 Support

- **Issues:** Check troubleshooting section above
- **Performance:** See performance tips section
- **Features:** All standard OpenAI API features supported
- **Models:** Supports any Hugging Face transformers model

## 📜 License

This server infrastructure is part of the Asthra programming language project. 
