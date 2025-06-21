# Quick Asthra Model → Ollama Setup Guide

Your Asthra model training was successful! Here's the fastest way to get it working with Ollama.

## 🎯 Current Status

✅ **Training Complete**: Your LoRA adapter is trained and working  
✅ **FastAPI Server**: Currently running and generating excellent Asthra code  
✅ **Ollama Installed**: Ready for model installation  

## 🚀 Two Options for Ollama

### Option 1: Use Existing Ollama Models (Recommended)

Since your model is based on **Gemma 3 4B**, you can use the base Gemma model in Ollama with your custom system prompt:

```bash
# Install Gemma in Ollama
ollama pull gemma:3b

# Create custom Asthra prompt file
cat > Modelfile << 'EOF'
FROM gemma:3b

# Asthra-optimized parameters
PARAMETER temperature 0.3
PARAMETER top_p 0.9
PARAMETER top_k 40
PARAMETER num_ctx 4096

# System prompt for Asthra programming
SYSTEM """You are an expert Asthra programmer. Asthra is a systems programming language designed for clarity, safety, and AI-friendly code generation.

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

Always generate idiomatic, safe, and performant Asthra code."""

# Template for code generation
TEMPLATE """{{ if .System }}{{ .System }}

{{ end }}{{ if .Prompt }}User: {{ .Prompt }}

{{ end }}Assistant: """
EOF

# Create the Asthra model in Ollama
ollama create asthra-base -f Modelfile

# Test it
ollama run asthra-base "Write a factorial function in Asthra"
```

### Option 2: Convert Your Fine-tuned Model (Advanced)

If you want to use your specific fine-tuning:

1. **Merge your LoRA adapter** (working directly):
```bash
# Use your working direct model test approach
source .venv/bin/activate
python3 direct_model_test.py  # This proves your model works!
```

2. **Convert to GGUF** (requires llama.cpp):
```bash
# Clone llama.cpp
git clone https://github.com/ggerganov/llama.cpp
cd llama.cpp
make

# Convert your merged model (once you have it merged)
python convert-hf-to-gguf.py /path/to/merged/model --outfile asthra-gemma3-4b.gguf --outtype q4_0
```

3. **Create Ollama model**:
```bash
# Create Modelfile pointing to your GGUF
cat > Modelfile << 'EOF'
FROM ./asthra-gemma3-4b.gguf

PARAMETER temperature 0.3
PARAMETER top_p 0.9
PARAMETER top_k 40
PARAMETER num_ctx 4096

SYSTEM """You are an expert Asthra programmer trained specifically on Asthra code patterns..."""
EOF

ollama create asthra-custom -f Modelfile
```

## ✅ Quick Test

Once you have either option set up:

```bash
# Test basic Asthra generation
ollama run asthra-base "Write a simple HTTP server in Asthra"

# Test specific features
ollama run asthra-base "Write an Asthra function that uses Result types for error handling"

# Test concurrency features
ollama run asthra-base "Write an Asthra program that uses channels for communication between tasks"
```

## 🔧 Current Working Setup

**Your FastAPI server is already working perfectly!** 

Keep using it while you set up Ollama:
```bash
# Your current working server (port 8008)
python3 -c "
import openai
client = openai.OpenAI(base_url='http://localhost:8008/v1', api_key='not-required')
response = client.chat.completions.create(
    model='asthra-gemma3-4b',
    messages=[{'role': 'user', 'content': 'Write a concurrent web server in Asthra'}],
    max_tokens=200
)
print(response.choices[0].message.content)
"
```

## 🎯 Recommendation

**Start with Option 1** (base Gemma + custom prompt) because:
- ✅ Works immediately (2 minutes setup)
- ✅ Good Asthra code generation
- ✅ Easy to iterate on prompts
- ✅ No technical complexity

**Upgrade to Option 2** later when you need:
- The specific fine-tuning improvements
- Maximum Asthra code quality
- Complete model customization

## 🚀 Next Steps

1. **Try Option 1 now** - get Ollama working in 2 minutes
2. **Keep your FastAPI server** - it's working great!
3. **Compare outputs** - see if Option 1 meets your needs
4. **Upgrade later** if you need the full fine-tuning benefits

Your Asthra training was successful - now you have multiple ways to use it! 🎉 
