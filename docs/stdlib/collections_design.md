# Asthra Standard Library - Enhanced Collections Module Design

**Version**: 2.0  
**Status**: Design Complete  
**Dependencies**: `stdlib/asthra/core`, `stdlib/asthra/algorithms`  
**Target**: AI-Optimized Code Generation

## Overview

The Enhanced Collections module provides comprehensive data containers optimized for AI code generation, featuring robust error handling, memory efficiency, and seamless integration with Asthra's type system and memory management.

## Design Principles

### 1. AI-Friendly Code Generation
- **Predictable APIs**: Consistent naming patterns and error handling across all collections
- **Explicit Error Types**: Structured error enums for precise error handling
- **Memory Safety**: Automatic cleanup with explicit destroy functions
- **Type Safety**: Generic types with compile-time validation

### 2. Performance Optimization
- **Configurable Growth**: Customizable growth factors and shrink thresholds
- **Memory Efficiency**: Automatic shrinking and manual capacity management
- **Cache-Friendly**: Power-of-two sizing for hash tables
- **Security**: Random hash seeds to prevent hash collision attacks

### 3. Comprehensive Error Handling
- **Structured Errors**: Detailed error types with contextual information
- **Graceful Degradation**: Operations continue safely after recoverable errors
- **Resource Cleanup**: Guaranteed memory cleanup even after errors
- **Debug Support**: Modification counters for iterator invalidation detection

## Collection Types

### Enhanced Vector (`Vec<T>`)

**Purpose**: Dynamic array with configurable growth behavior and comprehensive operations.

**Key Features**:
- Configurable growth factor (default 2.0, recommended 1.5 for memory efficiency)
- Automatic shrinking when load factor drops below threshold
- Insert/remove at arbitrary positions
- Fast swap-remove for unordered data
- Slice conversion and extension operations
- Memory-efficient capacity management

**Performance Characteristics**:
- **Access**: O(1) random access by index
- **Append**: O(1) amortized, O(n) worst case during resize
- **Insert**: O(n) for arbitrary position, O(1) for end
- **Remove**: O(n) for arbitrary position, O(1) for swap-remove
- **Search**: O(n) linear search

**Memory Layout**:
```asthra
struct Vec<T> {
    data: *mut T,                    // Pointer to heap-allocated array
    length: usize,                   // Current number of elements
    capacity: usize,                 // Total allocated capacity
    element_size: usize,             // Size of each element
    modification_count: usize,       // For iterator invalidation detection
    growth_factor: f64,              // Configurable growth (default 2.0)
    shrink_threshold: f64            // When to shrink (default 0.25)
}
```

**Usage Patterns**:
```asthra
// AI-friendly configuration
let config = collections::VecConfig {
    initial_capacity: 16,
    growth_factor: 1.5,              // Memory-efficient growth
    shrink_threshold: 0.3,           // Shrink when 30% full
    enable_shrinking: true
};

let mut vec = collections::vec_with_config::<i32>(config);

// Comprehensive error handling
let insert_result = collections::vec_insert(&mut vec, 0, 42);
match insert_result {
    Result.Ok(_) => { /* Success */ },
    Result.Err(collections::CollectionError.OutOfBounds(index)) => {
        // Handle index error with specific index
    },
    Result.Err(collections::CollectionError.MemoryAllocation(msg)) => {
        // Handle memory allocation failure
    },
    Result.Err(_) => { /* Other errors */ }
}
```

### Hash Set (`HashSet<T>`)

**Purpose**: Unordered collection of unique elements with fast membership testing.

**Key Features**:
- Built on enhanced HashMap for consistency
- Automatic resizing with configurable load factors
- Set operations (union, intersection, difference)
- Creation from slices and other collections
- Security-hardened hash function with random seeds

**Performance Characteristics**:
- **Insert**: O(1) average, O(n) worst case
- **Remove**: O(1) average, O(n) worst case  
- **Contains**: O(1) average, O(n) worst case
- **Set Operations**: O(n + m) where n, m are set sizes

