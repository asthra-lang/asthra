#!/usr/bin/env python3
"""
Asthra LLM Fine-tuning Training Script
Version: 1.0
Created: January 2025

Main training script for fine-tuning Devstral-Small-2505 on Asthra programming language data.
"""

import os
import sys
import yaml
import torch
import argparse
import logging
from pathlib import Path
from datetime import datetime
from typing import Dict, Any, Optional
from dotenv import load_dotenv

# Load environment variables from .env file
load_dotenv()

# Add src to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from transformers import (
    AutoModelForCausalLM,
    AutoTokenizer, 
    TrainingArguments,
    Trainer,
    DataCollatorForLanguageModeling
)
from datasets import load_dataset
from peft import LoraConfig, get_peft_model, TaskType
import wandb

# Authenticate with Hugging Face if token is available
def setup_hf_auth():
    """Set up Hugging Face authentication using environment token."""
    hf_token = os.getenv('HF_TOKEN')
    if hf_token:
        try:
            from huggingface_hub import login
            login(token=hf_token, add_to_git_credential=False)
            return True
        except Exception as e:
            print(f"Warning: Failed to authenticate with Hugging Face: {e}")
            return False
    return False

def setup_logging(log_dir: Path, verbose: bool = False) -> logging.Logger:
    """Set up logging configuration."""
    log_dir.mkdir(parents=True, exist_ok=True)
    
    log_level = logging.DEBUG if verbose else logging.INFO
    
    # Create logger
    logger = logging.getLogger('asthra_llm_training')
    logger.setLevel(log_level)
    
    # Console handler
    console_handler = logging.StreamHandler()
    console_handler.setLevel(log_level)
    console_format = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    console_handler.setFormatter(console_format)
    logger.addHandler(console_handler)
    
    # File handler
    log_file = log_dir / f"training_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
    file_handler = logging.FileHandler(log_file)
    file_handler.setLevel(logging.DEBUG)
    file_format = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(funcName)s:%(lineno)d - %(message)s'
    )
    file_handler.setFormatter(file_format)
    logger.addHandler(file_handler)
    
    return logger

def load_config(config_path: str) -> Dict[str, Any]:
    """Load configuration from YAML file."""
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)
    return config

def setup_model_and_tokenizer(config: Dict[str, Any], logger: logging.Logger):
    """Set up the Devstral model and tokenizer."""
    logger.info("Setting up Devstral-Small-2505 model and tokenizer...")
    
    model_config = config['model']
    
    # Load tokenizer
    logger.info(f"Loading tokenizer: {model_config['tokenizer_name']}")
    tokenizer = AutoTokenizer.from_pretrained(
        model_config['tokenizer_name'],
        trust_remote_code=model_config.get('trust_remote_code', True),
        use_fast=True
    )
    
    # Set padding token if not present
    if tokenizer.pad_token is None:
        tokenizer.pad_token = tokenizer.eos_token
        tokenizer.pad_token_id = tokenizer.eos_token_id
    
    # Load model with quantization
    logger.info(f"Loading model: {model_config['name']}")
    quantization_config = None
    
    if model_config.get('quantization', {}).get('load_in_4bit', False):
        from transformers import BitsAndBytesConfig
        quantization_config = BitsAndBytesConfig(
            load_in_4bit=True,
            bnb_4bit_compute_dtype=getattr(torch, model_config['quantization']['bnb_4bit_compute_dtype']),
            bnb_4bit_quant_type=model_config['quantization']['bnb_4bit_quant_type'],
            bnb_4bit_use_double_quant=model_config['quantization']['bnb_4bit_use_double_quant']
        )
        logger.info("Using 4-bit quantization for memory efficiency")
    
    # Determine attention implementation with fallback
    attn_implementation = None
    if torch.cuda.is_available():
        try:
            import flash_attn
            attn_implementation = "flash_attention_2"
            logger.info("Using Flash Attention 2 for faster training")
        except ImportError:
            logger.warning("Flash Attention not available, falling back to standard attention")
            logger.info("To install Flash Attention: run 'bash scripts/quick_flash_attention_fix.sh'")
            attn_implementation = None
    
    try:
        model = AutoModelForCausalLM.from_pretrained(
            model_config['name'],
            quantization_config=quantization_config,
            device_map=config['hardware'].get('device_map', 'auto'),
            trust_remote_code=model_config.get('trust_remote_code', True),
            torch_dtype=torch.bfloat16,
            attn_implementation=attn_implementation
        )
    except Exception as e:
        if "flash_attn" in str(e).lower():
            logger.warning(f"Flash Attention failed: {e}")
            logger.info("Retrying with standard attention...")
            model = AutoModelForCausalLM.from_pretrained(
                model_config['name'],
                quantization_config=quantization_config,
                device_map=config['hardware'].get('device_map', 'auto'),
                trust_remote_code=model_config.get('trust_remote_code', True),
                torch_dtype=torch.bfloat16,
                attn_implementation=None
            )
        else:
            raise
    
    logger.info(f"Model loaded successfully. Parameters: {model.num_parameters():,}")
    return model, tokenizer

