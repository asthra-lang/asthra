#!/bin/bash
# Asthra OpenAI-Compatible API Server Startup Script

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Default values
SERVER_TYPE="fastapi"
MODEL_PATH="./outputs/asthra-gemma3-4b-v1"
HOST="0.0.0.0"
PORT="8000"
LSP_PORT="9000"
START_LSP=false
LSP_ONLY=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            SERVER_TYPE="$2"
            shift 2
            ;;
        -m|--model)
            MODEL_PATH="$2"
            shift 2
            ;;
        -h|--host)
            HOST="$2"
            shift 2
            ;;
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        --lsp-port)
            LSP_PORT="$2"
            shift 2
            ;;
        --with-lsp)
            START_LSP=true
            shift
            ;;
        --lsp-only)
            LSP_ONLY=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -t, --type TYPE     Server type: fastapi, vllm, lsp (default: fastapi)"
            echo "  -m, --model PATH    Path to model (default: ./outputs/asthra-gemma3-4b-v1)"
            echo "  -h, --host HOST     Server host (default: 0.0.0.0)"
            echo "  -p, --port PORT     Server port (default: 8000)"
            echo "  --lsp-port PORT     LSP server port (default: 9000)"
            echo "  --with-lsp          Start LSP server alongside model server"
            echo "  --lsp-only          Start only the LSP server"
            echo ""
            echo "Examples:"
            echo "  $0 --with-lsp                     # Start model + LSP servers"
            echo "  $0 --type vllm --model ./models   # Start vLLM server"
            echo "  $0 --lsp-only                     # Start only LSP server"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Function to start LSP server in background
start_lsp_server() {
    print_info "Starting Asthra LSP server on port $LSP_PORT..."
    python servers/lsp_server.py \
        --host 127.0.0.1 \
        --port "$LSP_PORT" \
        --model-api "http://$HOST:$PORT/v1" &
    LSP_PID=$!
    print_success "LSP server started (PID: $LSP_PID)"
    echo $LSP_PID > .lsp.pid
}

# Function to cleanup on exit
cleanup() {
    if [ -f .lsp.pid ]; then
        LSP_PID=$(cat .lsp.pid)
        print_info "Stopping LSP server (PID: $LSP_PID)..."
        kill $LSP_PID 2>/dev/null || true
        rm -f .lsp.pid
    fi
    exit
}

# Set up signal handlers
trap cleanup INT TERM

# Handle LSP-only mode
if [ "$LSP_ONLY" = true ]; then
    print_info "Starting Asthra LSP server only"
    print_info "LSP Server: tcp://127.0.0.1:$LSP_PORT"
    print_info "Model API: http://localhost:8000/v1 (assumed running)"
    
    # Activate virtual environment if it exists
    if [ -d ".venv" ]; then
        source .venv/bin/activate
        print_success "Virtual environment activated"
    fi
    
    python servers/lsp_server.py \
        --host 127.0.0.1 \
        --port "$LSP_PORT" \
        --model-api "http://localhost:8000/v1"
    exit 0
fi

print_info "Starting Asthra OpenAI-Compatible API Server"
print_info "Type: $SERVER_TYPE"
print_info "Model: $MODEL_PATH"
print_info "Server: http://$HOST:$PORT"

if [ "$START_LSP" = true ]; then
    print_info "LSP Server: tcp://127.0.0.1:$LSP_PORT"
fi

# Check if model exists (not needed for LSP-only)
if [ ! -d "$MODEL_PATH" ] && [ "$SERVER_TYPE" != "lsp" ]; then
    print_error "Model directory not found: $MODEL_PATH"
    exit 1
fi

# Activate virtual environment if it exists
if [ -d ".venv" ]; then
    source .venv/bin/activate
    print_success "Virtual environment activated"
fi

# Start appropriate server
case $SERVER_TYPE in
    fastapi)
        print_info "Starting FastAPI server..."
        if [ "$START_LSP" = true ]; then
            # Start model server in background, then LSP server
            python servers/fastapi_server.py \
                --model "$MODEL_PATH" \
                --host "$HOST" \
                --port "$PORT" &
            MODEL_PID=$!
            sleep 3  # Wait for model server to start
            start_lsp_server
            wait $MODEL_PID  # Wait for model server to finish
        else
            python servers/fastapi_server.py \
                --model "$MODEL_PATH" \
                --host "$HOST" \
                --port "$PORT"
        fi
        ;;
    vllm)
        print_info "Starting vLLM server..."
        if [ "$START_LSP" = true ]; then
            # Start model server in background, then LSP server
            python servers/vllm_server.py \
                --model "$MODEL_PATH" \
                --host "$HOST" \
                --port "$PORT" &
            MODEL_PID=$!
            sleep 5  # Wait for vLLM server to start (takes longer)
            start_lsp_server
            wait $MODEL_PID  # Wait for model server to finish
        else
            python servers/vllm_server.py \
                --model "$MODEL_PATH" \
                --host "$HOST" \
                --port "$PORT"
        fi
        ;;
    lsp)
        print_info "Starting LSP server only..."
        python servers/lsp_server.py \
            --host 127.0.0.1 \
            --port "$LSP_PORT" \
            --model-api "http://localhost:8000/v1"
        ;;
    *)
        print_error "Invalid server type: $SERVER_TYPE"
        exit 1
        ;;
esac 
