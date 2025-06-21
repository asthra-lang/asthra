# 🖥️ Platform Comparison: macOS vs Linux for Asthra LLM Training

This guide compares training your Asthra LLM on different platforms and provides specific setup instructions for each.

## 📊 **Quick Comparison**

| Feature | macOS (M1/M2/M3) | macOS (Intel) | Linux (NVIDIA GPU) | Linux (CPU Only) |
|---------|------------------|---------------|-------------------|------------------|
| **Performance** | Good (MPS) | Limited (CPU) | Excellent (CUDA) | Limited |
| **Memory** | Unified (24-96GB) | Limited (32GB) | GPU VRAM (8-80GB) | System RAM |
| **Setup Complexity** | Easy | Easy | Moderate | Easy |
| **Cost** | Hardware owned | Hardware owned | Cloud/Hardware | Low |
| **Libraries** | Limited | Limited | Full Support | Limited |
| **Recommended Use** | Development | Light testing | Production training | Testing only |

## 🍎 **macOS Setup**

### **Apple Silicon (M1/M2/M3) - RECOMMENDED for Mac**

**Advantages:**
- ✅ Unified memory architecture (24-96GB)
- ✅ Metal Performance Shaders (MPS) acceleration
- ✅ Energy efficient
- ✅ No external dependencies

**Limitations:**
- ❌ No CUDA support
- ❌ Limited ML library support
- ❌ Slower than dedicated GPUs
- ❌ Some libraries not available (vLLM, Flash Attention)

**Setup:**
```bash
# Use the standard macOS requirements
pip install -r requirements.txt

# Training command
python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
```

**Expected Performance:**
- **Training Time**: 8-12 hours for Gemma3 4B
- **Memory Usage**: ~16-24GB unified memory
- **Best Model Size**: Up to 7B parameters

### **Intel Mac - NOT RECOMMENDED**

**Why avoid:**
- ❌ CPU-only training (very slow)
- ❌ Limited memory
- ❌ Poor performance for modern LLMs

**Alternative:**
Use cloud training instead (see Cloud Training Guide)

## 🐧 **Linux Setup**

### **RHEL/CentOS/Rocky Linux - PRODUCTION RECOMMENDED**

**Advantages:**
- ✅ Full CUDA support
- ✅ All ML libraries available
- ✅ Enterprise-grade stability
- ✅ Best performance
- ✅ Production deployment ready

**Requirements:**
- NVIDIA GPU (RTX 3090, RTX 4090, A100, H100)
- RHEL/CentOS/Rocky Linux 8+
- CUDA 11.8+ drivers

**Setup:**
```bash
# 1. Full system setup (run as sudo)
sudo bash scripts/setup_rhel.sh

# 2. Install Python dependencies
pip install -r requirements-linux.txt

# 3. Start training
python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
```

**Expected Performance:**
- **RTX 4090**: 3-5 hours for Gemma3 4B
- **A100 40GB**: 2-3 hours for Gemma3 4B
- **A100 80GB**: 1-2 hours for Gemma3 4B

### **Ubuntu/Debian**

**Setup:**
```bash
# Same as RHEL but uses apt instead of yum/dnf
sudo bash scripts/setup_rhel.sh  # Detects Ubuntu automatically
pip install -r requirements-linux.txt
```

## 📋 **Requirements Files Comparison**

### **requirements.txt (macOS Optimized)**
```bash
# Optimized for Apple Silicon
# Missing: vLLM, Flash Attention, bitsandbytes, CUDA libraries
# Includes: MPS support, Metal optimizations
pip install -r requirements.txt
```

### **requirements-linux.txt (Linux Full Power)**
```bash
# Includes ALL available optimizations
# Has: vLLM, Flash Attention, bitsandbytes, CUDA libraries, DeepSpeed
# Performance: 3-5x faster than macOS
pip install -r requirements-linux.txt
```

### **requirements-server.txt (API Server)**
```bash
# For running OpenAI-compatible servers
# Cross-platform compatibility
pip install -r requirements-server.txt
```

## 🚀 **Performance Benchmarks**

Based on Gemma3 4B training (1000 samples, 3 epochs):

