#!/usr/bin/env python3
"""
vLLM OpenAI-Compatible API Server for Asthra-Gemma3-4B
High-performance serving with OpenAI API compatibility
"""

import argparse
import asyncio
import logging
import os
import sys
from pathlib import Path
from typing import Dict, Any, Optional

# Add project root to path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

try:
    from vllm import LLM, SamplingParams
    from vllm.entrypoints.openai.api_server import run_server
    from vllm.entrypoints.openai.serving_chat import OpenAIServingChat
    from vllm.entrypoints.openai.serving_completion import OpenAIServingCompletion
except ImportError:
    print("❌ vLLM not installed. Install with: pip install vllm")
    sys.exit(1)

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class AsthrarvLLMServer:
    """vLLM-based OpenAI-compatible server for Asthra models."""
    
    def __init__(self, model_path: str, config: Dict[str, Any]):
        self.model_path = model_path
        self.config = config
        self.llm = None
        
    def load_model(self):
        """Load the fine-tuned Asthra model with vLLM."""
        logger.info(f"Loading Asthra model from: {self.model_path}")
        
        # vLLM configuration
        vllm_config = {
            "model": self.model_path,
            "tensor_parallel_size": self.config.get('tensor_parallel_size', 1),
            "dtype": self.config.get('dtype', 'auto'),
            "gpu_memory_utilization": self.config.get('gpu_memory_utilization', 0.9),
            "max_model_len": self.config.get('max_model_len', 32768),
            "quantization": self.config.get('quantization', None),
            "trust_remote_code": True
        }
        
        try:
            self.llm = LLM(**vllm_config)
            logger.info("✅ Asthra model loaded successfully with vLLM")
        except Exception as e:
            logger.error(f"❌ Failed to load model: {e}")
            raise
    
    async def start_server(self):
        """Start the OpenAI-compatible API server."""
        if not self.llm:
            self.load_model()
        
        server_config = {
            "host": self.config.get('host', '0.0.0.0'),
            "port": self.config.get('port', 8000),
            "log_level": self.config.get('log_level', 'info'),
            "model": self.model_path,
            "served_model_name": self.config.get('served_model_name', 'asthra-gemma3-4b'),
            "chat_template": self.config.get('chat_template', None),
            "response_role": "assistant"
        }
        
        logger.info(f"🚀 Starting Asthra vLLM server on {server_config['host']}:{server_config['port']}")
        logger.info(f"📝 Model name: {server_config['served_model_name']}")
        
        # Start the server
        await run_server(
            model=server_config['model'],
            host=server_config['host'],
            port=server_config['port'],
            log_level=server_config['log_level'],
            served_model_name=server_config['served_model_name'],
            chat_template=server_config['chat_template'],
            response_role=server_config['response_role']
        )

def load_server_config(config_path: str) -> Dict[str, Any]:
    """Load server configuration."""
    import yaml
    
    if not os.path.exists(config_path):
        logger.warning(f"Config file not found: {config_path}. Using defaults.")
        return {}
    
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)
    
    return config.get('server', {})

async def main():
    """Main server function."""
    parser = argparse.ArgumentParser(description="vLLM OpenAI-Compatible Server for Asthra Models")
    parser.add_argument('--model', required=True, help='Path to fine-tuned Asthra model')
    parser.add_argument('--config', default='configs/server_config.yaml', help='Server configuration file')
    parser.add_argument('--host', default='0.0.0.0', help='Server host')
    parser.add_argument('--port', type=int, default=8000, help='Server port')
    parser.add_argument('--model-name', default='asthra-gemma3-4b', help='Served model name')
    parser.add_argument('--max-model-len', type=int, default=32768, help='Maximum model length')
    parser.add_argument('--gpu-memory-utilization', type=float, default=0.9, help='GPU memory utilization')
    parser.add_argument('--tensor-parallel-size', type=int, default=1, help='Tensor parallel size')
    
    args = parser.parse_args()
    
    # Load configuration
    config = load_server_config(args.config)
    
    # Override with command line arguments
    config.update({
        'host': args.host,
        'port': args.port,
        'served_model_name': args.model_name,
        'max_model_len': args.max_model_len,
        'gpu_memory_utilization': args.gpu_memory_utilization,
        'tensor_parallel_size': args.tensor_parallel_size
    })
    
    # Validate model path
    if not os.path.exists(args.model):
        logger.error(f"❌ Model path not found: {args.model}")
        sys.exit(1)
    
    logger.info("🚀 Starting Asthra vLLM OpenAI-Compatible Server")
    logger.info("=" * 50)
    logger.info(f"Model: {args.model}")
    logger.info(f"Server: http://{args.host}:{args.port}")
    logger.info(f"Model name: {args.model_name}")
    logger.info(f"Max length: {args.max_model_len}")
    
    # Create and start server
    server = AsthrarvLLMServer(args.model, config)
    
    try:
        await server.start_server()
    except KeyboardInterrupt:
        logger.info("🛑 Server stopped by user")
    except Exception as e:
        logger.error(f"❌ Server error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    asyncio.run(main()) 
