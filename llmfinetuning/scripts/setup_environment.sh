#!/bin/bash

# Asthra LLM Fine-tuning Environment Setup
# Version: 1.0
# Created: January 2025
# Purpose: Set up complete training environment for Devstral-Small-2505

set -e  # Exit on any error

echo "🚀 Setting up Asthra LLM Fine-tuning Environment"
echo "================================================"

# Color definitions for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the correct directory
if [ ! -f "requirements.txt" ]; then
    print_error "Please run this script from the llmfinetuning/ directory"
    exit 1
fi

# Step 1: Check Python version
print_status "Checking Python version..."
python_version=$(python3 --version 2>&1 | cut -d" " -f2)
required_version="3.8"

if python3 -c "import sys; exit(0 if sys.version_info >= (3, 8) else 1)"; then
    print_success "Python ${python_version} is compatible"
else
    print_error "Python 3.8+ required. Found: ${python_version}"
    exit 1
fi

# Step 2: Check for CUDA/GPU availability
print_status "Checking GPU availability..."
if command -v nvidia-smi &> /dev/null; then
    gpu_count=$(nvidia-smi --list-gpus | wc -l)
    if [ $gpu_count -gt 0 ]; then
        print_success "Found ${gpu_count} GPU(s)"
        nvidia-smi --query-gpu=name,memory.total --format=csv,noheader,nounits | while read line; do
            print_status "  GPU: $line"
        done
    else
        print_warning "No GPUs detected"
    fi
else
    print_warning "nvidia-smi not found. GPU training may not be available."
fi

# Step 3: Create virtual environment
print_status "Creating Python virtual environment..."
if [ ! -d ".venv" ]; then
    python3 -m venv .venv
    print_success "Virtual environment created"
else
    print_status "Virtual environment already exists"
fi

# Step 4: Activate virtual environment
print_status "Activating virtual environment..."
source .venv/bin/activate

# Step 5: Upgrade pip
print_status "Upgrading pip..."
pip install --upgrade pip

# Step 6: Install PyTorch with CUDA support
print_status "Installing PyTorch with CUDA support..."
if command -v nvidia-smi &> /dev/null; then
    # Install PyTorch with CUDA
    pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
else
    # Install CPU-only PyTorch
    print_warning "Installing CPU-only PyTorch (GPU not detected)"
    pip install torch torchvision torchaudio
fi

# Step 7: Install main requirements
print_status "Installing Python dependencies..."
pip install -r requirements.txt

# Step 8: Create necessary directories
print_status "Creating directory structure..."
directories=(
    "data/raw/grammar"
    "data/raw/spec" 
    "data/raw/examples"
    "data/raw/stdlib"
    "data/raw/tests"
    "data/processed"
    "data/quality"
    "outputs/models"
    "outputs/logs/training"
    "outputs/logs/evaluation"
    "outputs/logs/wandb"
    "outputs/reports"
    "outputs/cache/datasets"
    "outputs/cache/tokenized"
    "outputs/exported/gguf"
    "outputs/exported/ollama"
)

for dir in "${directories[@]}"; do
    mkdir -p "$dir"
done
print_success "Directory structure created"

# Step 9: Download and setup Devstral model (if needed)
print_status "Checking Hugging Face CLI..."
if command -v huggingface-cli &> /dev/null; then
    print_success "Hugging Face CLI available"
    
    # Check if user is logged in
    if huggingface-cli whoami &> /dev/null; then
        print_success "Logged in to Hugging Face Hub"
    else
        print_warning "Please log in to Hugging Face Hub: huggingface-cli login"
        print_warning "This is required to download Devstral-Small-2505"
    fi
else
    print_warning "Hugging Face CLI not found. Install with: pip install huggingface_hub[cli]"
fi

# Step 10: Test environment
print_status "Testing environment setup..."

# Test Python imports
python3 -c "
import torch
import transformers
import datasets
import peft
import trl
print('✅ All core libraries imported successfully')
print(f'PyTorch version: {torch.__version__}')
print(f'Transformers version: {transformers.__version__}')
print(f'CUDA available: {torch.cuda.is_available()}')
if torch.cuda.is_available():
    print(f'CUDA devices: {torch.cuda.device_count()}')
    for i in range(torch.cuda.device_count()):
        print(f'  GPU {i}: {torch.cuda.get_device_name(i)}')
" 2>/dev/null

if [ $? -eq 0 ]; then
    print_success "Environment test passed"
else
    print_error "Environment test failed. Please check the error messages above."
    exit 1
fi

# Step 11: Check Asthra compiler availability  
print_status "Checking Asthra compiler..."
asthra_compiler="../build/asthra"
if [ -f "$asthra_compiler" ]; then
    print_success "Asthra compiler found at $asthra_compiler"
    # Test compiler
    if $asthra_compiler --version &> /dev/null; then
        compiler_version=$($asthra_compiler --version 2>&1 || echo "version unknown")
        print_success "Asthra compiler is functional: $compiler_version"
    else
        print_warning "Asthra compiler found but may not be functional"
    fi
else
    print_warning "Asthra compiler not found at $asthra_compiler"
    print_warning "Compilation testing will be disabled"
    print_warning "Build Asthra first: cd .. && make build"
fi

# Step 12: Create environment variables file
print_status "Creating environment configuration..."
cat > .env << EOF
# Asthra LLM Fine-tuning Environment Configuration
# Generated: $(date)

# Paths
ASTHRA_REPO_PATH=..
ASTHRA_COMPILER_PATH=../build/asthra
DATA_PATH=./data
OUTPUT_PATH=./outputs

# Hardware
CUDA_VISIBLE_DEVICES=0
OMP_NUM_THREADS=8

# Hugging Face
HF_DATASETS_CACHE=./outputs/cache/datasets
TRANSFORMERS_CACHE=./outputs/cache/transformers

# Weights & Biases (optional)
# WANDB_API_KEY=your_wandb_api_key_here
# WANDB_PROJECT=asthra-llm-finetuning

# Training
TOKENIZERS_PARALLELISM=false
EOF

print_success "Environment configuration created in .env"

# Step 13: Final summary
echo
echo "🎉 Environment Setup Complete!"
echo "=============================="
echo
print_success "Next steps:"
echo "  1. Activate environment: source .venv/bin/activate"
echo "  2. Set up Weights & Biases (optional): wandb login"
echo "  3. Prepare training data: python scripts/prepare_data.py"
echo "  4. Start training: python scripts/train_asthra_llm.py --config configs/devstral_config.yaml"
echo
print_status "For detailed guidance, see: docs/setup_guide.md"
echo

# Step 14: Create activation helper script
cat > activate.sh << 'EOF'
#!/bin/bash
# Quick activation script for Asthra LLM fine-tuning environment
source .venv/bin/activate
source .env
echo "🚀 Asthra LLM fine-tuning environment activated!"
echo "Current directory: $(pwd)"
echo "Python: $(which python3)"
echo "PyTorch CUDA: $(python3 -c 'import torch; print(torch.cuda.is_available())')"
EOF

chmod +x activate.sh
print_success "Quick activation script created: ./activate.sh"

print_success "Environment setup completed successfully! 🚀" 