| Platform | Hardware | Training Time | Memory Usage | Cost |
|----------|----------|---------------|--------------|------|
| **M2 Max 96GB** | Mac Studio | 8-10 hours | 24GB unified | $0 (owned) |
| **RTX 4090 24GB** | Linux Desktop | 3-4 hours | 18GB VRAM | $0 (owned) |
| **A100 40GB** | Cloud Linux | 2-3 hours | 32GB VRAM | $6-9 |
| **A100 80GB** | Cloud Linux | 1-2 hours | 64GB VRAM | $15-30 |
| **Intel Mac** | MacBook Pro | 24+ hours | 32GB RAM | $0 (not recommended) |

## 🛠️ **Platform-Specific Configurations**

### **macOS Configuration**
```yaml
# configs/macos_optimized_config.yaml
training:
  per_device_train_batch_size: 1    # Smaller batches for MPS
  gradient_accumulation_steps: 16   # Compensate with accumulation
  fp16: false                       # Use bf16 on Apple Silicon
  bf16: true
  dataloader_num_workers: 0         # Avoid multiprocessing issues

hardware:
  device_map: "mps"                 # Metal Performance Shaders
  max_memory_mb: 24000             # Conservative memory limit
```

### **Linux High-Performance Configuration**
```yaml
# configs/linux_optimized_config.yaml
training:
  per_device_train_batch_size: 4    # Larger batches for CUDA
  gradient_accumulation_steps: 4    # Fewer accumulation steps
  fp16: true                        # NVIDIA mixed precision
  dataloader_num_workers: 4         # Parallel data loading

hardware:
  device_map: "auto"                # Automatic GPU detection
  max_memory_mb: 40000             # Use most of GPU memory
```

## 🔧 **Setup Scripts**

### **Automatic Platform Detection**
```bash
# Universal setup script that detects your platform
bash scripts/detect_and_setup.sh
```

### **Platform-Specific Scripts**
```bash
# macOS setup
bash scripts/setup_macos.sh

# RHEL/Linux setup (with CUDA)
sudo bash scripts/setup_rhel.sh

# Cloud setup (any Linux instance)
bash scripts/quick_cloud_setup.sh
```

## 💡 **Recommendations by Use Case**

### **🧪 Development & Experimentation**
**Best Choice**: Apple Silicon Mac
- Fast iteration
- Good for smaller models
- No cloud costs
- Energy efficient

**Setup**: `pip install -r requirements.txt`

### **🏭 Production Training**
**Best Choice**: Linux with NVIDIA GPU
- Fastest training
- Full library support
- Scalable to larger models
- Best cost per training hour

**Setup**: `sudo bash scripts/setup_rhel.sh && pip install -r requirements-linux.txt`

### **💰 Budget-Conscious**
**Best Choice**: Cloud Linux (Vast.ai/RunPod)
- No hardware investment
- Pay only for training time
- Access to latest GPUs
- $1-5 per training run

**Setup**: Use cloud setup scripts

### **🏢 Enterprise**
**Best Choice**: On-premise Linux servers
- Data security
- Compliance requirements
- Predictable costs
- Full control

**Setup**: RHEL setup with enterprise considerations

## 🚨 **Common Issues & Solutions**

### **macOS Issues**
```bash
# MPS not available
export PYTORCH_ENABLE_MPS_FALLBACK=1

# Memory issues
export PYTORCH_MPS_HIGH_WATERMARK_RATIO=0.0

# Compilation errors
export MACOSX_DEPLOYMENT_TARGET=11.0
```

### **Linux Issues**
```bash
# CUDA out of memory
export CUDA_VISIBLE_DEVICES=0
nvidia-smi  # Check GPU usage

# Driver issues
sudo nvidia-smi
sudo systemctl restart nvidia-persistenced

# Permission issues
sudo usermod -aG docker $USER
newgrp docker
```

## 📚 **Next Steps**

1. **Choose your platform** based on the comparison above
2. **Run the appropriate setup script**
3. **Install the correct requirements file**
4. **Use the platform-optimized configuration**
5. **Follow the platform-specific troubleshooting guide**

For detailed setup instructions, see:
- [Cloud Training Guide](CLOUD_TRAINING_GUIDE.md)
- [RHEL Setup Script](../scripts/setup_rhel.sh)
- [Server Setup Guide](../servers/README.md)

## 🤝 **Hybrid Approach**

**Recommended Workflow:**
1. **Develop on macOS**: Test configurations, small experiments
2. **Train on Linux**: Production training runs
3. **Deploy anywhere**: OpenAI-compatible server works on both

This gives you the best of both worlds: convenient development and powerful training! 
