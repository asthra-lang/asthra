#!/bin/bash
# Quick Cloud Setup Script for Asthra LLM Training
# Designed for Vast.ai, RunPod, Lambda Labs, and other cloud GPU providers

set -e  # Exit on any error

echo "🚀 Asthra LLM Cloud Training Quick Setup"
echo "========================================"

# Function to print colored output
print_status() {
    echo -e "\e[32m✅ $1\e[0m"
}

print_warning() {
    echo -e "\e[33m⚠️  $1\e[0m"
}

print_error() {
    echo -e "\e[31m❌ $1\e[0m"
}

print_info() {
    echo -e "\e[34mℹ️  $1\e[0m"
}

# Check if running on a GPU instance
check_gpu() {
    print_info "Checking GPU availability..."
    if command -v nvidia-smi &> /dev/null; then
        nvidia-smi --query-gpu=name,memory.total --format=csv,noheader
        print_status "GPU detected successfully"
    else
        print_error "No GPU detected. This script requires a GPU instance."
        exit 1
    fi
}

# Update system packages
update_system() {
    print_info "Updating system packages..."
    apt update -qq
    apt install -y git python3-pip python3-venv curl wget htop screen tmux
    print_status "System packages updated"
}

# Setup Python environment
setup_python() {
    print_info "Setting up Python environment..."
    
    # Create virtual environment
    python3 -m venv asthra_env
    source asthra_env/bin/activate
    
    # Upgrade pip
    pip install --upgrade pip
    
    print_status "Python environment ready"
    echo "export PATH=\"$PWD/asthra_env/bin:\$PATH\"" >> ~/.bashrc
}

# Clone repository
clone_repo() {
    print_info "Cloning Asthra repository..."
    
    # Default repository - user can modify this
    REPO_URL=${1:-"https://github.com/your-username/asthra-lang.git"}
    
    if [ ! -d "asthra-lang" ]; then
        git clone $REPO_URL
        print_status "Repository cloned"
    else
        print_warning "Repository already exists, pulling latest changes..."
        cd asthra-lang && git pull && cd ..
    fi
}

# Install dependencies
install_dependencies() {
    print_info "Installing Python dependencies..."
    
    cd asthra-lang/llmfinetuning
    source ../../asthra_env/bin/activate
    
    # Install core dependencies
    pip install torch transformers datasets accelerate trl peft tokenizers
    pip install pyyaml python-dotenv psutil rich click
    pip install tensorboard  # For monitoring
    
    # Install requirements if file exists
    if [ -f "requirements.txt" ]; then
        pip install -r requirements.txt
    fi
    
    print_status "Dependencies installed"
}

# Configure wandb (disable for cloud)
configure_wandb() {
    print_info "Configuring Weights & Biases (disabled for cloud)..."
    
    export WANDB_DISABLED=true
    export WANDB_MODE=disabled
    echo "export WANDB_DISABLED=true" >> ~/.bashrc
    echo "export WANDB_MODE=disabled" >> ~/.bashrc
    
    # Create wandb disabled config
    mkdir -p ~/.config/wandb
    echo "disabled" > ~/.config/wandb/settings
    
    print_status "Wandb disabled for cloud training"
}

# Setup monitoring tools
setup_monitoring() {
    print_info "Setting up monitoring tools..."
    
    # Create monitoring script
    cat > monitor_training.sh << 'EOF'
#!/bin/bash
echo "🖥️  GPU Status:"
nvidia-smi --query-gpu=name,utilization.gpu,memory.used,memory.total --format=csv

echo -e "\n📊 Training Status:"
if [ -f "outputs/logs/*/training.log" ]; then
    tail -10 outputs/logs/*/training.log
else
    echo "No training logs found yet"
fi

echo -e "\n💾 Model Outputs:"
ls -la outputs/asthra-*/ 2>/dev/null || echo "No model outputs yet"

echo -e "\n🔄 Processes:"
ps aux | grep python | grep -v grep
EOF
    
    chmod +x monitor_training.sh
    print_status "Monitoring tools ready"
}

# Prepare training data
prepare_data() {
    print_info "Preparing training data..."
    
    cd asthra-lang/llmfinetuning
    source ../../asthra_env/bin/activate
    
    # Run data preparation if script exists
    if [ -f "scripts/prepare_data.py" ]; then
        python scripts/prepare_data.py --config configs/cloud_optimized_config.yaml
        print_status "Training data prepared"
    else
        print_warning "Data preparation script not found, skipping..."
    fi
}

# Show usage instructions
show_usage() {
    print_status "Setup complete! 🎉"
    echo ""
    echo "📋 Next Steps:"
    echo "=============="
    echo ""
    echo "1. 🏃 Start Training (Fast):"
    echo "   cd asthra-lang/llmfinetuning"
    echo "   source ../../asthra_env/bin/activate"
    echo "   screen -S training"
    echo "   python scripts/train_asthra_llm.py --config configs/cloud_optimized_config.yaml"
    echo "   # Press Ctrl+A+D to detach from screen"
    echo ""
    echo "2. 📊 Monitor Training:"
    echo "   ./monitor_training.sh"
    echo "   # Or reattach to screen: screen -r training"
    echo ""
    echo "3. 💾 Download Results:"
    echo "   # Training will save to: outputs/asthra-gemma-2b-cloud/"
    echo "   # Use scp to download: scp -r user@ip:~/outputs/ ./"
    echo ""
    echo "4. 🛑 Emergency Stop:"
    echo "   screen -r training  # Then Ctrl+C"
    echo "   # Or: pkill -f train_asthra_llm"
    echo ""
    echo "💡 Pro Tips:"
    echo "  • Expected training time: 2-4 hours on RTX 4090"
    echo "  • Expected cost: $1-2 on Vast.ai, $10 on Colab Pro"
    echo "  • Monitor GPU: watch -n 1 nvidia-smi"
    echo "  • Check logs: tail -f outputs/logs/*/training.log"
    echo ""
    echo "📚 Documentation:"
    echo "  • Cloud guide: docs/CLOUD_TRAINING_GUIDE.md"
    echo "  • Server setup: servers/README.md"
    echo "  • Editor config: editors/setup_guide.md"
}

# Main execution
main() {
    echo "Starting Asthra LLM cloud setup..."
    echo "Platform: $(uname -a)"
    echo "Working directory: $(pwd)"
    echo ""
    
    check_gpu
    update_system
    setup_python
    clone_repo $1  # Optional repo URL as first argument
    install_dependencies
    configure_wandb
    setup_monitoring
    prepare_data
    show_usage
    
    print_status "Cloud setup completed successfully! 🚀"
}

# Parse command line arguments
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "Asthra LLM Cloud Training Quick Setup"
    echo ""
    echo "Usage: $0 [repository_url]"
    echo ""
    echo "Arguments:"
    echo "  repository_url    Optional GitHub repository URL"
    echo "                    Default: https://github.com/your-username/asthra-lang.git"
    echo ""
    echo "Examples:"
    echo "  $0"
    echo "  $0 https://github.com/myuser/asthra-lang.git"
    echo ""
    echo "This script will:"
    echo "  • Check GPU availability"
    echo "  • Install system dependencies"
    echo "  • Setup Python environment"
    echo "  • Clone Asthra repository"
    echo "  • Install ML dependencies"
    echo "  • Configure training environment"
    echo "  • Prepare training data"
    echo "  • Provide usage instructions"
    exit 0
fi

# Run main function
main $1 