def setup_lora(model, config: Dict[str, Any], logger: logging.Logger):
    """Set up LoRA for parameter-efficient fine-tuning."""
    logger.info("Setting up LoRA configuration...")
    
    lora_config = config['lora']
    
    peft_config = LoraConfig(
        task_type=TaskType.CAUSAL_LM,
        inference_mode=False,
        r=lora_config['rank'],
        lora_alpha=lora_config['alpha'],
        lora_dropout=lora_config['dropout'],
        bias=lora_config.get('bias', 'none'),
        target_modules=lora_config['target_modules'],
        modules_to_save=lora_config.get('modules_to_save')
    )
    
    model = get_peft_model(model, peft_config)
    model.print_trainable_parameters()
    
    logger.info("LoRA configuration applied successfully")
    return model

def prepare_datasets(config: Dict[str, Any], tokenizer, logger: logging.Logger):
    """Prepare training and validation datasets."""
    logger.info("Preparing training datasets...")
    
    data_config = config['data']
    dataset_path = data_config['dataset_path']
    
    # Load datasets
    datasets = {}
    dataset_files = {
        'train': 'instruction_tuning.jsonl',
        'validation': 'validation.jsonl'
    }
    
    for split, filename in dataset_files.items():
        file_path = Path(dataset_path) / filename
        if file_path.exists():
            logger.info(f"Loading {split} dataset from {file_path}")
            datasets[split] = load_dataset('json', data_files=str(file_path), split='train')
            logger.info(f"{split} dataset size: {len(datasets[split]):,} samples")
        else:
            logger.warning(f"Dataset file not found: {file_path}")
    
    if 'train' not in datasets:
        raise FileNotFoundError("Training dataset not found. Run prepare_data.py first.")
    
    # Tokenize datasets
    def tokenize_function(examples):
        # Handle different input formats
        if 'instruction' in examples and 'response' in examples:
            # Instruction-response format
            texts = []
            for instruction, response in zip(examples['instruction'], examples['response']):
                text = f"### Instruction:\n{instruction}\n\n### Response:\n{response}"
                texts.append(text)
        elif 'prompt' in examples and 'completion' in examples:
            # Prompt-completion format
            texts = []
            for prompt, completion in zip(examples['prompt'], examples['completion']):
                text = f"{prompt}{completion}"
                texts.append(text)
        else:
            # Assume direct text content
            texts = examples.get('content', examples.get('text', []))
        
        # Tokenize
        tokenized = tokenizer(
            texts,
            truncation=True,
            padding=False,  # Let data collator handle padding
            max_length=data_config.get('max_seq_length', 2048),  # Use reasonable max length
            return_overflowing_tokens=False,
            return_attention_mask=True
        )
        
        # Set labels for language modeling (copy of input_ids)
        tokenized['labels'] = [ids.copy() for ids in tokenized['input_ids']]
        
        return tokenized
    
    logger.info("Tokenizing datasets...")
    tokenized_datasets = {}
    for split, dataset in datasets.items():
        tokenized_datasets[split] = dataset.map(
            tokenize_function,
            batched=True,
            num_proc=data_config.get('preprocessing_num_workers', 4),
            remove_columns=dataset.column_names
        )
        logger.info(f"Tokenized {split} dataset: {len(tokenized_datasets[split]):,} samples")
    
    return tokenized_datasets

