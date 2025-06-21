#!/usr/bin/env python3
"""
FastAPI OpenAI-Compatible Server for Asthra-Gemma3-4B
Custom implementation using transformers and FastAPI
"""

import argparse
import asyncio
import json
import logging
import os
import sys
import time
import uuid
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Any, AsyncGenerator

import torch
from fastapi import FastAPI, HTTPException, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import StreamingResponse, JSONResponse
from pydantic import BaseModel, Field
import uvicorn
from contextlib import asynccontextmanager

# Add project root to path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

try:
    from transformers import (
        AutoModelForCausalLM, 
        AutoTokenizer, 
        GenerationConfig,
        TextIteratorStreamer
    )
    from peft import PeftModel, PeftConfig
    from threading import Thread
except ImportError:
    print("❌ Required packages not installed. Install with: pip install transformers torch fastapi uvicorn peft")
    sys.exit(1)

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# OpenAI API Models
class ChatMessage(BaseModel):
    role: str = Field(..., description="Role of the message sender")
    content: str = Field(..., description="Content of the message")

class ChatCompletionRequest(BaseModel):
    model: str = Field(default="asthra-gemma3-4b", description="Model to use")
    messages: List[ChatMessage] = Field(..., description="Messages in the conversation")
    max_tokens: Optional[int] = Field(default=2048, description="Maximum tokens to generate")
    temperature: Optional[float] = Field(default=0.7, description="Sampling temperature")
    top_p: Optional[float] = Field(default=0.9, description="Top-p sampling")
    top_k: Optional[int] = Field(default=50, description="Top-k sampling")
    stop: Optional[List[str]] = Field(default=None, description="Stop sequences")
    stream: Optional[bool] = Field(default=False, description="Stream response")
    presence_penalty: Optional[float] = Field(default=0.0, description="Presence penalty")
    frequency_penalty: Optional[float] = Field(default=0.0, description="Frequency penalty")

class CompletionRequest(BaseModel):
    model: str = Field(default="asthra-gemma3-4b", description="Model to use")
    prompt: str = Field(..., description="Prompt for completion")
    max_tokens: Optional[int] = Field(default=2048, description="Maximum tokens to generate")
    temperature: Optional[float] = Field(default=0.7, description="Sampling temperature")
    top_p: Optional[float] = Field(default=0.9, description="Top-p sampling")
    top_k: Optional[int] = Field(default=50, description="Top-k sampling")
    stop: Optional[List[str]] = Field(default=None, description="Stop sequences")
    stream: Optional[bool] = Field(default=False, description="Stream response")
    echo: Optional[bool] = Field(default=False, description="Echo the prompt")

class ModelInfo(BaseModel):
    id: str
    object: str = "model"
    created: int
    owned_by: str = "asthra-team"

