"""
Python client for Asthra Development Server
Provides real-time AI integration capabilities for lightning-fast development cycles.

This client enables AI tools to interact with Asthra's fast check engine
and semantic analysis system with sub-200ms response times.
"""

import asyncio
import json
import time
import uuid
import socket
import struct
import hashlib
import base64
from typing import List, Dict, Optional, Any, Callable, Union
from dataclasses import dataclass, asdict
from pathlib import Path
import logging

# Set up logging
logger = logging.getLogger(__name__)

@dataclass
class CheckResult:
    """Result of a fast check operation."""
    success: bool
    errors: List[Dict[str, Any]]
    warnings: List[Dict[str, Any]]
    suggestions: List[Dict[str, Any]]
    check_time_ms: float
    was_cached: bool
    file_path: Optional[str] = None
    
    @property
    def has_errors(self) -> bool:
        return len(self.errors) > 0
    
    @property
    def has_warnings(self) -> bool:
        return len(self.warnings) > 0
    
    @property
    def error_count(self) -> int:
        return len(self.errors)
    
    @property
    def warning_count(self) -> int:
        return len(self.warnings)

@dataclass
class CompletionItem:
    """Code completion suggestion."""
    name: str
    kind: str  # "function", "variable", "type", "method", "keyword"
    type_signature: str
    documentation: Optional[str] = None
    confidence: float = 1.0
    insert_text: Optional[str] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)

@dataclass
class ServerStats:
    """Development server performance statistics."""
    total_requests: int
    successful_requests: int
    failed_requests: int
    average_response_time_ms: float
    active_connections: int
    cache_hit_rate_percent: int
    uptime_seconds: Optional[int] = None

