#!/bin/bash

# Quick Flash Attention Fix for Ubuntu 22.04.5 LTS
# This script tries the most common solutions in order

echo "🚀 Quick Flash Attention Fix for Ubuntu 22.04.5 LTS"
echo "=================================================="

# Step 0: Install build dependencies
echo "🔧 Installing build dependencies..."
pip install --upgrade pip setuptools wheel ninja

# Step 1: Clean install from source (most reliable)
echo "📦 Method 1: Installing from source..."
pip uninstall -y flash-attn 2>/dev/null || true
export FLASH_ATTENTION_FORCE_BUILD=TRUE
export MAX_JOBS=4
pip install flash-attn --no-build-isolation

# Test installation
if python -c "import flash_attn; print('✅ Flash Attention installed successfully')" 2>/dev/null; then
    echo "🎉 Success! Flash Attention is working."
    exit 0
fi

# Method 2: Pre-built wheel
echo "📦 Method 2: Trying pre-built wheel..."
pip install flash-attn --find-links https://github.com/Dao-AILab/flash-attention/releases

if python -c "import flash_attn; print('✅ Flash Attention installed successfully')" 2>/dev/null; then
    echo "🎉 Success! Flash Attention is working."
    exit 0
fi

# Method 3: Alternative source installation
echo "📦 Method 3: Alternative source installation..."
pip install git+https://github.com/Dao-AILab/flash-attention.git

if python -c "import flash_attn; print('✅ Flash Attention installed successfully')" 2>/dev/null; then
    echo "🎉 Success! Flash Attention is working."
    exit 0
fi

echo "❌ All quick fixes failed. Run the comprehensive fix script:"
echo "bash scripts/fix_flash_attention_ubuntu.sh" 