def setup_training_arguments(config: Dict[str, Any], logger: logging.Logger) -> TrainingArguments:
    """Set up training arguments."""
    logger.info("Setting up training arguments...")
    
    training_config = config['training']
    monitoring_config = config.get('monitoring', {})
    
    # Create output directory
    output_dir = Path(training_config['output_dir'])
    output_dir.mkdir(parents=True, exist_ok=True)
    
    training_args = TrainingArguments(
        # Output
        output_dir=str(output_dir),
        overwrite_output_dir=training_config.get('overwrite_output_dir', True),
        
        # Training schedule
        num_train_epochs=training_config.get('num_train_epochs', 3),
        max_steps=training_config.get('max_steps', -1),
        
        # Batch size and gradient accumulation
        per_device_train_batch_size=training_config.get('per_device_train_batch_size', 1),
        per_device_eval_batch_size=training_config.get('per_device_eval_batch_size', 1),
        gradient_accumulation_steps=training_config.get('gradient_accumulation_steps', 16),
        dataloader_num_workers=training_config.get('dataloader_num_workers', 4),
        
        # Optimization
        learning_rate=training_config.get('learning_rate', 5e-5),
        lr_scheduler_type=training_config.get('lr_scheduler_type', 'cosine'),
        warmup_ratio=training_config.get('warmup_ratio', 0.1),
        weight_decay=training_config.get('weight_decay', 0.01),
        max_grad_norm=training_config.get('max_grad_norm', 1.0),
        
        # Mixed precision
        fp16=training_config.get('fp16', False),
        bf16=training_config.get('bf16', True),
        
        # Logging and evaluation
        logging_dir=training_config.get('logging_dir', './outputs/logs/'),
        logging_steps=training_config.get('logging_steps', 50),
        eval_strategy=training_config.get('eval_strategy', 'steps'),
        eval_steps=training_config.get('eval_steps', 500),
        
        # Saving
        save_strategy=training_config.get('save_strategy', 'steps'),
        save_steps=training_config.get('save_steps', 1000),
        save_total_limit=training_config.get('save_total_limit', 3),
        
        # Other settings
        remove_unused_columns=training_config.get('remove_unused_columns', False),
        group_by_length=training_config.get('group_by_length', True),
        ddp_find_unused_parameters=training_config.get('ddp_find_unused_parameters', False),
        gradient_checkpointing=config['hardware'].get('gradient_checkpointing', True),
        dataloader_pin_memory=config['hardware'].get('dataloader_pin_memory', True),
        
        # Reporting
        report_to=["wandb", "tensorboard"] if monitoring_config.get('use_wandb') else ["tensorboard"],
        run_name=f"asthra_llm_{datetime.now().strftime('%Y%m%d_%H%M%S')}",
        
        # Reproducibility
        seed=config.get('advanced', {}).get('seed', 42)
    )
    
    logger.info("Training arguments configured successfully")
    return training_args

def setup_wandb(config: Dict[str, Any], logger: logging.Logger):
    """Set up Weights & Biases monitoring."""
    monitoring_config = config.get('monitoring', {})
    
    if not monitoring_config.get('use_wandb', False):
        logger.info("Weights & Biases monitoring disabled")
        return
    
    logger.info("Setting up Weights & Biases monitoring...")
    
    try:
        wandb.init(
            project=monitoring_config.get('wandb_project', 'asthra-llm-finetuning'),
            entity=monitoring_config.get('wandb_entity'),
            name=f"asthra_llm_{datetime.now().strftime('%Y%m%d_%H%M%S')}",
            config=config,
            tags=['asthra', 'devstral', 'fine-tuning'],
            notes="Fine-tuning Devstral-Small-2505 on Asthra programming language"
        )
        logger.info("Weights & Biases initialized successfully")
    except Exception as e:
        logger.warning(f"Failed to initialize Weights & Biases: {e}")
        logger.info("Continuing without W&B monitoring")

class AsthraProgrammingTrainer(Trainer):
    """Custom trainer with Asthra-specific functionality."""
    
    def __init__(self, asthra_compiler_path: Optional[str] = None, **kwargs):
        super().__init__(**kwargs)
        self.asthra_compiler_path = asthra_compiler_path
        self.compilation_success_rate = 0.0
    
    def compute_loss(self, model, inputs, return_outputs=False, **kwargs):
        """Compute loss with optional custom metrics."""
        return super().compute_loss(model, inputs, return_outputs=return_outputs, **kwargs)
    
    def log(self, logs, start_time=None):
        """Custom logging with Asthra-specific metrics."""
        logs.update({
            'compilation_success_rate': self.compilation_success_rate,
            'gpu_memory_used': torch.cuda.memory_allocated() / 1e9 if torch.cuda.is_available() else 0,
            'gpu_memory_reserved': torch.cuda.memory_reserved() / 1e9 if torch.cuda.is_available() else 0
        })
        super().log(logs, start_time)