class AsthraiModelServer:
    """FastAPI-based OpenAI-compatible server for Asthra models."""
    
    def __init__(self, model_path: str, config: Dict[str, Any]):
        self.model_path = model_path
        self.config = config
        self.model = None
        self.tokenizer = None
        self.device = torch.device("cuda" if torch.cuda.is_available() else "mps" if torch.backends.mps.is_available() else "cpu")
        
    def load_model(self):
        """Load the fine-tuned Asthra model."""
        logger.info(f"Loading Asthra model from: {self.model_path}")
        logger.info(f"Using device: {self.device}")
        
        try:
            # Check if this is a PEFT/LoRA adapter
            adapter_config_path = os.path.join(self.model_path, "adapter_config.json")
            is_peft_adapter = os.path.exists(adapter_config_path)
            
            if is_peft_adapter:
                logger.info("🔄 Detected PEFT/LoRA adapter model")
                
                # Load PEFT config to get base model
                peft_config = PeftConfig.from_pretrained(self.model_path)
                base_model_name = peft_config.base_model_name_or_path
                
                logger.info(f"📦 Loading base model: {base_model_name}")
                
                # Load tokenizer from adapter path (should be compatible)
                self.tokenizer = AutoTokenizer.from_pretrained(
                    self.model_path,
                    trust_remote_code=True,
                    padding_side="left"
                )
                
                # Load base model
                model_kwargs = {
                    "trust_remote_code": True,
                    "torch_dtype": torch.float16 if self.device.type == "cuda" else torch.float32,
                    "device_map": "auto" if self.device.type == "cuda" else None,
                    "low_cpu_mem_usage": True
                }
                
                base_model = AutoModelForCausalLM.from_pretrained(
                    base_model_name,
                    **model_kwargs
                )
                
                # Load PEFT adapter
                logger.info(f"🔧 Loading PEFT adapter from: {self.model_path}")
                self.model = PeftModel.from_pretrained(base_model, self.model_path)
                
            else:
                logger.info("📦 Loading standard model")
                
                # Load tokenizer
                self.tokenizer = AutoTokenizer.from_pretrained(
                    self.model_path,
                    trust_remote_code=True,
                    padding_side="left"
                )
                
                # Load model
                model_kwargs = {
                    "trust_remote_code": True,
                    "torch_dtype": torch.float16 if self.device.type == "cuda" else torch.float32,
                    "device_map": "auto" if self.device.type == "cuda" else None,
                    "low_cpu_mem_usage": True
                }
                
                self.model = AutoModelForCausalLM.from_pretrained(
                    self.model_path,
                    **model_kwargs
                )
            
            # Ensure pad token exists
            if self.tokenizer.pad_token is None:
                if self.tokenizer.eos_token is not None:
                    self.tokenizer.pad_token = self.tokenizer.eos_token
                else:
                    self.tokenizer.add_special_tokens({'pad_token': '[PAD]'})
            
            if self.device.type != "cuda":
                self.model = self.model.to(self.device)
            
            self.model.eval()
            
            logger.info("✅ Asthra model loaded successfully")
            if hasattr(self.model, 'num_parameters'):
                logger.info(f"📊 Model parameters: {self.model.num_parameters():,}")
            else:
                logger.info("📊 Model loaded (parameter count unavailable)")
            
        except Exception as e:
            logger.error(f"❌ Failed to load model: {e}")
            raise
    
    def format_chat_prompt(self, messages: List[ChatMessage]) -> str:
        """Format chat messages into a prompt."""
        # Simple chat formatting for Asthra
        prompt_parts = []
        
        for message in messages:
            if message.role == "system":
                prompt_parts.append(f"System: {message.content}")
            elif message.role == "user":
                prompt_parts.append(f"User: {message.content}")
            elif message.role == "assistant":
                prompt_parts.append(f"Assistant: {message.content}")
        
        prompt_parts.append("Assistant:")
        return "\n\n".join(prompt_parts)
    
    def generate_text(
        self, 
        prompt: str, 
        max_tokens: int = 2048,
        temperature: float = 0.7,
        top_p: float = 0.9,
        top_k: int = 50,
        stop_sequences: Optional[List[str]] = None,
        stream: bool = False
    ):
        """Generate text from the model."""
        # Tokenize input
        inputs = self.tokenizer(
            prompt,
            return_tensors="pt",
            truncation=True,
            max_length=self.config.get('max_context_length', 30720)  # Leave room for generation
        ).to(self.device)
        
        # Generation configuration
        generation_config = GenerationConfig(
            max_new_tokens=max_tokens,
            temperature=temperature,
            top_p=top_p,
            top_k=top_k,
            do_sample=True,
            pad_token_id=self.tokenizer.pad_token_id,
            eos_token_id=self.tokenizer.eos_token_id,
            repetition_penalty=1.1
        )
        
        if stream:
            return self._generate_stream(inputs, generation_config, stop_sequences)
        else:
            return self._generate_complete(inputs, generation_config, stop_sequences)
    
    def _generate_complete(self, inputs, generation_config, stop_sequences):
        """Generate complete response."""
        with torch.no_grad():
            outputs = self.model.generate(
                **inputs,
                generation_config=generation_config,
                return_dict_in_generate=True,
                output_scores=True
            )
        
        # Decode generated text
        generated_ids = outputs.sequences[0][len(inputs['input_ids'][0]):]
        generated_text = self.tokenizer.decode(generated_ids, skip_special_tokens=True)
        
        # Apply stop sequences
        if stop_sequences:
            for stop in stop_sequences:
                if stop in generated_text:
                    generated_text = generated_text.split(stop)[0]
                    break
        
        return generated_text.strip()
    
    def _generate_stream(self, inputs, generation_config, stop_sequences):
        """Generate streaming response."""
        streamer = TextIteratorStreamer(
            self.tokenizer,
            skip_prompt=True,
            skip_special_tokens=True
        )
        
        generation_kwargs = {
            **inputs,
            "generation_config": generation_config,
            "streamer": streamer
        }
        
        thread = Thread(target=self.model.generate, kwargs=generation_kwargs)
        thread.start()
        
        generated_text = ""
        for new_text in streamer:
            generated_text += new_text
            
            # Check for stop sequences
            if stop_sequences:
                for stop in stop_sequences:
                    if stop in generated_text:
                        generated_text = generated_text.split(stop)[0]
                        return generated_text.strip()
            
            yield new_text
        
        thread.join()