**Memory Layout**:
```asthra
struct HashSet<T> {
    map: HashMap<T, bool>,           // Internal HashMap with dummy values
    modification_count: usize        // Iterator invalidation detection
}
```

**Usage Patterns**:
```asthra
// Create from existing data
let items: []string = ["apple", "banana", "cherry"];
let set_result = collections::hashset_from_slice(items);

match set_result {
    Result.Ok(mut set) => {
        // Check membership
        if collections::hashset_contains(&set, &"apple") {
            // Element exists
        }
        
        // Insert with duplicate detection
        let insert_result = collections::hashset_insert(&mut set, "date");
        match insert_result {
            Result.Ok(was_new) => {
                if was_new {
                    // New element added
                } else {
                    // Element already existed
                }
            },
            Result.Err(_) => { /* Handle error */ }
        }
    },
    Result.Err(_) => { /* Handle creation error */ }
}
```

### Enhanced Hash Map (`HashMap<K, V>`)

**Purpose**: Key-value mapping with automatic resizing and collision handling.

**Key Features**:
- Power-of-two bucket sizing for optimal performance
- Cached hash values for faster rehashing
- Automatic resizing when load factor exceeds threshold
- Get-or-insert operations for default value patterns
- Security-hardened with random hash seeds
- Enhanced insert returning previous values

**Performance Characteristics**:
- **Insert**: O(1) average, O(n) worst case during resize
- **Get**: O(1) average, O(n) worst case
- **Remove**: O(1) average, O(n) worst case
- **Resize**: O(n) when triggered

**Memory Layout**:
```asthra
struct HashMap<K, V> {
    buckets: *mut HashBucket<K, V>,  // Array of hash buckets
    bucket_count: usize,             // Number of buckets (power of 2)
    size: usize,                     // Number of key-value pairs
    load_factor: f64,                // Current load factor
    max_load_factor: f64,            // Resize threshold (default 0.75)
    modification_count: usize,       // Iterator invalidation detection
    hash_seed: u64                   // Random seed for security
}

struct HashBucket<K, V> {
    key: K,
    value: V,
    next: *mut HashBucket<K, V>,     // Collision chain
    is_occupied: bool,
    hash_value: usize                // Cached hash for rehashing
}
```

**Usage Patterns**:
```asthra
// Enhanced insert with previous value detection
let insert_result = collections::hashmap_insert(&mut map, "score", 100);
match insert_result {
    Result.Ok(old_value_opt) => {
        match old_value_opt {
            Option.Some(old_value) => {
                // Key existed, old_value contains previous value
            },
            Option.None => {
                // New key inserted
            }
        }
    },
    Result.Err(_) => { /* Handle error */ }
}

// Get-or-insert pattern for default values
let value_result = collections::hashmap_get_or_insert(&mut map, "health", 100);
match value_result {
    Result.Ok(value_ptr) => {
        // value_ptr points to the value (existing or newly inserted)
        unsafe {
            *value_ptr = *value_ptr + 10;  // Modify in place
        }
    },
    Result.Err(_) => { /* Handle error */ }
}
```

### Enhanced Linked List (`LinkedList<T>`)

**Purpose**: Doubly-linked list optimized for insertions and deletions at arbitrary positions.

**Key Features**:
- Doubly-linked for bidirectional traversal
- Insert/remove at arbitrary positions
- Efficient front/back operations
- Find and contains operations
- Conversion to/from other collections

**Performance Characteristics**:
- **Push/Pop Front/Back**: O(1)
- **Insert/Remove at Position**: O(n) to find position, O(1) to modify
- **Find**: O(n) linear search
- **Access by Index**: O(n) traversal required

**Memory Layout**:
```asthra
struct LinkedList<T> {
    head: *mut ListNode<T>,          // First node
    tail: *mut ListNode<T>,          // Last node  
    length: usize,                   // Number of nodes
    modification_count: usize        // Iterator invalidation detection
}

struct ListNode<T> {
    data: T,
    next: *mut ListNode<T>,          // Forward link
    prev: *mut ListNode<T>           // Backward link
}
```