def main():
    """Main training function."""
    parser = argparse.ArgumentParser(description="Train Asthra LLM using Devstral-Small-2505")
    parser.add_argument('--config', default='configs/devstral_config.yaml', help='Configuration file')
    parser.add_argument('--resume', help='Resume from checkpoint')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logging')
    parser.add_argument('--debug', action='store_true', help='Debug mode (small dataset)')
    
    args = parser.parse_args()
    
    # Load configuration
    config = load_config(args.config)
    
    # Debug mode adjustments
    if args.debug:
        config['data']['max_samples'] = config.get('advanced', {}).get('debug_dataset_size', 100)
        config['training']['num_train_epochs'] = 1
        config['training']['save_steps'] = 10
        config['training']['eval_steps'] = 10
        config['training']['logging_steps'] = 5
    
    # Set up logging
    log_dir = Path(config['training']['logging_dir'])
    logger = setup_logging(log_dir, args.verbose)
    
    logger.info("🚀 Starting Asthra LLM Fine-tuning")
    logger.info("=" * 50)
    logger.info(f"Configuration: {args.config}")
    logger.info(f"Debug mode: {args.debug}")
    logger.info(f"Resume from: {args.resume or 'None'}")
    
    # Log system information
    logger.info(f"PyTorch version: {torch.__version__}")
    logger.info(f"CUDA available: {torch.cuda.is_available()}")
    if torch.cuda.is_available():
        logger.info(f"CUDA devices: {torch.cuda.device_count()}")
        for i in range(torch.cuda.device_count()):
            logger.info(f"  GPU {i}: {torch.cuda.get_device_name(i)}")
    
    try:
        # Set up Hugging Face authentication
        hf_authenticated = setup_hf_auth()
        if hf_authenticated:
            logger.info("✅ Hugging Face authentication successful")
        else:
            logger.info("⚠️  No HF_TOKEN found - using public models only")
        
        # Set up W&B monitoring
        setup_wandb(config, logger)
        
        # Load model and tokenizer
        model, tokenizer = setup_model_and_tokenizer(config, logger)
        
        # Set up LoRA
        model = setup_lora(model, config, logger)
        
        # Ensure model is in training mode and gradients are enabled
        model.train()
        
        # Enable gradients for LoRA parameters
        for name, param in model.named_parameters():
            if 'lora' in name.lower() or param.requires_grad:
                param.requires_grad_(True)
        
        logger.info(f"Model training mode: {model.training}")
        trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)
        logger.info(f"Trainable parameters: {trainable_params:,}")
        
        # Prepare datasets
        datasets = prepare_datasets(config, tokenizer, logger)
        
        # Set up training arguments
        training_args = setup_training_arguments(config, logger)
        
        # Set up data collator with proper padding for variable sequence lengths
        data_collator = DataCollatorForLanguageModeling(
            tokenizer=tokenizer,
            mlm=False,  # We're doing causal language modeling
            pad_to_multiple_of=8 if training_args.fp16 or training_args.bf16 else None,
            return_tensors="pt"
        )
        
        # Ensure tokenizer has a pad token
        if tokenizer.pad_token is None:
            if tokenizer.eos_token is not None:
                tokenizer.pad_token = tokenizer.eos_token
            else:
                tokenizer.add_special_tokens({'pad_token': '[PAD]'})
        
        # Create trainer
        asthra_compiler = config['evaluation'].get('asthra_compiler_path')
        trainer = AsthraProgrammingTrainer(
            model=model,
            args=training_args,
            train_dataset=datasets['train'],
            eval_dataset=datasets.get('validation'),
            tokenizer=tokenizer,
            data_collator=data_collator,
            asthra_compiler_path=asthra_compiler
        )
        
        # Resume from checkpoint if specified
        if args.resume:
            logger.info(f"Resuming training from checkpoint: {args.resume}")
            trainer.train(resume_from_checkpoint=args.resume)
        else:
            logger.info("Starting training from scratch")
            trainer.train()
        
        # Save final model
        final_model_path = Path(config['training']['output_dir']) / 'final'
        final_model_path.mkdir(exist_ok=True)
        
        logger.info(f"Saving final model to {final_model_path}")
        trainer.save_model(str(final_model_path))
        tokenizer.save_pretrained(str(final_model_path))
        
        # Save training summary
        training_summary = {
            'timestamp': datetime.now().isoformat(),
            'config': config,
            'final_model_path': str(final_model_path),
            'training_stats': trainer.state.log_history[-1] if trainer.state.log_history else {}
        }
        
        summary_file = final_model_path / 'training_summary.yaml'
        with open(summary_file, 'w') as f:
            yaml.dump(training_summary, f, default_flow_style=False)
        
        logger.info("✅ Training completed successfully!")
        logger.info(f"📁 Final model saved to: {final_model_path}")
        logger.info(f"📊 Training summary: {summary_file}")
        
        # Close W&B run
        if config.get('monitoring', {}).get('use_wandb', False):
            wandb.finish()
        
    except Exception as e:
        logger.error(f"❌ Training failed: {e}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main() 
