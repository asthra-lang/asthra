# AI Editor Tooling Setup Guide

This guide explains how to set up **AI editor tooling support** for your fine-tuned `asthra-gemma3-4b` model with popular AI-powered editors.

## 🎯 **What is AI Editor Tooling?**

AI editors expect **"tooling" support** which includes:

1. **Language Server Protocol (LSP)** - Real-time code analysis, completion, diagnostics
2. **Structured Code Analysis** - AST parsing, symbol extraction, type information  
3. **Context-Aware Completion** - Understanding code structure and dependencies
4. **Error Detection & Fixing** - Real-time diagnostics and suggestions
5. **Code Actions** - Refactoring, imports, quick fixes

## 🚀 **Quick Start**

### **Step 1: Start Your Model Server**

```bash
# Start the FastAPI server with your fine-tuned model
cd llmfinetuning
python servers/fastapi_server.py \
    --model-path ./models/asthra-gemma3-4b \
    --host 0.0.0.0 \
    --port 8000
```

### **Step 2: Start the LSP Server**

```bash
# Start the Language Server Protocol server
python servers/lsp_server.py \
    --host 127.0.0.1 \
    --port 9000 \
    --model-api http://localhost:8000/v1
```

### **Step 3: Configure Your Editor**

Choose your preferred AI editor below:

---

## 🎨 **Cursor Editor**

### **Installation**
1. Copy `editors/cursor_config.json` to your Cursor workspace settings
2. Install dependencies:
   ```bash
   pip install pygls openai
   ```

### **Configuration**
Add to your Cursor `settings.json`:

```json
{
  "asthra.lsp.enabled": true,
  "asthra.lsp.serverHost": "127.0.0.1",
  "asthra.lsp.serverPort": 9000,
  "asthra.lsp.modelApi": "http://localhost:8000/v1",
  "asthra.ai.enabled": true,
  "asthra.ai.completionMode": "hybrid",
  "asthra.ai.temperature": 0.3,
  "asthra.ai.maxTokens": 256
}
```

### **Features**
- ✅ AI-powered code completion
- ✅ Hover documentation with AI explanations
- ✅ Real-time diagnostics
- ✅ Symbol extraction and navigation
- ✅ Custom commands (Generate, Explain, Optimize)
- ✅ Keyboard shortcuts (Ctrl+Shift+G for generation)

---

## 🔄 **Continue.dev**

### **Installation**
1. Install Continue.dev extension in VS Code
2. Copy `editors/continue_config.json` to `~/.continue/config.json`

### **Configuration**
The config includes:

```json
{
  "models": [{
    "title": "Asthra Gemma 3 4B (Fine-tuned)",
    "provider": "openai",
    "model": "asthra-gemma3-4b", 
    "apiBase": "http://localhost:8000/v1",
    "contextLength": 32768
  }]
}
```

### **Features**
- ✅ Tab autocomplete with fine-tuned model
- ✅ Custom Asthra commands (`/asthra-explain`, `/asthra-optimize`)
- ✅ Context providers (codebase, diff, terminal)
- ✅ Built-in Asthra coding rules and best practices
- ✅ Documentation integration

### **Custom Commands**
- `/asthra-explain` - Explain selected code
- `/asthra-optimize` - Optimize code performance  
- `/asthra-debug` - Debug issues
- `/asthra-refactor` - Refactor code
- `/asthra-test` - Generate tests
- `/asthra-doc` - Generate documentation
- `/asthra-convert` - Convert code to Asthra

---

## 🧠 **GitHub Copilot**

### **Configuration**
While Copilot doesn't directly support custom models, you can enhance it with our LSP server:

1. **Install LSP extension** for your editor
2. **Configure LSP server** as shown above
3. **Use both together** - Copilot for general completion, LSP for Asthra-specific features

### **VS Code Integration**
Add to `settings.json`:
```json
{
  "asthra.lsp.enabled": true,
  "copilot.enable": {
    "asthra": true
  }
}
```

---

## 🔥 **CodeGPT**

### **Configuration**
1. Install CodeGPT extension
2. Add custom provider:

```json
{
  "codegpt.customProviders": [{
    "name": "Asthra Fine-tuned",
    "apiUrl": "http://localhost:8000/v1/chat/completions",
    "apiKey": "not-required",
    "model": "asthra-gemma3-4b"
  }]
}
```

---