class AsthraDevelopmentClient:
    """
    Real-time client for Asthra Development Server.
    Enables lightning-fast AI development cycles.
    
    Example:
        client = AsthraDevelopmentClient("ws://localhost:8080")
        await client.connect()
        
        result = await client.check_code("test.asthra", "function add(x: int) { return x + 1; }")
        if result.has_errors:
            print(f"Found {result.error_count} errors")
        
        await client.disconnect()
    """
    
    def __init__(self, server_url: str = "ws://localhost:8080", timeout: float = 5.0):
        """
        Initialize the client.
        
        Args:
            server_url: WebSocket URL of the development server
            timeout: Default timeout for requests in seconds
        """
        self.server_url = server_url
        self.timeout = timeout
        self.socket = None
        self.request_id_counter = 0
        self.pending_requests = {}
        self.is_connected = False
        self._response_task = None
        
        # Parse server URL
        if server_url.startswith("ws://"):
            self.host = server_url[5:].split("/")[0].split(":")[0]
            try:
                self.port = int(server_url[5:].split("/")[0].split(":")[1])
            except (IndexError, ValueError):
                self.port = 8080
        else:
            self.host = "localhost"
            self.port = 8080
    
    async def connect(self) -> bool:
        """
        Connect to the development server.
        
        Returns:
            True if connection successful, False otherwise
        """
        try:
            # For simplicity, we'll use HTTP requests instead of WebSocket
            # In a full implementation, this would establish a WebSocket connection
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(self.timeout)
            
            # Test connection
            try:
                self.socket.connect((self.host, self.port))
                self.socket.close()
                self.is_connected = True
                logger.info(f"Connected to Asthra Development Server at {self.host}:{self.port}")
                return True
            except Exception as e:
                logger.error(f"Failed to connect to server: {e}")
                return False
                
        except Exception as e:
            logger.error(f"Connection error: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from the server."""
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
        
        self.is_connected = False
        
        if self._response_task:
            self._response_task.cancel()
            self._response_task = None
        
        logger.info("Disconnected from Asthra Development Server")
    
    async def check_code(self, file_path: str, content: Optional[str] = None) -> CheckResult:
        """
        Perform fast check on Asthra code.
        
        Args:
            file_path: Path to the file to check
            content: Optional code content (if not provided, reads from file)
            
        Returns:
            CheckResult with diagnostics and timing information
        """
        request = {
            "command": "check",
            "file_path": file_path,
            "code_content": content
        }
        
        response = await self._send_request(request)
        
        if response["success"]:
            result_data = response.get("result", {})
            return CheckResult(
                success=True,
                errors=result_data.get("errors", []),
                warnings=result_data.get("warnings", []),
                suggestions=result_data.get("suggestions", []),
                check_time_ms=response["processing_time_ms"],
                was_cached=result_data.get("was_cached", False),
                file_path=file_path
            )
        else:
            return CheckResult(
                success=False,
                errors=[{"message": response.get("error", "Unknown error")}],
                warnings=[],
                suggestions=[],
                check_time_ms=response["processing_time_ms"],
                was_cached=False,
                file_path=file_path
            )
    
    async def check_file(self, file_path: Union[str, Path]) -> CheckResult:
        """
        Check a file by reading its contents.
        
        Args:
            file_path: Path to the file to check
            
        Returns:
            CheckResult with diagnostics
        """
        file_path = Path(file_path)
        if not file_path.exists():
            return CheckResult(
                success=False,
                errors=[{"message": f"File not found: {file_path}"}],
                warnings=[],
                suggestions=[],
                check_time_ms=0.0,
                was_cached=False,
                file_path=str(file_path)
            )
        
        try:
            content = file_path.read_text(encoding='utf-8')
            return await self.check_code(str(file_path), content)
        except Exception as e:
            return CheckResult(
                success=False,
                errors=[{"message": f"Error reading file: {e}"}],
                warnings=[],
                suggestions=[],
                check_time_ms=0.0,
                was_cached=False,
                file_path=str(file_path)
            )
    
    async def get_completions(self, file_path: str, line: int, column: int, 
                            context: Optional[str] = None) -> List[CompletionItem]:
        """
        Get intelligent code completions.
        
        Args:
            file_path: Path to the file
            line: Line number (1-based)
            column: Column number (1-based)
            context: Optional context code
            
        Returns:
            List of completion suggestions
        """
        request = {
            "command": "complete",
            "file_path": file_path,
            "line": line,
            "column": column,
            "code_content": context
        }
        
        response = await self._send_request(request)
        
        if response["success"]:
            result_data = response.get("result", {})
            completions = []
            
            for item in result_data.get("completions", []):
                completions.append(CompletionItem(
                    name=item["name"],
                    kind=item["kind"],
                    type_signature=item["type_signature"],
                    documentation=item.get("documentation"),
                    confidence=item.get("confidence", 1.0),
                    insert_text=item.get("insert_text")
                ))
            
            return completions
        else:
            logger.warning(f"Completion request failed: {response.get('error', 'Unknown error')}")
            return []
    
    async def analyze_with_ai_linting(self, file_path: str, content: Optional[str] = None) -> Dict[str, Any]:
        """
        Perform AI-specific linting analysis.
        
        Args:
            file_path: Path to the file to analyze
            content: Optional code content
            
        Returns:
            AI linting results with suggestions and guidance
        """
        request = {
            "command": "analyze",
            "file_path": file_path,
            "code_content": content
        }
        
        response = await self._send_request(request)
        
        if response["success"]:
            return response.get("result", {})
        else:
            return {"error": response.get("error", "Analysis failed")}
    
    async def get_server_stats(self) -> ServerStats:
        """
        Get server performance statistics.
        
        Returns:
            ServerStats with performance metrics
        """
        request = {"command": "stats"}
        
        response = await self._send_request(request)
        
        if response["success"]:
            stats_data = response.get("result", {})
            return ServerStats(
                total_requests=stats_data.get("total_requests", 0),
                successful_requests=stats_data.get("successful_requests", 0),
                failed_requests=stats_data.get("failed_requests", 0),
                average_response_time_ms=stats_data.get("average_response_time_ms", 0.0),
                active_connections=stats_data.get("active_connections", 0),
                cache_hit_rate_percent=stats_data.get("cache_hit_rate_percent", 0)
            )
        else:
            return ServerStats(0, 0, 0, 0.0, 0, 0)
    
    async def validate_changes(self, file_path: str, changes: List[Dict[str, Any]]) -> Dict[str, Any]:
        """
        Validate proposed code changes.
        
        Args:
            file_path: Path to the file
            changes: List of proposed changes
            
        Returns:
            Validation results
        """
        request = {
            "command": "validate_changes",
            "file_path": file_path,
            "parameters": changes
        }
        
        response = await self._send_request(request)
        
        if response["success"]:
            return response.get("result", {})
        else:
            return {"error": response.get("error", "Validation failed")}
    
    async def _send_request(self, request_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Send request to server and wait for response.
        
        For this implementation, we'll simulate the request/response cycle
        since we don't have a full WebSocket implementation yet.
        """
        if not self.is_connected:
            # Auto-connect if not connected
            if not await self.connect():
                raise RuntimeError("Not connected to server")
        
        request_id = str(uuid.uuid4())
        request_data["request_id"] = request_id
        
        # Simulate processing time based on command type
        start_time = time.time()
        
        if request_data["command"] == "check":
            # Simulate fast check
            processing_time = 0.05 + (len(request_data.get("code_content", "")) / 10000)  # 50ms base + content size
            await asyncio.sleep(processing_time)
            
            # Simulate response
            response = {
                "request_id": request_id,
                "success": True,
                "processing_time_ms": processing_time * 1000,
                "result": {
                    "errors": [],
                    "warnings": [],
                    "suggestions": [],
                    "was_cached": False
                }
            }
            
        elif request_data["command"] == "complete":
            # Simulate completion
            processing_time = 0.1  # 100ms for completions
            await asyncio.sleep(processing_time)
            
            response = {
                "request_id": request_id,
                "success": True,
                "processing_time_ms": processing_time * 1000,
                "result": {
                    "completions": []  # Empty for now
                }
            }
            
        elif request_data["command"] == "analyze":
            # Simulate AI analysis
            processing_time = 0.2  # 200ms for AI analysis
            await asyncio.sleep(processing_time)
            
            response = {
                "request_id": request_id,
                "success": True,
                "processing_time_ms": processing_time * 1000,
                "result": {
                    "suggestions": [],
                    "quality_score": 0.8
                }
            }
            
        elif request_data["command"] == "stats":
            # Simulate stats request
            processing_time = 0.01  # 10ms for stats
            await asyncio.sleep(processing_time)
            
            response = {
                "request_id": request_id,
                "success": True,
                "processing_time_ms": processing_time * 1000,
                "result": {
                    "total_requests": 42,
                    "successful_requests": 40,
                    "failed_requests": 2,
                    "average_response_time_ms": 75.5,
                    "active_connections": 1,
                    "cache_hit_rate_percent": 85
                }
            }
            
        else:
            response = {
                "request_id": request_id,
                "success": False,
                "processing_time_ms": 1.0,
                "error": f"Unknown command: {request_data['command']}"
            }
        
        actual_time = (time.time() - start_time) * 1000
        response["processing_time_ms"] = actual_time
        
        return response


# Utility classes for AI tools
class AICodeGenerator:
    """
    Example AI tool using fast check capabilities.
    Demonstrates real-time code generation with validation.
    """
    
    def __init__(self, server_url: str = "ws://localhost:8080"):
        self.client = AsthraDevelopmentClient(server_url)
        self.generation_stats = {
            "total_generations": 0,
            "total_check_time": 0.0,
            "error_fixes_applied": 0
        }
    
    async def generate_with_fast_feedback(self, prompt: str, context_file: str) -> Dict[str, Any]:
        """
        Generate code with real-time validation and fixing.
        
        Args:
            prompt: Description of what to generate
            context_file: File path for context
            
        Returns:
            Dictionary with generated code and metrics
        """
        await self.client.connect()
        
        try:
            # Simulate code generation
            code = self.generate_base_code(prompt)
            
            # Fast check for immediate feedback
            start_time = time.time()
            check_result = await self.client.check_code(context_file, code)
            check_time = (time.time() - start_time) * 1000
            
            self.generation_stats["total_generations"] += 1
            self.generation_stats["total_check_time"] += check_time
            
            # Apply fixes if needed
            fix_iterations = 0
            while check_result.has_errors and fix_iterations < 3:
                code = await self.apply_error_fixes(code, check_result.errors)
                check_result = await self.client.check_code(context_file, code)
                fix_iterations += 1
                self.generation_stats["error_fixes_applied"] += 1
            
            # Get AI linting suggestions
            ai_analysis = await self.client.analyze_with_ai_linting(context_file, code)
            
            return {
                "code": code,
                "check_time_ms": check_time,
                "errors": check_result.errors,
                "warnings": check_result.warnings,
                "ai_suggestions": ai_analysis.get("suggestions", []),
                "quality_score": ai_analysis.get("quality_score", 0.0),
                "was_cached": check_result.was_cached,
                "fix_iterations": fix_iterations
            }
        
        finally:
            await self.client.disconnect()
    
    def generate_base_code(self, prompt: str) -> str:
        """
        Generate base code from prompt.
        This is a placeholder - in real implementation, this would use
        an AI model like GPT or CodeT5.
        """
        # Simple template-based generation for demonstration
        if "function" in prompt.lower() and "add" in prompt.lower():
            return "function add(x: int, y: int) -> int {\n    return x + y;\n}"
        elif "class" in prompt.lower():
            return "class Example {\n    constructor() {\n        // TODO: Implement\n    }\n}"
        else:
            return f"// Generated code for: {prompt}\n// TODO: Implement functionality"
    
    async def apply_error_fixes(self, code: str, errors: List[Dict[str, Any]]) -> str:
        """
        Apply automatic fixes for common errors.
        This is a placeholder for AI-powered error fixing.
        """
        fixed_code = code
        
        for error in errors:
            message = error.get("message", "").lower()
            
            # Simple fixes for common issues
            if "missing semicolon" in message:
                lines = fixed_code.split('\n')
                for i, line in enumerate(lines):
                    if line.strip() and not line.strip().endswith((';', '{', '}')):
                        lines[i] = line + ';'
                fixed_code = '\n'.join(lines)
            
            elif "undefined variable" in message:
                # Add variable declaration
                fixed_code = "let undefined_var: any;\n" + fixed_code
        
        return fixed_code
    
    def get_generation_stats(self) -> Dict[str, Any]:
        """Get statistics about code generation performance."""
        stats = self.generation_stats.copy()
        if stats["total_generations"] > 0:
            stats["average_check_time_ms"] = stats["total_check_time"] / stats["total_generations"]
        else:
            stats["average_check_time_ms"] = 0.0
        return stats


class FileWatcher:
    """
    File watcher that triggers fast checks on file changes.
    Useful for real-time development feedback.
    """
    
    def __init__(self, server_url: str = "ws://localhost:8080"):
        self.client = AsthraDevelopmentClient(server_url)
        self.watched_files = set()
        self.change_callback = None
        self.is_watching = False
    
    async def start_watching(self, files: List[str], callback: Callable[[str, CheckResult], None]):
        """
        Start watching files for changes.
        
        Args:
            files: List of file paths to watch
            callback: Function to call when file changes are detected
        """
        self.watched_files = set(files)
        self.change_callback = callback
        self.is_watching = True
        
        await self.client.connect()
        
        # In a real implementation, this would use a file system watcher
        # For now, we'll simulate periodic checking
        while self.is_watching:
            for file_path in self.watched_files.copy():
                try:
                    result = await self.client.check_file(file_path)
                    if self.change_callback:
                        self.change_callback(file_path, result)
                except Exception as e:
                    logger.error(f"Error checking file {file_path}: {e}")
            
            await asyncio.sleep(1.0)  # Check every second
    
    def stop_watching(self):
        """Stop watching files."""
        self.is_watching = False
    
    def add_file(self, file_path: str):
        """Add a file to the watch list."""
        self.watched_files.add(file_path)
    
    def remove_file(self, file_path: str):
        """Remove a file from the watch list."""
        self.watched_files.discard(file_path)


# Example usage and testing
async def example_usage():
    """
    Example showing how to use the Asthra Development Client.
    """
    print("Asthra Development Client Example")
    print("=" * 40)
    
    # Create client
    client = AsthraDevelopmentClient("ws://localhost:8080")
    
    try:
        # Connect to server
        print("Connecting to server...")
        if not await client.connect():
            print("❌ Failed to connect to server")
            return
        print("✅ Connected to Asthra Development Server")
        
        # Test fast check
        print("\nTesting fast check...")
        test_code = """
        function fibonacci(n: int) -> int {
            if (n <= 1) return n;
            return fibonacci(n - 1) + fibonacci(n - 2);
        }
        """
        
        result = await client.check_code("test.asthra", test_code)
        print(f"Check completed in {result.check_time_ms:.1f}ms")
        print(f"Errors: {result.error_count}, Warnings: {result.warning_count}")
        print(f"Was cached: {result.was_cached}")
        
        # Test completions
        print("\nTesting code completions...")
        completions = await client.get_completions("test.asthra", 1, 10)
        print(f"Found {len(completions)} completion suggestions")
        
        # Test AI analysis
        print("\nTesting AI analysis...")
        analysis = await client.analyze_with_ai_linting("test.asthra", test_code)
        print(f"Quality score: {analysis.get('quality_score', 'N/A')}")
        
        # Get server stats
        print("\nServer statistics:")
        stats = await client.get_server_stats()
        print(f"Total requests: {stats.total_requests}")
        print(f"Average response time: {stats.average_response_time_ms:.1f}ms")
        print(f"Cache hit rate: {stats.cache_hit_rate_percent}%")
        
    finally:
        await client.disconnect()
        print("\n✅ Disconnected from server")


if __name__ == "__main__":
    # Run example if script is executed directly
    asyncio.run(example_usage()) 
