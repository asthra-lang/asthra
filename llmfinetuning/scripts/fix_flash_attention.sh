#!/bin/bash

# Flash Attention Fix Script for Linux
# Diagnoses and fixes Flash Attention installation issues
# Version: 1.0
# Compatible with: RHEL, Ubuntu, CentOS, Rocky Linux

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}ℹ️  INFO:${NC} $1"
}

log_success() {
    echo -e "${GREEN}✅ SUCCESS:${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}⚠️  WARNING:${NC} $1"
}

log_error() {
    echo -e "${RED}❌ ERROR:${NC} $1"
}

# Check if running as root
check_sudo() {
    if [[ $EUID -eq 0 ]]; then
        log_warning "Running as root. This is not recommended for Python package installation."
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

# Detect Linux distribution
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$NAME
        VER=$VERSION_ID
    else
        log_error "Cannot detect Linux distribution"
        exit 1
    fi
    log_info "Detected OS: $OS $VER"
}

# Check CUDA installation
check_cuda() {
    log_info "Checking CUDA installation..."
    
    if command -v nvidia-smi &> /dev/null; then
        CUDA_VERSION=$(nvidia-smi | grep "CUDA Version" | sed 's/.*CUDA Version: \([0-9.]*\).*/\1/')
        log_success "NVIDIA driver detected - CUDA version: $CUDA_VERSION"
    else
        log_error "NVIDIA driver not found. Please install NVIDIA drivers first."
        exit 1
    fi
    
    if command -v nvcc &> /dev/null; then
        NVCC_VERSION=$(nvcc --version | grep "release" | sed 's/.*release \([0-9.]*\).*/\1/')
        log_success "NVCC detected - version: $NVCC_VERSION"
    else
        log_warning "NVCC not found. Flash Attention might need CUDA toolkit."
        log_info "To install CUDA toolkit on RHEL/CentOS:"
        log_info "  sudo yum install cuda-toolkit"
        log_info "To install CUDA toolkit on Ubuntu:"
        log_info "  sudo apt install nvidia-cuda-toolkit"
    fi
}

# Check Python environment
check_python() {
    log_info "Checking Python environment..."
    
    PYTHON_VERSION=$(python3 --version | sed 's/Python //')
    log_info "Python version: $PYTHON_VERSION"
    
    if python3 -c "import torch; print(f'PyTorch version: {torch.__version__}'); print(f'CUDA available: {torch.cuda.is_available()}')" 2>/dev/null; then
        log_success "PyTorch is properly installed with CUDA support"
    else
        log_error "PyTorch is not properly installed or CUDA support is missing"
        exit 1
    fi
}

# Check current Flash Attention status
check_flash_attention() {
    log_info "Checking current Flash Attention installation..."
    
    if python3 -c "import flash_attn; print(f'Flash Attention version: {flash_attn.__version__}')" 2>/dev/null; then
        log_success "Flash Attention is already installed"
        FLASH_ATTN_INSTALLED=true
    else
        log_warning "Flash Attention is not installed or not working"
        FLASH_ATTN_INSTALLED=false
    fi
}

# Install system dependencies
install_system_deps() {
    log_info "Installing system dependencies..."
    
    case "$OS" in
        *"Red Hat"*|*"CentOS"*|*"Rocky"*|*"Fedora"*)
            log_info "Installing RHEL/CentOS dependencies..."
            sudo yum update -y
            sudo yum groupinstall -y "Development Tools"
            sudo yum install -y python3-devel gcc gcc-c++ cmake ninja-build
            ;;
        *"Ubuntu"*|*"Debian"*)
            log_info "Installing Ubuntu/Debian dependencies..."
            sudo apt update
            sudo apt install -y build-essential python3-dev cmake ninja-build
            ;;
        *)
            log_warning "Unknown distribution. Please install build tools manually."
            ;;
    esac
}

