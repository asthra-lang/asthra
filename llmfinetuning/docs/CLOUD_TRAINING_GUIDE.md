# ☁️ Cloud Training Guide for Asthra LLM

This guide covers the best cloud platforms for training your fine-tuned Asthra model, with cost comparisons and setup instructions.

## 🎯 **Recommended Cloud Platforms**

### **🥇 1. Google Colab Pro/Pro+ (Easiest & Cheapest)**

**Best for:** Quick experiments, learning, budget-conscious training

**Pricing:**
- **Colab Pro:** $10/month - A100 access, 24GB VRAM
- **Colab Pro+:** $50/month - Premium GPUs, longer runtimes
- **Pay As You Go:** $0.50-1.00/hour for premium GPUs

**Pros:**
- No setup required, Jupyter notebook interface
- Pre-installed ML libraries
- Easy to share and collaborate
- Great for Gemma3 4B model (fits in 24GB)

**Cons:**
- Runtime limits (12-24 hours)
- May disconnect during long training
- Limited persistent storage

**Setup Instructions:**
```python
# In Colab notebook
!git clone https://github.com/your-username/asthra-lang.git
%cd asthra-lang/llmfinetuning
!pip install -r requirements.txt
!python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
```

### **🥈 2. Vast.ai (Best Value for Money)**

**Best for:** Cost-effective GPU rental, flexible pricing

**Pricing:**
- **RTX 4090:** $0.20-0.40/hour (24GB VRAM)
- **A100 40GB:** $0.80-1.20/hour
- **A100 80GB:** $1.50-2.50/hour

**Pros:**
- Cheapest GPU rental platform
- Wide variety of hardware options
- Spot pricing for even lower costs
- Full root access

**Cons:**
- Requires more setup
- Instances can be preempted
- Variable reliability

**Setup Instructions:**
```bash
# SSH into your Vast.ai instance
ssh root@your-instance-ip

# Setup environment
git clone https://github.com/your-username/asthra-lang.git
cd asthra-lang/llmfinetuning
pip install -r requirements.txt

# Start training
python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
```

### **🥉 3. RunPod (Balanced Option)**

**Best for:** Reliable GPU instances, good community

**Pricing:**
- **RTX 4090:** $0.34/hour (24GB VRAM)
- **A100 40GB:** $1.14/hour
- **A100 80GB:** $2.29/hour

**Pros:**
- Reliable infrastructure
- Good documentation
- Community templates
- Persistent storage options

**Cons:**
- Slightly more expensive than Vast.ai
- Limited free tier

### **🔥 4. Lambda Labs (Premium Option)**

**Best for:** Professional ML workloads, consistent performance

**Pricing:**
- **A100 40GB:** $1.50/hour
- **A100 80GB:** $2.50/hour
- **H100:** $4.00-6.00/hour

**Pros:**
- High-quality infrastructure
- Excellent for production training
- Pre-configured ML environments
- Great customer support

**Cons:**
- More expensive
- May have waiting lists for popular GPUs

### **🏢 5. Major Cloud Providers**

#### **Google Cloud Platform (GCP)**
**Pricing:**
- **A100 40GB:** $2.25-3.75/hour
- **V100:** $1.20-2.45/hour

**Best for:** Integration with Google services, enterprise use

#### **Amazon Web Services (AWS)**
**Pricing:**
- **A100 (p4d):** $2.50-4.00/hour
- **V100 (p3):** $1.20-3.06/hour

**Best for:** AWS ecosystem integration, enterprise use

#### **Microsoft Azure**
**Pricing:**
- **A100:** $2.50-4.50/hour
- **V100:** $1.50-3.60/hour

**Best for:** Microsoft ecosystem integration

## 💰 **Cost Comparison for Gemma3 4B Training**

Estimated training time: **6-12 hours** for 3 epochs

