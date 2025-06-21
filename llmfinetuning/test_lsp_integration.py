#!/usr/bin/env python3
"""
Test script for Asthra LSP integration
Verifies that the LSP server can communicate with the model server
"""

import asyncio
import json
import socket
import time
import sys
from typing import Dict, Any

class LSPTestClient:
    """Simple LSP test client to verify server functionality."""
    
    def __init__(self, host: str = "127.0.0.1", port: int = 9000):
        self.host = host
        self.port = port
        self.reader = None
        self.writer = None
        self.msg_id = 0
    
    async def connect(self) -> bool:
        """Connect to LSP server."""
        try:
            self.reader, self.writer = await asyncio.open_connection(self.host, self.port)
            print(f"✅ Connected to LSP server at {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"❌ Failed to connect to LSP server: {e}")
            return False
    
    async def disconnect(self):
        """Disconnect from LSP server."""
        if self.writer:
            self.writer.close()
            await self.writer.wait_closed()
            print("📡 Disconnected from LSP server")
    
    def create_message(self, method: str, params: Dict[str, Any] = None) -> str:
        """Create an LSP message."""
        self.msg_id += 1
        message = {
            "jsonrpc": "2.0",
            "id": self.msg_id,
            "method": method
        }
        if params:
            message["params"] = params
        
        content = json.dumps(message)
        return f"Content-Length: {len(content)}\r\n\r\n{content}"
    
    async def send_message(self, method: str, params: Dict[str, Any] = None) -> Dict[str, Any]:
        """Send LSP message and get response."""
        if not self.writer:
            raise RuntimeError("Not connected to LSP server")
        
        message = self.create_message(method, params)
        self.writer.write(message.encode())
        await self.writer.drain()
        
        # Read response
        header = await self.reader.readuntil(b'\r\n\r\n')
        header_str = header.decode()
        
        # Parse content length
        content_length = 0
        for line in header_str.split('\r\n'):
            if line.startswith('Content-Length:'):
                content_length = int(line.split(':')[1].strip())
                break
        
        if content_length > 0:
            content = await self.reader.read(content_length)
            return json.loads(content.decode())
        
        return {}
    
    async def test_initialize(self) -> bool:
        """Test LSP initialize request."""
        print("🔧 Testing LSP initialization...")
        
        params = {
            "processId": None,
            "clientInfo": {
                "name": "asthra-lsp-test",
                "version": "1.0.0"
            },
            "rootUri": None,
            "capabilities": {
                "textDocument": {
                    "completion": {
                        "completionItem": {
                            "snippetSupport": True
                        }
                    },
                    "hover": {
                        "contentFormat": ["markdown", "plaintext"]
                    }
                }
            }
        }
        
        try:
            response = await self.send_message("initialize", params)
            if "result" in response:
                print("✅ LSP initialization successful")
                return True
            else:
                print(f"❌ LSP initialization failed: {response}")
                return False
        except Exception as e:
            print(f"❌ LSP initialization error: {e}")
            return False
    
    async def test_completion(self) -> bool:
        """Test code completion."""
        print("💡 Testing code completion...")
        
        # First, open a document
        doc_params = {
            "textDocument": {
                "uri": "file:///test.asthra",
                "languageId": "asthra",
                "version": 1,
                "text": "pub fn test_function() -> i32 {\n    let x: i32 = "
            }
        }
        
        try:
            # Send document open notification
            message = {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": doc_params
            }
            content = json.dumps(message)
            notification = f"Content-Length: {len(content)}\r\n\r\n{content}"
            self.writer.write(notification.encode())
            await self.writer.drain()
            
            # Wait a bit for processing
            await asyncio.sleep(1)
            
            # Request completion
            completion_params = {
                "textDocument": {
                    "uri": "file:///test.asthra"
                },
                "position": {
                    "line": 1,
                    "character": 17
                }
            }
            
            response = await self.send_message("textDocument/completion", completion_params)
            
            if "result" in response and "items" in response["result"]:
                items = response["result"]["items"]
                print(f"✅ Code completion successful ({len(items)} items)")
                if items:
                    print(f"   Sample completion: {items[0].get('label', 'N/A')}")
                return True
            else:
                print(f"❌ Code completion failed: {response}")
                return False
                
        except Exception as e:
            print(f"❌ Code completion error: {e}")
            return False
    
    async def test_hover(self) -> bool:
        """Test hover information."""
        print("🔍 Testing hover information...")
        
        hover_params = {
            "textDocument": {
                "uri": "file:///test.asthra"
            },
            "position": {
                "line": 1,
                "character": 8
            }
        }
        
        try:
            response = await self.send_message("textDocument/hover", hover_params)
            
            if "result" in response:
                if response["result"]:
                    print("✅ Hover information successful")
                    contents = response["result"].get("contents", {})
                    if isinstance(contents, dict) and "value" in contents:
                        print(f"   Hover content: {contents['value'][:100]}...")
                else:
                    print("✅ Hover request successful (no content for position)")
                return True
            else:
                print(f"❌ Hover failed: {response}")
                return False
                
        except Exception as e:
            print(f"❌ Hover error: {e}")
            return False


def check_server_running(host: str = "127.0.0.1", port: int = 9000) -> bool:
    """Check if LSP server is running."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(1)
        result = sock.connect_ex((host, port))
        sock.close()
        return result == 0
    except:
        return False


def check_model_server_running(host: str = "localhost", port: int = 8000) -> bool:
    """Check if model server is running."""
    try:
        import requests
        response = requests.get(f"http://{host}:{port}/health", timeout=5)
        return response.status_code == 200
    except:
        return False


async def main():
    """Main test function."""
    print("🚀 Asthra LSP Integration Test")
    print("=" * 40)
    
    # Check if servers are running
    print("📡 Checking server status...")
    
    model_running = check_model_server_running()
    lsp_running = check_server_running()
    
    print(f"   Model server (8000): {'✅ Running' if model_running else '❌ Not running'}")
    print(f"   LSP server (9000):   {'✅ Running' if lsp_running else '❌ Not running'}")
    
    if not model_running:
        print("\n⚠️  Model server not running. Start with:")
        print("   python servers/fastapi_server.py --model ./models/asthra-gemma3-4b")
    
    if not lsp_running:
        print("\n⚠️  LSP server not running. Start with:")
        print("   python servers/lsp_server.py")
        return False
    
    # Test LSP functionality
    print("\n🔧 Testing LSP functionality...")
    
    client = LSPTestClient()
    
    try:
        # Connect to LSP server
        if not await client.connect():
            return False
        
        # Run tests
        tests = [
            ("Initialize", client.test_initialize()),
            ("Completion", client.test_completion()),
            ("Hover", client.test_hover())
        ]
        
        results = []
        for test_name, test_coro in tests:
            print(f"\n📝 Running {test_name} test...")
            result = await test_coro
            results.append((test_name, result))
        
        # Summary
        print("\n📊 Test Results:")
        print("-" * 20)
        passed = 0
        for test_name, result in results:
            status = "✅ PASS" if result else "❌ FAIL"
            print(f"   {test_name}: {status}")
            if result:
                passed += 1
        
        total = len(results)
        print(f"\nOverall: {passed}/{total} tests passed ({passed/total*100:.1f}%)")
        
        if passed == total:
            print("\n🎉 All tests passed! LSP integration is working correctly.")
            return True
        else:
            print("\n⚠️  Some tests failed. Check server logs for details.")
            return False
            
    except Exception as e:
        print(f"\n❌ Test error: {e}")
        return False
    
    finally:
        await client.disconnect()


if __name__ == "__main__":
    success = asyncio.run(main())
    sys.exit(0 if success else 1) 