# Uninstall problematic Flash Attention
uninstall_flash_attention() {
    log_info "Removing existing Flash Attention installation..."
    
    # Try multiple uninstall methods
    pip uninstall -y flash-attn || true
    pip uninstall -y flash_attn || true
    
    # Clean pip cache
    pip cache purge || true
    
    # Remove from site-packages if still there
    python3 -c "import site; import shutil; import os; [shutil.rmtree(os.path.join(sp, 'flash_attn'), ignore_errors=True) for sp in site.getsitepackages()]" || true
}

# Install Flash Attention with proper compilation
install_flash_attention() {
    log_info "Installing Flash Attention from source..."
    
    # Set compilation flags
    export FLASH_ATTENTION_FORCE_BUILD=TRUE
    export FLASH_ATTENTION_FORCE_CXX11_ABI=TRUE
    export MAX_JOBS=$(nproc)
    
    # Install with specific options
    log_info "This may take 10-20 minutes to compile..."
    
    # Try precompiled wheel first
    if pip install flash-attn>=2.5.0 --no-cache-dir --index-url https://download.pytorch.org/whl/cu121; then
        log_success "Installed Flash Attention from precompiled wheel"
    else
        log_info "Precompiled wheel failed, compiling from source..."
        # Compile from source as fallback
        pip install flash-attn>=2.5.0 --no-cache-dir --no-build-isolation
    fi
}

# Verify installation
verify_installation() {
    log_info "Verifying Flash Attention installation..."
    
    if python3 -c "
import torch
import flash_attn
print(f'✅ Flash Attention version: {flash_attn.__version__}')
print(f'✅ PyTorch version: {torch.__version__}')
print(f'✅ CUDA available: {torch.cuda.is_available()}')
print(f'✅ CUDA device count: {torch.cuda.device_count()}')

# Test basic functionality
try:
    from flash_attn import flash_attn_func
    print('✅ Flash Attention function import successful')
except ImportError as e:
    print(f'❌ Flash Attention function import failed: {e}')
    exit(1)
"; then
        log_success "Flash Attention verification passed!"
    else
        log_error "Flash Attention verification failed!"
        exit 1
    fi
}

# Fix training script to handle Flash Attention gracefully
fix_training_script() {
    log_info "Updating training script to handle Flash Attention gracefully..."
    
    TRAINING_SCRIPT="scripts/train_asthra_llm.py"
    
    if [ -f "$TRAINING_SCRIPT" ]; then
        # Create backup
        cp "$TRAINING_SCRIPT" "${TRAINING_SCRIPT}.backup.$(date +%Y%m%d_%H%M%S)"
        
        # Update the script to handle Flash Attention more gracefully
        python3 << 'EOF'
import re

# Read the training script
with open('scripts/train_asthra_llm.py', 'r') as f:
    content = f.read()

# Replace the flash attention line with a more robust version
old_pattern = r'attn_implementation="flash_attention_2" if torch\.cuda\.is_available\(\) else None'
new_pattern = '''# Try to use Flash Attention 2 if available
attn_implementation = None
if torch.cuda.is_available():
    try:
        import flash_attn
        attn_implementation = "flash_attention_2"
        print("✅ Using Flash Attention 2 for improved performance")
    except ImportError:
        print("⚠️  Flash Attention 2 not available, using standard attention")
        attn_implementation = None'''

if re.search(old_pattern, content):
    content = re.sub(old_pattern, new_pattern, content)
    
    # Write back the updated content
    with open('scripts/train_asthra_llm.py', 'w') as f:
        f.write(content)
    
    print("✅ Training script updated to handle Flash Attention gracefully")
else:
    print("ℹ️  Training script pattern not found, no changes made")
EOF
        
        log_success "Training script updated"
    else
        log_warning "Training script not found at $TRAINING_SCRIPT"
    fi
}

