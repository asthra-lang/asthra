#!/bin/bash
# Script to disable Weights & Biases monitoring
# Multiple methods to ensure wandb doesn't interrupt training

echo "🚫 Disabling Weights & Biases monitoring..."

# Method 1: Set environment variable
export WANDB_DISABLED=true
export WANDB_MODE=disabled
echo "✅ Set WANDB_DISABLED=true environment variable"

# Method 2: Create wandb disabled file
mkdir -p ~/.config/wandb
echo "disabled" > ~/.config/wandb/settings
echo "✅ Created wandb disabled settings file"

# Method 3: Add to shell profile for permanent disable
SHELL_PROFILE=""
if [ -f ~/.bashrc ]; then
    SHELL_PROFILE=~/.bashrc
elif [ -f ~/.zshrc ]; then
    SHELL_PROFILE=~/.zshrc
elif [ -f ~/.bash_profile ]; then
    SHELL_PROFILE=~/.bash_profile
fi

if [ ! -z "$SHELL_PROFILE" ]; then
    if ! grep -q "WANDB_DISABLED" "$SHELL_PROFILE"; then
        echo "" >> "$SHELL_PROFILE"
        echo "# Disable Weights & Biases monitoring" >> "$SHELL_PROFILE"
        echo "export WANDB_DISABLED=true" >> "$SHELL_PROFILE"
        echo "export WANDB_MODE=disabled" >> "$SHELL_PROFILE"
        echo "✅ Added wandb disable to $SHELL_PROFILE"
    else
        echo "✅ wandb disable already in $SHELL_PROFILE"
    fi
fi

# Method 4: Update all config files
echo "🔧 Updating configuration files..."
for config_file in configs/*.yaml; do
    if [ -f "$config_file" ]; then
        # Check if file contains use_wandb: true
        if grep -q "use_wandb: true" "$config_file"; then
            # Replace with false
            sed -i.bak 's/use_wandb: true/use_wandb: false  # Disabled to avoid interactive prompts/' "$config_file"
            echo "✅ Updated $config_file"
        fi
    fi
done

echo ""
echo "🎉 Weights & Biases monitoring is now disabled!"
echo ""
echo "📋 What was done:"
echo "   • Set WANDB_DISABLED=true environment variable"
echo "   • Created ~/.config/wandb/settings with 'disabled'"
echo "   • Added permanent disable to shell profile"
echo "   • Updated all config files to use_wandb: false"
echo ""
echo "🚀 You can now run training without wandb interruptions!"
echo ""
echo "💡 To re-enable wandb later:"
echo "   • Set use_wandb: true in your config file"
echo "   • Remove WANDB_DISABLED from environment"
echo "   • Run: wandb login" 
