#!/usr/bin/env python3
"""
Test client for Asthra Development Server
Demonstrates real-time AI integration capabilities
"""

import asyncio
import sys
import time
from pathlib import Path

# Add current directory to path so we can import python_client
sys.path.insert(0, str(Path(__file__).parent))

from python_client import (
    AsthraDevelopmentClient, 
    AICodeGenerator, 
    FileWatcher, 
    CheckResult
)

async def test_basic_functionality():
    """Test basic development server functionality."""
    print("🚀 Testing Asthra Development Server Basic Functionality")
    print("=" * 60)
    
    client = AsthraDevelopmentClient("ws://localhost:8080")
    
    try:
        # Test connection
        print("1. Testing connection...")
        connected = await client.connect()
        if connected:
            print("✅ Connected to development server")
        else:
            print("❌ Failed to connect - server may not be running")
            return False
        
        # Test fast check with valid code
        print("\n2. Testing fast check with valid code...")
        valid_code = """
        function factorial(n: int) -> int {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }
        """
        
        start_time = time.time()
        result = await client.check_code("test_valid.asthra", valid_code)
        check_time = (time.time() - start_time) * 1000
        
        print(f"✅ Check completed in {check_time:.1f}ms (simulated: {result.check_time_ms:.1f}ms)")
        print(f"   Errors: {result.error_count}, Warnings: {result.warning_count}")
        print(f"   Success: {result.success}, Cached: {result.was_cached}")
        
        # Test fast check with invalid code
        print("\n3. Testing fast check with invalid code...")
        invalid_code = """
        function broken_function( {
            return undefined_variable + 1
        }
        """
        
        result = await client.check_code("test_invalid.asthra", invalid_code)
        print(f"✅ Invalid code check completed in {result.check_time_ms:.1f}ms")
        print(f"   Errors: {result.error_count}, Warnings: {result.warning_count}")
        print(f"   Success: {result.success}")
        
        # Test code completions
        print("\n4. Testing code completions...")
        completions = await client.get_completions("test.asthra", 5, 10, valid_code)
        print(f"✅ Found {len(completions)} completion suggestions")
        
        # Test AI analysis
        print("\n5. Testing AI analysis...")
        analysis = await client.analyze_with_ai_linting("test.asthra", valid_code)
        print(f"✅ AI analysis completed")
        print(f"   Quality score: {analysis.get('quality_score', 'N/A')}")
        print(f"   Suggestions: {len(analysis.get('suggestions', []))}")
        
        # Test server statistics
        print("\n6. Testing server statistics...")
        stats = await client.get_server_stats()
        print(f"✅ Server stats retrieved")
        print(f"   Total requests: {stats.total_requests}")
        print(f"   Success rate: {stats.successful_requests}/{stats.total_requests}")
        print(f"   Average response time: {stats.average_response_time_ms:.1f}ms")
        print(f"   Cache hit rate: {stats.cache_hit_rate_percent}%")
        
        return True
        
    finally:
        await client.disconnect()
        print("\n✅ Disconnected from server")

async def test_ai_code_generator():
    """Test AI code generator with fast feedback."""
    print("\n🤖 Testing AI Code Generator with Fast Feedback")
    print("=" * 60)
    
    generator = AICodeGenerator("ws://localhost:8080")
    
    # Test various code generation scenarios
    test_prompts = [
        "Create a function that adds two numbers",
        "Create a class for managing user accounts",
        "Generate a function to calculate fibonacci numbers"
    ]
    
    for i, prompt in enumerate(test_prompts, 1):
        print(f"\n{i}. Generating: {prompt}")
        
        start_time = time.time()
        result = await generator.generate_with_fast_feedback(prompt, "context.asthra")
        total_time = (time.time() - start_time) * 1000
        
        print(f"✅ Generated in {total_time:.1f}ms total")
        print(f"   Check time: {result['check_time_ms']:.1f}ms")
        print(f"   Errors: {len(result['errors'])}")
        print(f"   Fix iterations: {result['fix_iterations']}")
        print(f"   Quality score: {result['quality_score']}")
        print(f"   Was cached: {result['was_cached']}")
        
        # Show generated code (first 100 characters)
        code_preview = result['code'].replace('\n', ' ')[:100]
        print(f"   Code preview: {code_preview}...")
    
    # Show generation statistics
    stats = generator.get_generation_stats()
    print(f"\n📊 Generation Statistics:")
    print(f"   Total generations: {stats['total_generations']}")
    print(f"   Average check time: {stats['average_check_time_ms']:.1f}ms")
    print(f"   Error fixes applied: {stats['error_fixes_applied']}")