# Global model server instance
model_server: Optional[AsthraiModelServer] = None

def get_model_config():
    """Get model configuration from environment variables or command line."""
    model_path = os.environ.get('ASTHRA_MODEL_PATH', './outputs/asthra-gemma3-4b-v1')
    max_context = int(os.environ.get('ASTHRA_MAX_CONTEXT', '30720'))
    
    return {
        'model_path': model_path,
        'config': {'max_context_length': max_context}
    }

@asynccontextmanager
async def lifespan(app: FastAPI):
    """Lifespan context manager for startup and shutdown."""
    # Startup
    global model_server
    
    try:
        model_config = get_model_config()
        model_path = model_config['model_path']
        config = model_config['config']
        
        logger.info("🚀 Initializing model on server startup...")
        logger.info(f"📂 Model path: {model_path}")
        
        if os.path.exists(model_path):
            model_server = AsthraiModelServer(model_path, config)
            model_server.load_model()
            logger.info("✅ Model initialization completed!")
        else:
            logger.error(f"❌ Model path not found: {model_path}")
            logger.error("Set ASTHRA_MODEL_PATH environment variable or ensure model exists")
    except Exception as e:
        logger.error(f"❌ Failed to initialize model: {e}")
        # Don't fail startup, just log the error
    
    yield
    
    # Shutdown
    logger.info("🛑 Server shutting down...")
    if model_server:
        del model_server

# Create FastAPI app with lifespan
app = FastAPI(
    title="Asthra OpenAI-Compatible API",
    description="OpenAI-compatible API for fine-tuned Asthra models",
    version="1.0.0",
    lifespan=lifespan
)

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/v1/models")
async def list_models():
    """List available models."""
    return {
        "object": "list",
        "data": [
            ModelInfo(
                id="asthra-gemma3-4b",
                created=int(time.time()),
                owned_by="asthra-team"
            )
        ]
    }

@app.post("/v1/chat/completions")
async def create_chat_completion(request: ChatCompletionRequest):
    """Create a chat completion."""
    if not model_server:
        raise HTTPException(status_code=503, detail="Model not loaded")
    
    try:
        # Format prompt
        prompt = model_server.format_chat_prompt(request.messages)
        
        if request.stream:
            return StreamingResponse(
                generate_chat_stream(request, prompt),
                media_type="text/plain"
            )
        else:
            # Generate response
            generated_text = model_server.generate_text(
                prompt=prompt,
                max_tokens=request.max_tokens,
                temperature=request.temperature,
                top_p=request.top_p,
                top_k=request.top_k,
                stop_sequences=request.stop,
                stream=False
            )
            
            # Format response
            response = {
                "id": f"chatcmpl-{uuid.uuid4().hex}",
                "object": "chat.completion",
                "created": int(time.time()),
                "model": request.model,
                "choices": [
                    {
                        "index": 0,
                        "message": {
                            "role": "assistant",
                            "content": generated_text
                        },
                        "finish_reason": "stop"
                    }
                ],
                "usage": {
                    "prompt_tokens": 0,  # TODO: Calculate actual tokens
                    "completion_tokens": 0,
                    "total_tokens": 0
                }
            }
            
            return JSONResponse(content=response)
            
    except Exception as e:
        logger.error(f"❌ Chat completion error: {e}")
        raise HTTPException(status_code=500, detail=str(e))

