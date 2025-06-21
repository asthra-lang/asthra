#!/bin/bash
# RHEL/CentOS/Rocky Linux Setup Script for Asthra LLM Training
# Installs CUDA, system dependencies, and Python environment

set -e  # Exit on any error

echo "🐧 Asthra LLM RHEL/Linux Setup"
echo "=============================="

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

# Detect Linux distribution
detect_distro() {
    if [ -f /etc/redhat-release ]; then
        DISTRO=$(cat /etc/redhat-release)
        if [[ $DISTRO == *"Red Hat"* ]]; then
            OS="rhel"
            PKG_MGR="yum"
        elif [[ $DISTRO == *"CentOS"* ]]; then
            OS="centos"
            PKG_MGR="yum"
        elif [[ $DISTRO == *"Rocky"* ]]; then
            OS="rocky"
            PKG_MGR="dnf"
        elif [[ $DISTRO == *"AlmaLinux"* ]]; then
            OS="alma"
            PKG_MGR="dnf"
        else
            OS="rhel"
            PKG_MGR="yum"
        fi
    elif [ -f /etc/os-release ]; then
        . /etc/os-release
        if [[ $ID == "ubuntu" ]]; then
            OS="ubuntu"
            PKG_MGR="apt"
        elif [[ $ID == "debian" ]]; then
            OS="debian"
            PKG_MGR="apt"
        else
            OS="rhel"
            PKG_MGR="yum"
        fi
    else
        OS="unknown"
        PKG_MGR="yum"
    fi
    
    print_info "Detected OS: $OS, Package Manager: $PKG_MGR"
}

# Check if running as root
check_root() {
    if [ "$EUID" -ne 0 ]; then
        print_error "This script needs to be run as root (use sudo)"
        print_info "Usage: sudo bash setup_rhel.sh"
        exit 1
    fi
}

# Install system dependencies
install_system_deps() {
    print_info "Installing system dependencies..."
    
    if [[ $PKG_MGR == "yum" ]]; then
        # RHEL/CentOS 7-8
        yum update -y
        yum groupinstall -y "Development Tools"
        yum install -y epel-release
        yum install -y python3 python3-pip python3-devel
        yum install -y git curl wget htop screen tmux
        yum install -y gcc gcc-c++ make cmake
        yum install -y kernel-devel kernel-headers
        yum install -y dkms
        
    elif [[ $PKG_MGR == "dnf" ]]; then
        # RHEL/Rocky/Alma 9+
        dnf update -y
        dnf groupinstall -y "Development Tools"
        dnf install -y epel-release
        dnf install -y python3 python3-pip python3-devel
        dnf install -y git curl wget htop screen tmux
        dnf install -y gcc gcc-c++ make cmake
        dnf install -y kernel-devel kernel-headers
        dnf install -y dkms
        
    elif [[ $PKG_MGR == "apt" ]]; then
        # Ubuntu/Debian
        apt update
        apt install -y build-essential
        apt install -y python3 python3-pip python3-dev python3-venv
        apt install -y git curl wget htop screen tmux
        apt install -y gcc g++ make cmake
        apt install -y linux-headers-$(uname -r)
        apt install -y dkms
    fi
    
    print_status "System dependencies installed"
}

# Install NVIDIA drivers and CUDA
install_nvidia_cuda() {
    print_info "Installing NVIDIA drivers and CUDA..."
    
    # Check if NVIDIA GPU is present
    if ! lspci | grep -i nvidia &> /dev/null; then
        print_warning "No NVIDIA GPU detected. Skipping NVIDIA driver installation."
        return 0
    fi
    
    # Check if drivers are already installed
    if command -v nvidia-smi &> /dev/null; then
        print_info "NVIDIA drivers already installed:"
        nvidia-smi --query-gpu=name,driver_version --format=csv,noheader
        
        # Check CUDA
        if command -v nvcc &> /dev/null; then
            print_status "CUDA already installed: $(nvcc --version | grep release)"
            return 0
        fi
    fi
    
    if [[ $PKG_MGR == "yum" || $PKG_MGR == "dnf" ]]; then
        # RHEL/CentOS NVIDIA setup
        print_info "Setting up NVIDIA repository..."
        
        # Add NVIDIA repository
        $PKG_MGR-config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
        
        # Install NVIDIA drivers
        print_info "Installing NVIDIA drivers..."
        $PKG_MGR install -y nvidia-driver nvidia-settings
        
        # Install CUDA toolkit
        print_info "Installing CUDA toolkit..."
        $PKG_MGR install -y cuda-toolkit-12-1
        
        # Add CUDA to PATH
        echo 'export PATH=/usr/local/cuda/bin:$PATH' >> /etc/profile.d/cuda.sh
        echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> /etc/profile.d/cuda.sh
        
    elif [[ $PKG_MGR == "apt" ]]; then
        # Ubuntu NVIDIA setup
        print_info "Installing NVIDIA drivers for Ubuntu..."
        
        # Add NVIDIA repository
        wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.0-1_all.deb
        dpkg -i cuda-keyring_1.0-1_all.deb
        apt update
        
        # Install NVIDIA drivers and CUDA
        apt install -y nvidia-driver-535 cuda-toolkit-12-1
    fi
    
    print_status "NVIDIA drivers and CUDA installed"
    print_warning "REBOOT REQUIRED! Please reboot and run this script again."
}