**Usage Patterns**:
```asthra
// Create from slice
let data: []string = ["first", "second", "third"];
let list_result = collections::list_from_slice(data);

match list_result {
    Result.Ok(mut list) => {
        // Insert at specific position
        let insert_result = collections::list_insert(&mut list, 1, "inserted");
        match insert_result {
            Result.Ok(_) => { /* Success */ },
            Result.Err(collections::CollectionError.OutOfBounds(index)) => {
                // Handle invalid position
            },
            Result.Err(_) => { /* Other errors */ }
        }
        
        // Find element
        let find_result = collections::list_find(&list, &"second");
        match find_result {
            Result.Ok(index) => {
                // Found at index
            },
            Result.Err(collections::CollectionError.KeyNotFound(_)) => {
                // Element not found
            },
            Result.Err(_) => { /* Other errors */ }
        }
    },
    Result.Err(_) => { /* Handle creation error */ }
}
```

## Error Handling Architecture

### Structured Error Types

```asthra
enum CollectionError {
    OutOfBounds(usize),              // Index with attempted value
    EmptyCollection(string),         // Operation context
    KeyNotFound(string),             // Key/element description
    DuplicateKey(string),            // Key that caused conflict
    InsufficientCapacity(usize),     // Required capacity
    InvalidOperation(string),        // Operation description
    MemoryAllocation(string),        // Allocation context
    IteratorInvalidated(string)      // Invalidation reason
}
```

### Error Handling Patterns

**Comprehensive Matching**:
```asthra
match operation_result {
    Result.Ok(value) => {
        // Handle success case
    },
    Result.Err(collections::CollectionError.OutOfBounds(index)) => {
        // Specific handling for bounds errors
        log("Index " + index + " is out of bounds");
    },
    Result.Err(collections::CollectionError.MemoryAllocation(msg)) => {
        // Handle memory allocation failures
        log("Memory allocation failed: " + msg);
        return Result.Err("Insufficient memory");
    },
    Result.Err(error) => {
        // Generic error handling
        log("Collection operation failed");
    }
}
```

**Graceful Degradation**:
```asthra
// Continue operation even if some elements fail
let mut successful_insertions: usize = 0;
let mut i: usize = 0;
while i < items.len {
    let insert_result = collections::vec_push(&mut vec, items[i]);
    match insert_result {
        Result.Ok(_) => {
            successful_insertions = successful_insertions + 1;
        },
        Result.Err(_) => {
            log("Failed to insert item at index " + i);
            // Continue with next item
        }
    }
    i = i + 1;
}
log("Successfully inserted " + successful_insertions + " out of " + items.len + " items");
```

## Memory Management Integration

### Automatic Cleanup

All collections provide explicit destroy functions for guaranteed cleanup:

```asthra
fn cleanup_collections() -> void {
    let mut vec = collections::vec_new::<i32>();
    let mut map = collections::hashmap_new::<string, i32>();
    let mut set = collections::hashset_new::<string>();
    let mut list = collections::list_new::<i32>();
    
    // Use collections...
    
    // Explicit cleanup (recommended for AI-generated code)
    collections::vec_destroy(&mut vec);
    collections::hashmap_destroy(&mut map);
    collections::hashset_destroy(&mut set);
    collections::list_destroy(&mut list);
}
```

### Memory Zone Integration

Collections integrate with Asthra's memory zone system:

```asthra
// All allocations use GC zone by default
let data_result = core.alloc(total_size, core.MemoryZone.GC);

// Automatic cleanup when GC runs
// Manual cleanup for immediate resource release
```

### Capacity Management

**Automatic Shrinking**:
```asthra
// Vector automatically shrinks when load factor drops
let mut vec = collections::vec_new::<i32>();
// ... add many elements ...
// ... remove many elements ...
// Vector automatically shrinks capacity

// Manual shrinking for immediate memory release
let shrink_result = collections::vec_shrink_to_fit(&mut vec);
```