| Platform | GPU | Hourly Rate | Total Cost (10h) | Pros |
|----------|-----|-------------|------------------|------|
| **Colab Pro** | A100 24GB | $10/month | $10 | Easiest, includes storage |
| **Vast.ai** | RTX 4090 | $0.30/hour | $3 | Cheapest |
| **RunPod** | RTX 4090 | $0.34/hour | $3.40 | Reliable |
| **Lambda** | A100 40GB | $1.50/hour | $15 | Premium quality |
| **GCP** | A100 40GB | $3.00/hour | $30 | Enterprise grade |

## 🚀 **Setup Instructions by Platform**

### **Option 1: Google Colab (Recommended for Beginners)**

1. **Open Colab Notebook:**
```python
# Create new notebook at colab.research.google.com
# Enable GPU: Runtime > Change runtime type > GPU > A100

# Install requirements
!pip install torch transformers datasets accelerate trl peft wandb

# Clone your repository
!git clone https://github.com/your-username/asthra-lang.git
%cd asthra-lang/llmfinetuning

# Disable wandb
import os
os.environ['WANDB_DISABLED'] = 'true'

# Start training
!python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
```

2. **Monitor Training:**
```python
# Check GPU usage
!nvidia-smi

# View logs
!tail -f outputs/logs/asthra-gemma3-4b/training.log
```

### **Option 2: Vast.ai Setup**

1. **Create Account & Find Instance:**
   - Go to vast.ai
   - Search for: "RTX 4090" or "A100"
   - Filter: 24GB+ VRAM, 60GB+ RAM
   - Choose instance with good reliability score

2. **Connect & Setup:**
```bash
# SSH into instance
ssh root@your-vast-instance

# Update system
apt update && apt install -y git python3-pip

# Clone repository
git clone https://github.com/your-username/asthra-lang.git
cd asthra-lang/llmfinetuning

# Install dependencies
pip install -r requirements.txt

# Disable wandb
export WANDB_DISABLED=true

# Start training
python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
```

3. **Monitor Training:**
```bash
# Check GPU usage
watch -n 1 nvidia-smi

# Monitor logs
tail -f outputs/logs/asthra-gemma3-4b/training.log

# Screen session for persistence
screen -S training
python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
# Ctrl+A+D to detach, screen -r training to reattach
```

### **Option 3: RunPod Setup**

1. **Create Instance:**
   - Sign up at runpod.io
   - Choose "GPU Pods"
   - Select RTX 4090 or A100
   - Use PyTorch template

2. **Setup Environment:**
```bash
# In RunPod terminal
git clone https://github.com/your-username/asthra-lang.git
cd asthra-lang/llmfinetuning

# Install additional requirements
pip install -r requirements.txt

# Run training
WANDB_DISABLED=true python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml
```

## 🔧 **Cloud-Optimized Configuration**

Create a cloud-specific config file:

```yaml
# configs/cloud_gemma3_config.yaml
model:
  name: "google/gemma-2-2b-it"  # Smaller model for cloud efficiency
  trust_remote_code: true

training:
  output_dir: "./outputs/asthra-gemma3-4b-cloud"
  per_device_train_batch_size: 2  # Larger batch for cloud GPUs
  gradient_accumulation_steps: 8   # Reduced for faster training
  num_train_epochs: 2             # Fewer epochs for cost efficiency
  max_steps: 500                  # Limited steps
  
  # Faster evaluation
  eval_steps: 50
  save_steps: 100
  logging_steps: 25

data:
  max_samples: 1000               # Reduced dataset for cloud training
  max_seq_length: 2048           # Shorter sequences

monitoring:
  use_wandb: false               # Disabled for cloud
  use_tensorboard: true
```

## 📊 **Monitoring Your Training**

### **1. Using Screen/Tmux (Recommended)**
```bash
# Start screen session
screen -S asthra-training

# Run training
python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml

# Detach: Ctrl+A+D
# Reattach: screen -r asthra-training
```

### **2. Using nohup**
```bash
# Run in background
nohup python scripts/train_asthra_llm.py --config configs/gemma3_config.yaml > training.log 2>&1 &

# Monitor
tail -f training.log
```

