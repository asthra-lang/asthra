#!/bin/bash

# Fix Flash Attention Installation on Ubuntu 22.04.5 LTS
# Script for Asthra LLM Fine-Tuning
# Version: 1.0

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging function
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if running as root (not recommended for conda/pip)
if [[ $EUID -eq 0 ]]; then
   log_warning "Running as root. This may cause permission issues with conda/pip."
   log_info "Consider running as a regular user."
fi

log_info "🚀 Starting Flash Attention Fix for Ubuntu 22.04.5 LTS"

# Step 1: Check system prerequisites
log_info "📋 Checking system prerequisites..."

# Check CUDA availability
if command -v nvidia-smi &> /dev/null; then
    log_success "✅ NVIDIA driver detected"
    nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv,noheader
else
    log_error "❌ NVIDIA driver not found. Install NVIDIA drivers first:"
    echo "  sudo apt update"
    echo "  sudo apt install -y nvidia-driver-535"  # Latest stable
    echo "  sudo reboot"
    exit 1
fi

# Check CUDA toolkit
if command -v nvcc &> /dev/null; then
    CUDA_VERSION=$(nvcc --version | grep release | sed 's/.*release //' | sed 's/,.*//')
    log_success "✅ CUDA toolkit detected: $CUDA_VERSION"
else
    log_warning "⚠️  CUDA toolkit not found in PATH"
    log_info "Installing CUDA toolkit..."
    
    # Install CUDA toolkit for Ubuntu 22.04
    wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
    sudo dpkg -i cuda-keyring_1.1-1_all.deb
    sudo apt-get update
    sudo apt-get -y install cuda-toolkit-12-1
    
    # Add CUDA to PATH
    echo 'export PATH=/usr/local/cuda-12.1/bin:$PATH' >> ~/.bashrc
    echo 'export LD_LIBRARY_PATH=/usr/local/cuda-12.1/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
    source ~/.bashrc
    
    log_success "✅ CUDA toolkit installed"
fi

# Step 2: Install system dependencies for Flash Attention compilation
log_info "🔧 Installing system dependencies..."

sudo apt update
sudo apt install -y \
    build-essential \
    ninja-build \
    cmake \
    git \
    python3-dev \
    python3-pip \
    libnccl2 \
    libnccl-dev \
    libcudnn8 \
    libcudnn8-dev

log_success "✅ System dependencies installed"

# Step 3: Check Python environment
log_info "🐍 Checking Python environment..."

if command -v python &> /dev/null; then
    PYTHON_VERSION=$(python --version 2>&1 | awk '{print $2}')
    log_success "✅ Python detected: $PYTHON_VERSION"
else
    log_error "❌ Python not found"
    exit 1
fi

# Check PyTorch installation
if python -c "import torch; print(f'PyTorch {torch.__version__} (CUDA: {torch.cuda.is_available()})')" 2>/dev/null; then
    TORCH_CUDA=$(python -c "import torch; print(torch.cuda.is_available())")
    if [ "$TORCH_CUDA" = "True" ]; then
        log_success "✅ PyTorch with CUDA support detected"
    else
        log_error "❌ PyTorch without CUDA support detected"
        log_info "Installing PyTorch with CUDA support..."
        pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
    fi
else
    log_warning "⚠️  PyTorch not found. Installing..."
    pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
fi

# Step 4: Clean previous Flash Attention installations
log_info "🧹 Cleaning previous Flash Attention installations..."

pip uninstall -y flash-attn 2>/dev/null || true
pip cache purge

# Remove any cached builds
rm -rf ~/.cache/pip/wheels/flash*
rm -rf /tmp/pip-*flash*

log_success "✅ Previous installations cleaned"

# Step 5: Install Flash Attention from source (most reliable on Ubuntu)
log_info "⚡ Installing Flash Attention from source..."

# Set environment variables for compilation
export FLASH_ATTENTION_FORCE_BUILD=TRUE
export FLASH_ATTENTION_FORCE_CXX11_ABI=TRUE
export MAX_JOBS=4  # Limit parallel jobs to avoid OOM

# Install from source (more reliable than pre-built wheels)
pip install flash-attn --no-build-isolation --verbose

log_success "✅ Flash Attention installed from source"

# Step 6: Alternative installation methods if source fails
if ! python -c "import flash_attn" 2>/dev/null; then
    log_warning "⚠️  Source installation failed. Trying pre-built wheel..."
    
    # Try pre-built wheel for CUDA 12.1
    pip install flash-attn --find-links https://github.com/Dao-AILab/flash-attention/releases
    
    if ! python -c "import flash_attn" 2>/dev/null; then
        log_warning "⚠️  Pre-built wheel failed. Trying conda-forge..."
        
        # Try conda installation if available
        if command -v conda &> /dev/null; then
            conda install -c conda-forge flash-attn -y
        else
            log_error "❌ All installation methods failed"
            log_info "Manual installation required. See troubleshooting section."
        fi
    fi
fi

# Step 7: Verify installation
log_info "🔍 Verifying Flash Attention installation..."

if python -c "import flash_attn; print(f'Flash Attention {flash_attn.__version__} successfully imported')" 2>/dev/null; then
    log_success "✅ Flash Attention verification successful"
    
    # Test basic functionality
    python -c "
import torch
import flash_attn
from flash_attn import flash_attn_func

# Test basic Flash Attention functionality
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
if device.type == 'cuda':
    # Small test tensors
    batch_size, seqlen, num_heads, head_dim = 2, 128, 8, 64
    q = torch.randn(batch_size, seqlen, num_heads, head_dim, device=device, dtype=torch.float16)
    k = torch.randn(batch_size, seqlen, num_heads, head_dim, device=device, dtype=torch.float16)
    v = torch.randn(batch_size, seqlen, num_heads, head_dim, device=device, dtype=torch.float16)
    
    try:
        out = flash_attn_func(q, k, v)
        print('✅ Flash Attention functional test passed')
    except Exception as e:
        print(f'❌ Flash Attention functional test failed: {e}')
