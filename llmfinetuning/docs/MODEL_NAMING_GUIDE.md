# Asthra LLM Model Naming Guide

**Version:** 1.0  
**Created:** January 2025  
**Purpose:** Standardized naming conventions for Asthra fine-tuned models

## 🎯 **Naming Philosophy**

Our model names should be:
- **Clear**: Immediately identifiable as Asthra-related
- **Informative**: Indicate base model and capabilities
- **Consistent**: Follow predictable patterns
- **Professional**: Suitable for production deployment

## 📋 **Standard Naming Convention**

### **Primary Format**
```
asthra-{base-model}-{size}[-{variant}][-v{version}]
```

### **Examples**
```
asthra-gemma3-4b          # Primary recommendation
asthra-gemma3-4b-v1.0     # With version
asthra-codellama-7b       # CodeLlama base
asthra-qwen-3b-coder      # Specialized variant
asthra-mistral-7b-instruct # Instruction-tuned variant
```

## 🏷️ **Specific Model Names**

### **Current Models**

| Base Model | Recommended Name | Description |
|------------|------------------|-------------|
| Gemma-3-4B-IT | `asthra-gemma3-4b` | Latest Gemma 3 multimodal |
| CodeLlama-7B-Instruct | `asthra-codellama-7b` | Code-specialized variant |
| Qwen2.5-Coder-3B | `asthra-qwen-3b-coder` | Compact coding model |
| Mistral-7B-Instruct | `asthra-mistral-7b-instruct` | General instruction model |
| Devstral-Small-2505 | `asthra-devstral-24b` | Target production model |

### **Capability-Based Variants**
```
asthra-gemma3-4b-dev        # Development assistant
asthra-gemma3-4b-debug      # Debugging specialist  
asthra-gemma3-4b-complete   # Code completion focused
asthra-gemma3-4b-explain    # Code explanation focused
```

## 📁 **Directory Structure**

### **Training Outputs**
```
./outputs/
├── asthra-gemma3-4b-training/    # Training checkpoints
├── asthra-gemma3-4b-v1/          # Final model
├── logs/asthra-gemma3-4b/        # Training logs
└── exported/
    ├── asthra-gemma3-4b-q4_k_m.gguf
    ├── asthra-gemma3-4b-q8_0.gguf
    └── ollama/
        └── asthra-gemma3-4b/
```

### **Hugging Face Hub**
```
asthra-team/asthra-gemma3-4b
asthra-lang/asthra-gemma3-4b
[your-username]/asthra-gemma3-4b
```

## 🔧 **Implementation Examples**

### **Configuration Files**
```yaml
# In config YAML
export:
  output_dir: "./outputs/asthra-gemma3-4b-v1"
  hub_model_id: "asthra-team/asthra-gemma3-4b"
  
ollama:
  model_name: "asthra-gemma3-4b"
  
monitoring:
  wandb_run_name: "asthra-gemma3-4b-v1"
```

### **GGUF Export**
```bash
# Quantized variants
asthra-gemma3-4b-q4_k_m.gguf    # 4-bit (recommended)
asthra-gemma3-4b-q5_k_m.gguf    # 5-bit (balanced)
asthra-gemma3-4b-q8_0.gguf      # 8-bit (high quality)
asthra-gemma3-4b-f16.gguf       # Full precision
```

### **Ollama Deployment**
```bash
# Install model
ollama import asthra-gemma3-4b-q4_k_m.gguf asthra-gemma3-4b

# Run model
ollama run asthra-gemma3-4b
ollama run asthra-gemma3-4b:v1.0
```

## 📝 **Model Cards and Documentation**

### **Model Card Template**
```markdown
# Asthra-Gemma3-4B

**Base Model:** google/gemma-3-4b-it  
**Fine-tuned on:** Asthra Programming Language  
**Model Size:** 4.3B parameters  
**Training Data:** Asthra grammar, examples, stdlib  
**Use Case:** Asthra code generation and assistance  

## Usage
```python
from transformers import AutoModelForCausalLM, AutoTokenizer

model = AutoModelForCausalLM.from_pretrained("asthra-team/asthra-gemma3-4b")
tokenizer = AutoTokenizer.from_pretrained("asthra-team/asthra-gemma3-4b")
```

### **README Template**
```markdown
# 🚀 Asthra-Gemma3-4B

Fine-tuned Gemma 3 4B model specialized for the Asthra programming language.

## Quick Start

### Ollama (Recommended)
```bash
ollama pull asthra-team/asthra-gemma3-4b
ollama run asthra-gemma3-4b "Write a Hello World in Asthra"
```

### Python
```python
# Example usage code
```

## Performance
- **Compilation Success Rate:** 90%+
- **Semantic Correctness:** 85%+
- **Idiomatic Quality:** 80%+
```

## 🎯 **Version Management**

### **Semantic Versioning**
```
asthra-gemma3-4b-v1.0.0    # Major.Minor.Patch
asthra-gemma3-4b-v1.1.0    # Feature addition
asthra-gemma3-4b-v1.0.1    # Bug fix
asthra-gemma3-4b-v2.0.0    # Breaking change
```

### **Release Tags**
```
asthra-gemma3-4b:latest    # Latest stable
asthra-gemma3-4b:v1.0      # Specific version
asthra-gemma3-4b:beta      # Beta release
asthra-gemma3-4b:dev       # Development build
```

## 🌟 **Best Practices**

### **DO ✅**
- Use consistent `asthra-` prefix
- Include base model name and size
- Use lowercase with hyphens
- Version your models
- Create clear model cards
- Document capabilities and limitations

### **DON'T ❌**
- Use generic names like "model" or "llm"
- Mix naming conventions
- Skip versioning
- Use special characters or spaces
- Make names too long (>50 characters)
- Forget to document training details

## 📊 **Naming Decision Matrix**

| Factor | Weight | Considerations |
|--------|--------|----------------|
| **Clarity** | High | Immediately identifiable as Asthra model |
| **Base Model** | High | Include original model name/size |
| **Version** | Medium | Enable tracking of improvements |
| **Length** | Medium | Balance clarity with brevity |
| **Compatibility** | High | Work with all deployment platforms |

## 🚀 **Deployment Examples**

### **Local Development**
```bash
# Load model locally
python -c "from transformers import pipeline; 
pipe = pipeline('text-generation', 'asthra-team/asthra-gemma3-4b'); 
print(pipe('Write an Asthra function:'))"
```

### **Production Deployment**
```yaml
# Docker deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: asthra-gemma3-4b-api
spec:
  template:
    spec:
      containers:
      - name: model-server
        image: asthra-team/asthra-gemma3-4b:v1.0
```

## 📚 **Related Documentation**

- [Model Training Guide](../MODEL_TRAINING_GUIDE.md)
- [Deployment Guide](../DEPLOYMENT_GUIDE.md)
- [Evaluation Framework](../EVALUATION_FRAMEWORK.md)
- [Contributing Guidelines](../CONTRIBUTING.md)

---

**Recommendation:** Use `asthra-gemma3-4b` as your primary model name. It's clear, professional, and follows all best practices while being memorable and easy to use across all deployment platforms. 