**Capacity Reservation**:
```asthra
// Reserve capacity to avoid multiple reallocations
let reserve_result = collections::vec_reserve(&mut vec, 1000);
match reserve_result {
    Result.Ok(_) => {
        // Capacity reserved, subsequent pushes won't reallocate
    },
    Result.Err(_) => {
        // Handle reservation failure
    }
}
```

## Performance Optimization Guidelines

### Vector Optimization

**Choose Appropriate Growth Factor**:
```asthra
// Memory-efficient configuration
let config = collections::VecConfig {
    initial_capacity: 16,
    growth_factor: 1.5,              // Less memory waste than 2.0
    shrink_threshold: 0.3,           // Shrink when 30% full
    enable_shrinking: true
};
```

**Use Swap Remove for Unordered Data**:
```asthra
// O(1) removal for unordered vectors
let remove_result = collections::vec_swap_remove(&mut vec, index);
// vs O(n) removal for ordered vectors
let remove_result = collections::vec_remove(&mut vec, index);
```

**Reserve Capacity for Known Sizes**:
```asthra
// Avoid multiple reallocations
let vec_result = collections::vec_with_capacity(expected_size);
```

### Hash Map Optimization

**Use Power-of-Two Capacities**:
```asthra
// Optimal performance with power-of-two sizing
let map = collections::hashmap_with_capacity(64);  // Good
let map = collections::hashmap_with_capacity(60);  // Rounded up to 64
```

**Batch Operations**:
```asthra
// Insert multiple items efficiently
let items: []struct { key: string, value: i32 } = [...];
let mut i: usize = 0;
while i < items.len {
    let _ = collections::hashmap_insert(&mut map, items[i].key, items[i].value);
    i = i + 1;
}
```

### Linked List Optimization

**Use for Frequent Insertions/Deletions**:
```asthra
// Efficient for queue-like operations
collections::list_push_back(&mut queue, item);     // O(1)
let item_result = collections::list_pop_front(&mut queue);  // O(1)

// Avoid for random access
let get_result = collections::vec_get(&vec, index);  // O(1) - Better
// vs traversing linked list to index                // O(n) - Worse
```

## Collection Conversion Utilities

### Seamless Conversions

```asthra
// Vector to slice (zero-copy)
let slice = collections::vec_as_slice(&vec);

// Vector to linked list
let list_result = collections::vec_to_list(&vec);

// Linked list to vector
let vec_result = collections::list_to_vec(&list);

// Array slice to any collection
let vec_result = collections::vec_from_slice(array_data);
let set_result = collections::hashset_from_slice(array_data);
let list_result = collections::list_from_slice(array_data);
```

### Conversion Performance

| Conversion | Time Complexity | Memory Usage |
|------------|----------------|--------------|
| `vec_as_slice` | O(1) | Zero-copy |
| `vec_to_list` | O(n) | O(n) additional |
| `list_to_vec` | O(n) | O(n) additional |
| `*_from_slice` | O(n) | O(n) additional |

## Integration with Algorithms Module

### Sorting Integration

```asthra
// Sort vector using custom comparison
fn compare_i32(a: *const i32, b: *const i32) -> i32 {
    unsafe {
        if *a < *b { return -1; }
        if *a > *b { return 1; }
        return 0;
    }
}

collections::vec_sort(&mut vec, compare_i32);
```

### Search Integration

```asthra
// Binary search in sorted vector
let search_result = collections::vec_binary_search(&vec, &target, compare_i32);
match search_result {
    Result.Ok(index) => {
        // Found at index
    },
    Result.Err(collections::CollectionError.KeyNotFound(_)) => {
        // Not found
    },
    Result.Err(_) => {
        // Other error
    }
}
```

## AI Code Generation Best Practices

### Consistent Error Handling