else:
    print('⚠️  CUDA not available, skipping functional test')
"
    
else
    log_error "❌ Flash Attention verification failed"
    
    # Provide troubleshooting information
    log_info "🔧 Troubleshooting information:"
    echo "Python version: $(python --version)"
    echo "PyTorch version: $(python -c 'import torch; print(torch.__version__)' 2>/dev/null || echo 'Not installed')"
    echo "CUDA available: $(python -c 'import torch; print(torch.cuda.is_available())' 2>/dev/null || echo 'Unknown')"
    echo "CUDA version: $(python -c 'import torch; print(torch.version.cuda)' 2>/dev/null || echo 'Unknown')"
    echo "GPU info:"
    nvidia-smi --query-gpu=name,compute_cap --format=csv,noheader 2>/dev/null || echo "NVIDIA info not available"
    
    exit 1
fi

# Step 8: Update training configuration
log_info "⚙️  Updating training configuration..."

CONFIG_FILE="../configs/gemma3_config.yaml"
if [ -f "$CONFIG_FILE" ]; then
    # Add Flash Attention configuration if not present
    if ! grep -q "use_flash_attention" "$CONFIG_FILE"; then
        cat >> "$CONFIG_FILE" << EOF

# Flash Attention Configuration (added by fix script)
flash_attention:
  enabled: true
  implementation: "flash_attention_2"
  # Only enable on CUDA devices with compute capability >= 7.5
EOF
        log_success "✅ Configuration updated with Flash Attention settings"
    else
        log_info "Flash Attention configuration already present"
    fi
else
    log_warning "⚠️  Configuration file not found: $CONFIG_FILE"
fi

# Step 9: Create verification script
log_info "📝 Creating verification script..."

cat > verify_flash_attention.py << 'EOF'
#!/usr/bin/env python3
"""
Flash Attention Verification Script for Ubuntu 22.04.5 LTS
"""

import sys
import torch
import traceback

def check_flash_attention():
    """Comprehensive Flash Attention verification"""
    print("🔍 Flash Attention Verification")
    print("=" * 50)
    
    # Basic import test
    try:
        import flash_attn
        print(f"✅ Flash Attention imported successfully (v{flash_attn.__version__})")
    except ImportError as e:
        print(f"❌ Flash Attention import failed: {e}")
        return False
    
    # Check PyTorch compatibility
    print(f"🐍 Python version: {sys.version}")
    print(f"🔥 PyTorch version: {torch.__version__}")
    print(f"🎮 CUDA available: {torch.cuda.is_available()}")
    
    if torch.cuda.is_available():
        print(f"📱 GPU count: {torch.cuda.device_count()}")
        for i in range(torch.cuda.device_count()):
            gpu_name = torch.cuda.get_device_name(i)
            gpu_memory = torch.cuda.get_device_properties(i).total_memory / 1e9
            compute_cap = torch.cuda.get_device_capability(i)
            print(f"   GPU {i}: {gpu_name} ({gpu_memory:.1f}GB, Compute {compute_cap[0]}.{compute_cap[1]})")
    
    # Functional test
    if torch.cuda.is_available():
        try:
            from flash_attn import flash_attn_func
            
            # Test with small tensors
            device = torch.device('cuda')
            batch_size, seqlen, num_heads, head_dim = 1, 64, 4, 32
            
            q = torch.randn(batch_size, seqlen, num_heads, head_dim, device=device, dtype=torch.float16)
            k = torch.randn(batch_size, seqlen, num_heads, head_dim, device=device, dtype=torch.float16)
            v = torch.randn(batch_size, seqlen, num_heads, head_dim, device=device, dtype=torch.float16)
            
            out = flash_attn_func(q, k, v)
            print("✅ Flash Attention functional test passed")
            print(f"   Input shape: {q.shape}")
            print(f"   Output shape: {out.shape}")
            
            return True
            
        except Exception as e:
            print(f"❌ Flash Attention functional test failed: {e}")
            print("📋 Full traceback:")
            traceback.print_exc()
            return False
    else:
        print("⚠️  CUDA not available, skipping functional test")
        return True

if __name__ == "__main__":
    success = check_flash_attention()
    sys.exit(0 if success else 1)
EOF

chmod +x verify_flash_attention.py
log_success "✅ Verification script created: verify_flash_attention.py"

# Step 10: Final verification
log_info "🎯 Running final verification..."
python verify_flash_attention.py

if [ $? -eq 0 ]; then
    log_success "🎉 Flash Attention setup completed successfully!"
    echo ""
    echo "Next steps:"
    echo "1. Run your training script: python scripts/train_asthra_llm.py"
    echo "2. Monitor training progress in the logs"
    echo "3. Flash Attention should now work without errors"
    echo ""
    echo "If you encounter issues, run: python verify_flash_attention.py"
else
    log_error "❌ Flash Attention setup failed"
    echo ""
    echo "Troubleshooting:"
    echo "1. Check GPU compute capability: nvidia-smi --query-gpu=compute_cap --format=csv,noheader"
    echo "2. Ensure CUDA 11.8+ is installed: nvcc --version"
    echo "3. Try alternative installation: pip install flash-attn --no-build-isolation"
    echo "4. Consider disabling Flash Attention in training script temporarily"
    echo ""
    echo "For help: https://github.com/Dao-AILab/flash-attention/issues"
    exit 1
fi

log_info "📄 Script completed. Check verify_flash_attention.py for detailed status." 