### **3. Real-time Monitoring**
```bash
# GPU monitoring
watch -n 1 nvidia-smi

# Log monitoring
tail -f outputs/logs/asthra-gemma3-4b/training.log

# Process monitoring
htop
```

## 💾 **Data Transfer & Storage**

### **Upload Your Data:**
```bash
# Using scp
scp -r ./data/ username@instance-ip:~/asthra-lang/llmfinetuning/

# Using rsync
rsync -avz ./data/ username@instance-ip:~/asthra-lang/llmfinetuning/data/

# Using git (if data is in repo)
git clone --recurse-submodules https://github.com/your-username/asthra-lang.git
```

### **Download Results:**
```bash
# Download trained model
scp -r username@instance-ip:~/asthra-lang/llmfinetuning/outputs/ ./

# Download specific model
scp -r username@instance-ip:~/asthra-lang/llmfinetuning/outputs/asthra-gemma3-4b-v1/ ./
```

## 🛠️ **Cloud Training Optimizations**

### **1. Memory Optimization**
```yaml
# In your config file
training:
  gradient_checkpointing: true
  dataloader_pin_memory: true
  fp16: true  # Enable for NVIDIA GPUs
  
lora:
  rank: 16    # Reduce for faster training
  alpha: 8
```

### **2. Speed Optimization**
```yaml
training:
  per_device_train_batch_size: 4  # Larger batches
  gradient_accumulation_steps: 4   # Fewer accumulation steps
  dataloader_num_workers: 4        # Parallel data loading
```

### **3. Cost Optimization**
```yaml
data:
  max_samples: 1000     # Smaller dataset
  max_seq_length: 2048  # Shorter sequences

training:
  num_train_epochs: 2   # Fewer epochs
  max_steps: 500        # Hard limit
```

## 🚨 **Cloud Training Checklist**

### **Before Starting:**
- [ ] Upload your data to the cloud instance
- [ ] Test the environment with a small training run
- [ ] Set up monitoring (screen/tmux)
- [ ] Configure automatic model saving
- [ ] Estimate costs and set budgets

### **During Training:**
- [ ] Monitor GPU utilization (`nvidia-smi`)
- [ ] Check training logs regularly
- [ ] Verify model checkpoints are being saved
- [ ] Monitor costs (if applicable)

### **After Training:**
- [ ] Download the trained model
- [ ] Download training logs
- [ ] Test the model locally
- [ ] Terminate the cloud instance

## 💡 **Pro Tips**

### **Cost Savings:**
1. **Use Spot/Preemptible Instances:** 50-90% cheaper
2. **Monitor Usage:** Set up billing alerts
3. **Auto-shutdown:** Configure automatic instance termination
4. **Optimize Batch Size:** Larger batches = faster training

### **Reliability:**
1. **Save Checkpoints Frequently:** Every 100-250 steps
2. **Use Persistent Storage:** Attach external volumes
3. **Monitor Remotely:** Set up notifications
4. **Have Backup Plan:** Test on multiple platforms

### **Performance:**
1. **Choose Right GPU:** RTX 4090 sweet spot for 4B models
2. **Optimize Data Loading:** Use multiple workers
3. **Enable Mixed Precision:** fp16 or bf16
4. **Profile Memory Usage:** Optimize batch sizes

## 🎯 **My Recommendation**

For your Asthra Gemma3 4B training, I recommend:

**Best Starting Option:**
- **Google Colab Pro** ($10/month) - Easy, reliable, includes everything

**Best Value Option:**
- **Vast.ai RTX 4090** (~$3-4 total) - Cheapest, sufficient for 4B model

**Best Professional Option:**
- **Lambda Labs A100** (~$15 total) - Premium quality, reliable

Start with Colab Pro to test everything, then move to Vast.ai for production training if you need to run multiple experiments.

Would you like me to help you set up training on any specific platform? 
