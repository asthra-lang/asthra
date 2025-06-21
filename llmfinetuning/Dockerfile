# Multi-stage Dockerfile for Asthra OpenAI-Compatible API Server
# Supports both FastAPI and vLLM backends

FROM nvidia/cuda:12.1-devel-ubuntu22.04 as base

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV PYTHONUNBUFFERED=1
ENV PYTHONDONTWRITEBYTECODE=1

# Install system dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    git \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Create virtual environment
RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

# Copy requirements
COPY requirements.txt .

# Install Python dependencies
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Install additional server dependencies
RUN pip install --no-cache-dir \
    fastapi \
    uvicorn \
    pydantic \
    httpx \
    openai

# Stage for vLLM support (optional)
FROM base as vllm-support
RUN pip install --no-cache-dir vllm

# Final stage
FROM base as final

# Copy application code
COPY servers/ ./servers/
COPY configs/ ./configs/
COPY scripts/ ./scripts/
COPY examples/ ./examples/

# Create directories for models and logs
RUN mkdir -p /app/models /app/logs /app/outputs

# Set permissions
RUN chmod +x scripts/start_server.sh

# Expose port
EXPOSE 8000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=60s --retries=3 \
    CMD curl -f http://localhost:8000/health || exit 1

# Default command (FastAPI server)
CMD ["python", "servers/fastapi_server.py", "--model", "/app/models", "--host", "0.0.0.0", "--port", "8000"]

# Labels
LABEL maintainer="Asthra Team"
LABEL description="OpenAI-compatible API server for Asthra fine-tuned models"
LABEL version="1.0.0" 