```asthra
// Template for AI-generated collection operations
fn ai_generated_operation() -> Result<void, string> {
    let mut collection = collections::vec_new::<DataType>();
    
    let operation_result = collections::collection_operation(&mut collection, parameters);
    match operation_result {
        Result.Ok(value) => {
            // Process successful result
            return Result.Ok(());
        },
        Result.Err(collections::CollectionError.SpecificError(context)) => {
            // Handle specific error type
            return Result.Err("Specific error: " + context);
        },
        Result.Err(_) => {
            // Handle generic errors
            return Result.Err("Collection operation failed");
        }
    }
}
```

### Resource Management

```asthra
// Template for AI-generated resource management
fn ai_generated_function() -> Result<ProcessedData, string> {
    let mut vec = collections::vec_new::<RawData>();
    let mut map = collections::hashmap_new::<string, ProcessedData>();
    
    // Perform operations...
    let result = process_collections(&vec, &map);
    
    // Explicit cleanup (AI-friendly pattern)
    collections::vec_destroy(&mut vec);
    collections::hashmap_destroy(&mut map);
    
    return result;
}
```

### Performance-Aware Generation

```asthra
// AI should choose appropriate collection based on usage pattern
fn choose_collection_for_use_case() -> void {
    // For frequent random access
    let mut data_store = collections::vec_with_capacity(expected_size);
    
    // For frequent membership testing
    let mut unique_items = collections::hashset_with_capacity(expected_size);
    
    // For key-value associations
    let mut lookup_table = collections::hashmap_with_capacity(expected_size);
    
    // For frequent insertions/deletions at arbitrary positions
    let mut ordered_list = collections::list_new();
}
```

## Testing and Validation

### Comprehensive Test Coverage

The enhanced collections module includes comprehensive tests for:

- **Functionality**: All operations work correctly
- **Error Handling**: All error conditions are properly handled
- **Memory Safety**: No memory leaks or corruption
- **Performance**: Operations meet expected complexity bounds
- **Thread Safety**: Modification counters work correctly
- **Integration**: Seamless interaction with other modules

### AI-Generated Code Validation

```asthra
// Template for validating AI-generated collection usage
fn validate_ai_generated_code() -> bool {
    // Check proper error handling
    let operation_result = collections::vec_push(&mut vec, item);
    match operation_result {
        Result.Ok(_) => { /* Success path */ },
        Result.Err(_) => { /* Error path - must be present */ }
    }
    
    // Check proper resource cleanup
    collections::vec_destroy(&mut vec);  // Must be present
    
    // Check appropriate collection choice for use case
    // Vector for indexed access, HashMap for lookups, etc.
    
    return true;
}
```

## Future Enhancements

### Planned Features

1. **Iterator Support**: Safe iteration with invalidation detection
2. **Concurrent Collections**: Thread-safe variants for parallel processing
3. **Specialized Collections**: Priority queues, deques, trees
4. **Advanced Set Operations**: Full implementation of union, intersection, difference
5. **Memory Pool Integration**: Custom allocators for performance-critical code
6. **Serialization Support**: Integration with JSON and binary serialization
7. **Generic Algorithms**: More comprehensive algorithm integration

### Performance Improvements

1. **SIMD Optimization**: Vectorized operations for bulk data processing
2. **Cache Optimization**: Memory layout improvements for better cache locality
3. **Lazy Evaluation**: Deferred operations for better performance
4. **Adaptive Algorithms**: Dynamic algorithm selection based on data characteristics

## Conclusion

The Enhanced Collections module provides a comprehensive, AI-friendly foundation for data structure operations in Asthra. With robust error handling, performance optimization, and seamless integration with the language's type system and memory management, it enables reliable and efficient code generation while maintaining the safety and predictability that AI systems require.

The design prioritizes:
- **Predictability** for reliable AI code generation
- **Performance** for production-ready applications  
- **Safety** through comprehensive error handling
- **Flexibility** through configurable behavior
- **Integration** with the broader Asthra ecosystem

This foundation supports both simple AI-generated scripts and complex, performance-critical applications while maintaining consistent patterns that AI systems can reliably generate and maintain. 