# Setup Python environment
setup_python_env() {
    print_info "Setting up Python environment for regular user..."
    
    # Get the original user (since we're running as sudo)
    ORIGINAL_USER=${SUDO_USER:-$USER}
    ORIGINAL_HOME=$(eval echo ~$ORIGINAL_USER)
    
    # Switch to original user for Python setup
    sudo -u $ORIGINAL_USER bash << 'EOF'
    cd ~
    
    # Create virtual environment
    python3 -m venv asthra_env
    source asthra_env/bin/activate
    
    # Upgrade pip
    pip install --upgrade pip setuptools wheel
    
    # Add to bashrc
    echo 'export PATH="$HOME/asthra_env/bin:$PATH"' >> ~/.bashrc
    echo 'source $HOME/asthra_env/bin/activate' >> ~/.bashrc
    
    echo "✅ Python environment created at ~/asthra_env"
EOF
    
    print_status "Python environment setup complete"
}

# Install Python ML dependencies
install_ml_deps() {
    print_info "Installing Python ML dependencies..."
    
    ORIGINAL_USER=${SUDO_USER:-$USER}
    
    sudo -u $ORIGINAL_USER bash << 'EOF'
    cd ~
    source asthra_env/bin/activate
    
    # Install PyTorch with CUDA support
    pip install torch==2.3.0+cu121 torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121
    
    # Install core ML libraries
    pip install transformers>=4.46.0 datasets>=2.20.0 accelerate>=0.34.0
    pip install trl>=0.9.6 peft>=0.12.0 tokenizers>=0.20.0
    
    # Install Linux-specific optimizations
    pip install bitsandbytes>=0.43.0 flash-attn>=2.5.0
    pip install vllm>=0.8.5 deepspeed>=0.14.0
    
    # Install monitoring tools
    pip install wandb tensorboard nvidia-ml-py gpustat
    
    echo "✅ Core ML dependencies installed"
EOF
    
    print_status "ML dependencies installed"
}

# Install additional tools
install_additional_tools() {
    print_info "Installing additional development tools..."
    
    # Install Docker (optional)
    if ! command -v docker &> /dev/null; then
        print_info "Installing Docker..."
        if [[ $PKG_MGR == "yum" || $PKG_MGR == "dnf" ]]; then
            $PKG_MGR install -y docker
            systemctl enable docker
            systemctl start docker
            usermod -aG docker ${SUDO_USER:-$USER}
        elif [[ $PKG_MGR == "apt" ]]; then
            apt install -y docker.io
            systemctl enable docker
            systemctl start docker
            usermod -aG docker ${SUDO_USER:-$USER}
        fi
        print_status "Docker installed"
    fi
    
    # Install htop, nvtop (GPU monitoring)
    if [[ $PKG_MGR == "yum" || $PKG_MGR == "dnf" ]]; then
        $PKG_MGR install -y htop
        # nvtop needs to be compiled from source on RHEL
    elif [[ $PKG_MGR == "apt" ]]; then
        apt install -y htop nvtop
    fi
    
    print_status "Additional tools installed"
}