async def generate_chat_stream(request: ChatCompletionRequest, prompt: str):
    """Generate streaming chat completion."""
    for chunk in model_server.generate_text(
        prompt=prompt,
        max_tokens=request.max_tokens,
        temperature=request.temperature,
        top_p=request.top_p,
        top_k=request.top_k,
        stop_sequences=request.stop,
        stream=True
    ):
        chunk_data = {
            "id": f"chatcmpl-{uuid.uuid4().hex}",
            "object": "chat.completion.chunk",
            "created": int(time.time()),
            "model": request.model,
            "choices": [
                {
                    "index": 0,
                    "delta": {"content": chunk},
                    "finish_reason": None
                }
            ]
        }
        yield f"data: {json.dumps(chunk_data)}\n\n"
    
    # Send final chunk
    final_chunk = {
        "id": f"chatcmpl-{uuid.uuid4().hex}",
        "object": "chat.completion.chunk",
        "created": int(time.time()),
        "model": request.model,
        "choices": [
            {
                "index": 0,
                "delta": {},
                "finish_reason": "stop"
            }
        ]
    }
    yield f"data: {json.dumps(final_chunk)}\n\n"
    yield "data: [DONE]\n\n"

@app.post("/v1/completions")
async def create_completion(request: CompletionRequest):
    """Create a text completion."""
    if not model_server:
        raise HTTPException(status_code=503, detail="Model not loaded")
    
    try:
        generated_text = model_server.generate_text(
            prompt=request.prompt,
            max_tokens=request.max_tokens,
            temperature=request.temperature,
            top_p=request.top_p,
            top_k=request.top_k,
            stop_sequences=request.stop,
            stream=request.stream
        )
        
        response = {
            "id": f"cmpl-{uuid.uuid4().hex}",
            "object": "text_completion",
            "created": int(time.time()),
            "model": request.model,
            "choices": [
                {
                    "text": generated_text,
                    "index": 0,
                    "finish_reason": "stop"
                }
            ]
        }
        
        return JSONResponse(content=response)
        
    except Exception as e:
        logger.error(f"❌ Completion error: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/health")
async def health_check():
    """Health check endpoint."""
    model_loaded = model_server is not None and model_server.model is not None
    return {
        "status": "healthy", 
        "model_loaded": model_loaded,
        "model_path": os.environ.get('ASTHRA_MODEL_PATH', 'Not set')
    }

def main():
    """Main server function."""
    parser = argparse.ArgumentParser(description="FastAPI OpenAI-Compatible Server for Asthra Models")
    parser.add_argument('--model', required=True, help='Path to fine-tuned Asthra model')
    parser.add_argument('--host', default='0.0.0.0', help='Server host')
    parser.add_argument('--port', type=int, default=8000, help='Server port')
    parser.add_argument('--workers', type=int, default=1, help='Number of workers')
    parser.add_argument('--max-context-length', type=int, default=30720, help='Maximum context length')
    
    args = parser.parse_args()
    
    # Validate model path
    if not os.path.exists(args.model):
        logger.error(f"❌ Model path not found: {args.model}")
        sys.exit(1)
    
    logger.info("🚀 Starting Asthra FastAPI OpenAI-Compatible Server")
    logger.info("=" * 50)
    logger.info(f"Model: {args.model}")
    logger.info(f"Server: http://{args.host}:{args.port}")
    logger.info(f"Max context: {args.max_context_length}")
    
    # Initialize model server
    global model_server
    config = {
        'max_context_length': args.max_context_length
    }
    
    # Set environment variables for the lifespan function
    os.environ['ASTHRA_MODEL_PATH'] = args.model
    os.environ['ASTHRA_MAX_CONTEXT'] = str(args.max_context_length)
    
    # Model will be initialized in startup event
    logger.info("📝 Model configuration set, will initialize on startup...")
    
    # Start server
    uvicorn.run(
        "fastapi_server:app",
        host=args.host,
        port=args.port,
        workers=args.workers,
        log_level="info"
    )

if __name__ == '__main__':
    main() 
