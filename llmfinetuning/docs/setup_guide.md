# Asthra LLM Fine-tuning Setup Guide

**Version:** 1.0  
**Created:** January 2025  
**Target Model:** Devstral-Small-2505 (24B)

## 🚀 Quick Start

### Prerequisites

- **Python 3.8+** (recommended: 3.11)
- **CUDA-compatible GPU** (recommended: 24GB+ VRAM for 24B model)
- **Git and Git LFS** for model downloads
- **Asthra compiler** built and functional (for evaluation)

### 1. Environment Setup

```bash
# Navigate to llmfinetuning directory
cd llmfinetuning/

# Run automated setup
bash scripts/setup_environment.sh

# Activate environment (alternative to sourcing manually)
source activate.sh
```

The setup script will:
- ✅ Create Python virtual environment
- ✅ Install PyTorch with CUDA support
- ✅ Install all dependencies
- ✅ Check GPU availability
- ✅ Create directory structure
- ✅ Set up environment variables

### 2. Hugging Face Authentication

```bash
# Install Hugging Face CLI (if not already installed)
pip install huggingface_hub[cli]

# Login to access Devstral-Small-2505
huggingface-cli login
```

Enter your Hugging Face token when prompted. This is required to download Devstral-Small-2505.

### 3. Data Preparation

```bash
# Prepare training data from Asthra repository
python scripts/prepare_data.py --asthra-repo .. --verbose

# Check data quality
ls -la data/processed/
cat data/quality/quality_report.json
```

Expected output:
- `instruction_tuning.jsonl` - Primary training dataset
- `code_completion.jsonl` - Code completion examples
- `explanation.jsonl` - Code explanation pairs
- `validation.jsonl` - Validation dataset

### 4. Start Training

```bash
# Start fine-tuning with default configuration
python scripts/train_asthra_llm.py --config configs/devstral_config.yaml --verbose

# OR start with debug mode (small dataset, fast iteration)
python scripts/train_asthra_llm.py --config configs/devstral_config.yaml --debug
```

### 5. Monitor Training

Training progress can be monitored through:

- **Console output** - Real-time training logs
- **TensorBoard** - `tensorboard --logdir outputs/logs/`
- **Weights & Biases** - (if configured) https://wandb.ai/

### 6. Evaluate Model

```bash
# Evaluate trained model
python scripts/evaluate_model.py \
    --model-path outputs/models/final \
    --asthra-compiler ../build/asthra \
    --verbose

# View evaluation results
cat outputs/evaluation/evaluation_report.md
```

## 📋 Detailed Configuration

### Hardware Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **GPU** | 16GB VRAM | 24GB+ VRAM |
| **RAM** | 32GB | 64GB+ |
| **Storage** | 100GB free | 500GB+ SSD |
| **CPU** | 8 cores | 16+ cores |

### Training Configuration

The main configuration is in `configs/devstral_config.yaml`. Key sections:

#### Model Settings
```yaml
model:
  name: "mistralai/Devstral-Small-2505"
  max_length: 128000  # 128K context window
  quantization:
    load_in_4bit: true  # Reduces memory usage
```

#### Training Parameters
```yaml
training:
  per_device_train_batch_size: 1
  gradient_accumulation_steps: 16  # Effective batch size = 16
  learning_rate: 5e-5
  num_train_epochs: 3
```

#### LoRA Configuration
```yaml
lora:
  rank: 64
  alpha: 32
  target_modules: ["q_proj", "k_proj", "v_proj", "o_proj", "gate_proj", "up_proj", "down_proj"]
```

### Memory Optimization

For systems with limited GPU memory:

1. **Increase quantization**:
   ```yaml
   quantization:
     load_in_4bit: true
     bnb_4bit_compute_dtype: "bfloat16"
   ```

2. **Reduce batch size**:
   ```yaml
   training:
     per_device_train_batch_size: 1
     gradient_accumulation_steps: 32
   ```

3. **Enable gradient checkpointing**:
   ```yaml
   hardware:
     gradient_checkpointing: true
   ```