# Create monitoring scripts
create_monitoring_scripts() {
    print_info "Creating monitoring scripts..."
    
    ORIGINAL_USER=${SUDO_USER:-$USER}
    ORIGINAL_HOME=$(eval echo ~$ORIGINAL_USER)
    
    # GPU monitoring script
    cat > $ORIGINAL_HOME/monitor_gpu.sh << 'EOF'
#!/bin/bash
echo "🖥️  GPU Status:"
nvidia-smi
echo -e "\n📊 GPU Utilization:"
nvidia-smi --query-gpu=name,utilization.gpu,utilization.memory,memory.used,memory.total,temperature.gpu --format=csv
echo -e "\n🔥 GPU Processes:"
nvidia-smi pmon
EOF
    
    # Training monitoring script
    cat > $ORIGINAL_HOME/monitor_training.sh << 'EOF'
#!/bin/bash
echo "🏃 Training Status:"
ps aux | grep python | grep -E "(train|asthra)" | grep -v grep

echo -e "\n📊 Latest Training Logs:"
find ~/asthra-lang/llmfinetuning/outputs/logs/ -name "*.log" -exec tail -10 {} \; 2>/dev/null || echo "No logs found"

echo -e "\n💾 Model Outputs:"
ls -la ~/asthra-lang/llmfinetuning/outputs/asthra-*/ 2>/dev/null || echo "No model outputs yet"

echo -e "\n🖥️  System Resources:"
htop -n 1 | head -20
EOF
    
    chmod +x $ORIGINAL_HOME/monitor_gpu.sh
    chmod +x $ORIGINAL_HOME/monitor_training.sh
    chown $ORIGINAL_USER:$ORIGINAL_USER $ORIGINAL_HOME/monitor_*.sh
    
    print_status "Monitoring scripts created"
}

# Main installation function
main() {
    print_info "Starting RHEL/Linux setup for Asthra LLM training..."
    echo "Platform: $(uname -a)"
    echo ""
    
    detect_distro
    check_root
    install_system_deps
    install_nvidia_cuda
    
    # Check if reboot is needed
    if [ ! -f /tmp/asthra_reboot_done ]; then
        if ! command -v nvidia-smi &> /dev/null; then
            print_warning "REBOOT REQUIRED for NVIDIA drivers!"
            print_info "After reboot, run: sudo bash setup_rhel.sh --continue"
            touch /tmp/asthra_reboot_needed
            exit 0
        fi
    fi
    
    setup_python_env
    install_ml_deps
    install_additional_tools
    create_monitoring_scripts
    
    print_status "RHEL/Linux setup completed successfully! 🎉"
    
    echo ""
    echo "📋 Next Steps:"
    echo "=============="
    echo ""
    echo "1. 🔄 Source the environment:"
    echo "   source ~/.bashrc"
    echo "   # Or log out and log back in"
    echo ""
    echo "2. 📥 Clone Asthra repository:"
    echo "   git clone https://github.com/your-username/asthra-lang.git"
    echo "   cd asthra-lang/llmfinetuning"
    echo ""
    echo "3. 📦 Install Asthra-specific requirements:"
    echo "   pip install -r requirements-linux.txt"
    echo ""
    echo "4. 🏃 Start training:"
    echo "   python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml"
    echo ""
    echo "5. 📊 Monitor training:"
    echo "   ~/monitor_training.sh"
    echo "   ~/monitor_gpu.sh"
    echo ""
    echo "💡 Pro Tips:"
    echo "  • Use screen/tmux for long training sessions"
    echo "  • Monitor GPU temperature and utilization"
    echo "  • Check system logs if issues occur: journalctl -xe"
    echo ""
    echo "🚨 If you encounter issues:"
    echo "  • Verify CUDA: nvcc --version"
    echo "  • Check GPU: nvidia-smi"
    echo "  • Test PyTorch GPU: python -c 'import torch; print(torch.cuda.is_available())'"
}

# Handle command line arguments
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "RHEL/CentOS/Rocky Linux Setup for Asthra LLM Training"
    echo ""
    echo "Usage: sudo $0 [--continue]"
    echo ""
    echo "Options:"
    echo "  --continue    Continue setup after reboot (internal use)"
    echo "  --help, -h    Show this help message"
    echo ""
    echo "This script will:"
    echo "  • Install system dependencies (Development Tools, Python, Git)"
    echo "  • Install NVIDIA drivers and CUDA toolkit"
    echo "  • Setup Python virtual environment"
    echo "  • Install ML libraries (PyTorch, Transformers, etc.)"
    echo "  • Create monitoring scripts"
    echo "  • Configure environment for GPU training"
    echo ""
    echo "Requirements:"
    echo "  • RHEL/CentOS/Rocky Linux 8+ (or Ubuntu/Debian)"
    echo "  • NVIDIA GPU (for GPU acceleration)"
    echo "  • Internet connection"
    echo "  • Root access (sudo)"
    exit 0
fi

if [ "$1" = "--continue" ]; then
    touch /tmp/asthra_reboot_done
fi

# Run main installation
main 