## ⚡ **Codeium**

### **Configuration**
For Codeium integration:

1. Use our **LSP server** for Asthra-specific features
2. Configure Codeium for general assistance
3. **Hybrid approach** - best of both worlds

---

## 🛠️ **LSP Server Features**

Our Language Server Protocol implementation provides:

### **Code Completion**
- **AI-powered completions** using your fine-tuned model
- **Static completions** for keywords, types, snippets
- **Context-aware suggestions** based on surrounding code
- **Hybrid mode** combining AI and static completions

### **Hover Information**
- **AI explanations** of code elements
- **Type information** and documentation
- **Contextual help** for functions and variables

### **Diagnostics** 
- **Real-time error detection** using AI analysis
- **Syntax validation** and style suggestions
- **Performance hints** and optimization recommendations

### **Symbols & Navigation**
- **Document symbols** extraction (functions, structs, enums)
- **Workspace symbols** for project-wide navigation
- **Go-to-definition** support

### **Code Actions**
- **Format document** commands
- **Refactoring suggestions** 
- **Quick fixes** for common issues

---

## 🔧 **Advanced Configuration**

### **LSP Server Options**
```bash
python servers/lsp_server.py \
    --host 127.0.0.1 \
    --port 9000 \
    --model-api http://localhost:8000/v1 \
    --verbose  # Enable debug logging
```

### **Model Server Options**
```bash
python servers/fastapi_server.py \
    --model-path ./models/asthra-gemma3-4b \
    --host 0.0.0.0 \
    --port 8000 \
    --device cuda \          # Use GPU
    --max-length 32768 \     # Context length
    --temperature 0.3 \      # Generation temperature
    --top-p 0.9             # Nucleus sampling
```

### **Docker Deployment**
```bash
# Build and run with Docker
docker build -t asthra-lsp .
docker run -p 8000:8000 -p 9000:9000 \
    -v ./models:/app/models \
    asthra-lsp
```

---

## 🎯 **Editor-Specific Tips**

### **Cursor**
- Use **Ctrl+Shift+G** to generate code
- Use **Ctrl+Shift+E** to explain selected code
- Enable **auto-save** for real-time diagnostics

### **Continue.dev**
- Use **Tab completion** for fast coding
- Try **custom commands** with `/asthra-*`
- Enable **codebase indexing** for better context

### **VS Code with LSP**
- Install **"LSP Client"** extension
- Configure **language association** for `.asthra` files
- Use **Command Palette** for LSP commands

---

## 📊 **Performance Optimization**

### **Model Serving**
- Use **GPU acceleration** when available
- Enable **model quantization** for faster inference  
- Adjust **context length** based on needs
- Use **vLLM** for high-throughput serving

### **LSP Server**
- Enable **debounced analysis** to reduce API calls
- Use **symbol caching** for better performance
- Configure **logging levels** appropriately

---

## 🐛 **Troubleshooting**

### **LSP Connection Issues**
```bash
# Check if LSP server is running
netstat -an | grep 9000

# Test LSP server directly
telnet 127.0.0.1 9000
```

### **Model Server Issues**
```bash
# Check model server health
curl http://localhost:8000/health

# Test model inference
curl -X POST http://localhost:8000/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{"model":"asthra-gemma3-4b","messages":[{"role":"user","content":"Hello"}]}'
```

### **Common Issues**

1. **"pygls not found"** → `pip install pygls`
2. **"Model not responding"** → Check model server logs  
3. **"LSP timeout"** → Increase timeout in editor settings
4. **"No completions"** → Verify model API endpoint

---

## 🚀 **What's Next?**

1. **Try different editors** to find your preference
2. **Customize prompts** for your specific use cases
3. **Integrate with CI/CD** for automated code analysis
4. **Share configurations** with your team
5. **Contribute improvements** to the LSP server

---

## 📚 **Additional Resources**

- **[LSP Specification](https://microsoft.github.io/language-server-protocol/)** - Official LSP docs
- **[OpenAI API](https://platform.openai.com/docs/api-reference)** - API compatibility reference
- **[Asthra Documentation](../docs/)** - Language specification and examples
- **[Fine-tuning Guide](../LLM_FINE_TUNING_MASTER_PLAN.md)** - Model training details

The tooling support transforms your fine-tuned Asthra model into a **professional development experience** with intelligent code completion, real-time analysis, and seamless editor integration! 🎉 