4. **Use DeepSpeed** (advanced):
   ```bash
   pip install deepspeed
   # Update config to use DeepSpeed ZeRO
   ```

## 🛠️ Troubleshooting

### Common Issues

#### 1. CUDA Out of Memory
```
RuntimeError: CUDA out of memory
```

**Solutions:**
- Reduce `per_device_train_batch_size` to 1
- Increase `gradient_accumulation_steps`
- Enable 4-bit quantization
- Use gradient checkpointing

#### 2. Model Download Issues
```
OSError: mistralai/Devstral-Small-2505 is not a local folder
```

**Solutions:**
- Ensure Hugging Face login: `huggingface-cli whoami`
- Check internet connection
- Try manual download: `huggingface-cli download mistralai/Devstral-Small-2505`

#### 3. Compilation Errors During Evaluation
```
Asthra compiler not found
```

**Solutions:**
- Build Asthra compiler: `cd .. && make build`
- Update compiler path in config: `asthra_compiler_path: "../build/asthra"`
- Check executable permissions: `chmod +x ../build/asthra`

#### 4. Data Preparation Failures
```
FileNotFoundError: [Errno 2] No such file or directory: 'grammar.txt'
```

**Solutions:**
- Ensure you're in the correct directory: `cd llmfinetuning/`
- Check Asthra repo path: `python scripts/prepare_data.py --asthra-repo ../`
- Verify Asthra repo structure

### Performance Optimization

#### 1. Enable Flash Attention
```bash
pip install flash-attn --no-build-isolation
```

#### 2. Use Mixed Precision Training
```yaml
training:
  bf16: true  # Better than fp16 for large models
  fp16: false
```

#### 3. Optimize Data Loading
```yaml
data:
  preprocessing_num_workers: 8
  dataloader_num_workers: 4
```

### Monitoring and Debugging

#### Enable Verbose Logging
```bash
python scripts/train_asthra_llm.py --config configs/devstral_config.yaml --verbose
```

#### Debug Mode (Fast Iteration)
```bash
python scripts/train_asthra_llm.py --config configs/devstral_config.yaml --debug
```

#### Check GPU Usage
```bash
# Monitor GPU memory and utilization
watch -n 1 nvidia-smi

# Monitor during training
nvidia-smi dmon
```

## 🔧 Advanced Configuration

### Multi-GPU Training

For systems with multiple GPUs:

```yaml
hardware:
  device_map: "auto"  # Automatic device mapping
  
training:
  ddp_find_unused_parameters: false
  dataloader_num_workers: 8
```

### Custom Data Sources

To add custom training data:

1. Create JSONL files in `data/processed/`
2. Update `configs/devstral_config.yaml`:
   ```yaml
   data:
     dataset_path: "./data/processed/"
     custom_datasets:
       - "custom_instructions.jsonl"
       - "custom_completions.jsonl"
   ```

### Weights & Biases Integration

1. **Install and login**:
   ```bash
   pip install wandb
   wandb login
   ```

2. **Update config**:
   ```yaml
   monitoring:
     use_wandb: true
     wandb_project: "asthra-llm-finetuning"
     wandb_entity: "your-username"
   ```

## 📈 Success Metrics

### Target Performance
- **Compilation Success Rate**: 90%+
- **Semantic Correctness**: 85%+
- **Idiomatic Quality**: 80%+
- **Complex Feature Coverage**: 70%+

### Training Indicators
- **Loss decreasing steadily** over epochs
- **Perplexity improving** on validation set
- **No overfitting** (train/val loss gap reasonable)
- **GPU utilization 80%+** during training

## 🚀 Next Steps

After successful fine-tuning:

1. **Export to GGUF**: `python scripts/export_to_ollama.py`
2. **Deploy locally**: Install with Ollama
3. **Integration**: Use in development workflow
4. **Continuous improvement**: Collect feedback data for next iteration

## 📞 Getting Help

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: Complete guides in `docs/` directory
- **Logs**: Check `outputs/logs/` for detailed information
- **Community**: Asthra programming language community

---

**Last Updated:** January 2025  
**Version:** 1.0 