# Alternative: Disable Flash Attention in config
create_no_flash_config() {
    log_info "Creating alternative configuration without Flash Attention..."
    
    CONFIG_FILE="configs/gemma3_config_no_flash.yaml"
    
    if [ -f "configs/gemma3_config.yaml" ]; then
        cp "configs/gemma3_config.yaml" "$CONFIG_FILE"
        
        # Add Flash Attention disable setting
        cat >> "$CONFIG_FILE" << 'EOF'

# Flash Attention settings (added by fix script)
flash_attention:
  enabled: false
  reason: "Disabled due to installation issues"

comments:
  flash_attention_note: |
    Flash Attention has been disabled due to installation issues.
    This will use standard attention mechanism which is slower but more compatible.
    To re-enable Flash Attention:
    1. Ensure CUDA toolkit is installed
    2. Run: pip install flash-attn>=2.5.0 --no-cache-dir
    3. Set flash_attention.enabled: true
EOF
        
        log_success "Created no-flash configuration: $CONFIG_FILE"
        log_info "You can use this config with: python scripts/train_asthra_llm.py --config $CONFIG_FILE"
    fi
}

# Test with a simple training example
test_training() {
    log_info "Testing training script with Flash Attention..."
    
    # Create a minimal test
    python3 << 'EOF'
import torch
from transformers import AutoTokenizer, AutoModelForCausalLM

try:
    # Test model loading with Flash Attention
    model_name = "google/gemma-2-2b"  # Smaller model for testing
    print("Loading tokenizer...")
    tokenizer = AutoTokenizer.from_pretrained(model_name, trust_remote_code=True)
    
    print("Loading model...")
    try:
        # Try with Flash Attention first
        import flash_attn
        model = AutoModelForCausalLM.from_pretrained(
            model_name,
            torch_dtype=torch.bfloat16,
            device_map="auto",
            attn_implementation="flash_attention_2",
            trust_remote_code=True
        )
        print("✅ Model loaded successfully with Flash Attention 2")
    except Exception as e:
        print(f"⚠️  Flash Attention failed: {e}")
        print("Trying standard attention...")
        model = AutoModelForCausalLM.from_pretrained(
            model_name,
            torch_dtype=torch.bfloat16,
            device_map="auto",
            trust_remote_code=True
        )
        print("✅ Model loaded successfully with standard attention")
    
    print("✅ Test completed successfully")
    
except Exception as e:
    print(f"❌ Test failed: {e}")
    exit(1)
EOF
}

# Main execution
main() {
    echo "🔧 Flash Attention Fix Script for Linux"
    echo "======================================"
    
    check_sudo
    detect_os
    check_cuda
    check_python
    check_flash_attention
    
    if [ "$FLASH_ATTN_INSTALLED" = true ]; then
        log_info "Flash Attention is already installed. Testing functionality..."
        verify_installation
        log_success "Flash Attention is working correctly!"
        exit 0
    fi
    
    log_info "Flash Attention needs to be installed. Proceeding with installation..."
    
    install_system_deps
    uninstall_flash_attention
    install_flash_attention
    verify_installation
    fix_training_script
    create_no_flash_config
    
    log_success "Flash Attention setup completed!"
    log_info "You can now run training with Flash Attention support."
    log_info "If you encounter issues, use the no-flash config: configs/gemma3_config_no_flash.yaml"
}

# Handle script arguments
case "${1:-}" in
    --help|-h)
        echo "Flash Attention Fix Script"
        echo ""
        echo "Usage: $0 [OPTIONS]"
        echo ""
        echo "Options:"
        echo "  --help, -h     Show this help message"
        echo "  --test         Test Flash Attention functionality only"
        echo "  --verify       Verify existing installation only"
        echo "  --force        Force reinstallation even if already installed"
        echo ""
        exit 0
        ;;
    --test)
        test_training
        exit 0
        ;;
    --verify)
        check_python
        verify_installation
        exit 0
        ;;
    --force)
        FLASH_ATTN_INSTALLED=false
        main
        ;;
    *)
        main
        ;;
esac 