async def test_file_watcher():
    """Test file watcher for real-time feedback."""
    print("\n👁️  Testing File Watcher (5 second demo)")
    print("=" * 60)
    
    # Create test files
    test_files = []
    for i in range(3):
        file_path = f"test_watch_{i}.asthra"
        test_files.append(file_path)
        with open(file_path, 'w') as f:
            f.write(f"// Test file {i}\nfunction test_{i}() {{ return {i}; }}")
    
    print(f"Created {len(test_files)} test files for watching")
    
    # Callback for file changes
    change_count = 0
    def on_file_change(file_path: str, result: CheckResult):
        nonlocal change_count
        change_count += 1
        print(f"  📁 {file_path}: {result.error_count} errors, {result.warning_count} warnings ({result.check_time_ms:.1f}ms)")
    
    # Start watching
    watcher = FileWatcher("ws://localhost:8080")
    print("Starting file watcher...")
    
    # Watch for 5 seconds
    watch_task = asyncio.create_task(
        watcher.start_watching(test_files, on_file_change)
    )
    
    await asyncio.sleep(5.0)
    watcher.stop_watching()
    
    try:
        await asyncio.wait_for(watch_task, timeout=1.0)
    except asyncio.TimeoutError:
        pass
    
    print(f"✅ File watcher processed {change_count} file checks")
    
    # Clean up test files
    for file_path in test_files:
        try:
            Path(file_path).unlink()
        except:
            pass
    print("Cleaned up test files")

async def test_performance_benchmark():
    """Test performance characteristics."""
    print("\n⚡ Performance Benchmark")
    print("=" * 60)
    
    client = AsthraDevelopmentClient("ws://localhost:8080")
    await client.connect()
    
    try:
        # Test code of varying sizes
        test_cases = [
            ("Small", "function small() { return 1; }"),
            ("Medium", "function medium() {\n" + "    let x = 0;\n" * 50 + "    return x;\n}"),
            ("Large", "function large() {\n" + "    let array = [];\n" * 200 + "    return array;\n}")
        ]
        
        for name, code in test_cases:
            print(f"\nTesting {name} code ({len(code)} characters):")
            
            # Run multiple checks to test caching
            times = []
            for i in range(5):
                start_time = time.time()
                result = await client.check_code(f"test_{name.lower()}.asthra", code)
                check_time = (time.time() - start_time) * 1000
                times.append(check_time)
                
                cache_status = "cached" if result.was_cached else "fresh"
                print(f"  Check {i+1}: {check_time:.1f}ms ({cache_status})")
            
            avg_time = sum(times) / len(times)
            min_time = min(times)
            max_time = max(times)
            
            print(f"  📊 Average: {avg_time:.1f}ms, Min: {min_time:.1f}ms, Max: {max_time:.1f}ms")
        
        # Test concurrent requests
        print(f"\nTesting concurrent requests:")
        concurrent_tasks = []
        
        start_time = time.time()
        for i in range(10):
            task = client.check_code(f"concurrent_{i}.asthra", "function test() { return 42; }")
            concurrent_tasks.append(task)
        
        results = await asyncio.gather(*concurrent_tasks)
        total_time = (time.time() - start_time) * 1000
        
        successful_requests = sum(1 for r in results if r.success)
        avg_individual_time = sum(r.check_time_ms for r in results) / len(results)
        
        print(f"  ✅ {successful_requests}/{len(results)} requests successful")
        print(f"  ⏱️  Total time: {total_time:.1f}ms")
        print(f"  📊 Average individual time: {avg_individual_time:.1f}ms")
        print(f"  🚀 Throughput: {len(results) / (total_time / 1000):.1f} requests/second")
        
    finally:
        await client.disconnect()

async def main():
    """Run all tests."""
    print("🎯 Asthra Development Server - Real-Time AI Integration Test Suite")
    print("=" * 70)
    
    # Check if server is accessible
    client = AsthraDevelopmentClient("ws://localhost:8080")
    if not await client.connect():
        print("❌ Cannot connect to development server at localhost:8080")
        print("   Please ensure the server is running with: ./asthra-dev-server --test")
        return
    await client.disconnect()
    
    # Run test suites
    try:
        success = await test_basic_functionality()
        if not success:
            return
        
        await test_ai_code_generator()
        await test_file_watcher()
        await test_performance_benchmark()
        
        print("\n🎉 All tests completed successfully!")
        print("\n📊 Summary:")
        print("  ✅ Basic functionality: PASSED")
        print("  ✅ AI code generator: PASSED")
        print("  ✅ File watcher: PASSED")
        print("  ✅ Performance benchmark: PASSED")
        
        print("\n🚀 Asthra Development Server is ready for real-time AI integration!")
        
    except KeyboardInterrupt:
        print("\n⚠️  Tests interrupted by user")
    except Exception as e:
        print(f"\n❌ Test failed with error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    asyncio.run(main()) 
