# Arrays and Slices in Asthra

Asthra provides powerful array and slice functionality with a syntax inspired by Rust and Go, designed for safety and performance.

## Fixed-Size Arrays

Fixed-size arrays in Asthra have their size known at compile time, providing memory efficiency and compile-time bounds checking opportunities.

### Syntax

```asthra
// Array type: [size]Type
let arr: [10]i32;              // Array of 10 i32 values
let buffer: [256]u8;           // Array of 256 bytes
let matrix: [3][4]f32;         // 3x4 matrix of f32 values
```

### Array Initialization

#### Repeated Element Syntax

The most common way to initialize arrays is using the repeated element syntax from Rust:

```asthra
// Syntax: [value; count]
let zeros: [10]i32 = [0; 10];        // Array of 10 zeros
let ones: [5]f32 = [1.0; 5];         // Array of 5 ones
let flags: [8]bool = [false; 8];     // Array of 8 false values
```

#### Explicit Element List

For small arrays, you can list elements explicitly:

```asthra
let primes: [5]i32 = [2, 3, 5, 7, 11];
let colors: [3]string = ["red", "green", "blue"];
```

### Compile-Time Constants

Array sizes must be compile-time constants:

```asthra
const BUFFER_SIZE: i32 = 1024;
const DOUBLE_SIZE: i32 = BUFFER_SIZE * 2;

let buffer: [BUFFER_SIZE]u8 = [0; BUFFER_SIZE];
let large: [DOUBLE_SIZE]u8 = [0; DOUBLE_SIZE];

// This is also valid:
let arr: [10 + 5]i32 = [0; 15];
```

## Slices

Slices are dynamically-sized views into arrays or other slices. They consist of a pointer to the data and a length.

### Slice Types

```asthra
// Slice type: []Type
let slice: []i32;              // Slice of i32 values
let bytes: []u8;               // Slice of bytes
```

### Creating Slices

Slices are typically created from arrays using Go-style slicing syntax:

```asthra
let arr: [10]i32 = [0; 10];

// Full slice - all elements
let full: []i32 = arr[:];

// Slice from start to index (exclusive)
let first_half: []i32 = arr[:5];    // Elements 0-4

// Slice from index to end
let second_half: []i32 = arr[5:];   // Elements 5-9

// Slice from start to end index
let middle: []i32 = arr[2:8];       // Elements 2-7
```

### Slice Operations

```asthra
// Slicing a slice
let data: [10]i32 = [0; 10];
let slice1: []i32 = data[2:8];      // 6 elements
let slice2: []i32 = slice1[1:4];    // 3 elements from slice1

// Accessing elements
let first: i32 = slice[0];
let last: i32 = slice[slice.length - 1];
```

## Multi-Dimensional Arrays

Asthra supports multi-dimensional arrays with natural syntax:

```asthra
// 2D array (matrix)
let matrix: [3][4]i32 = [[0; 4]; 3];

// 3D array
let cube: [2][3][4]i32 = [[[0; 4]; 3]; 2];

// Accessing elements
let element: i32 = matrix[1][2];
let row: [4]i32 = matrix[0];
let row_slice: []i32 = matrix[1][:];
```

## Function Parameters

Arrays and slices can be passed to functions:

```asthra
// Function accepting a slice (flexible size)
pub fn sum(numbers: []i32) -> i32 {
    let total: i32 = 0;
    for n in numbers {
        total = total + n;
    }
    return total;
}

// Function accepting a fixed-size array
pub fn process_buffer(buffer: [256]u8) -> void {
    // Process exactly 256 bytes
}

// Usage
let data: [5]i32 = [1, 2, 3, 4, 5];
let result: i32 = sum(data[:]);     // Pass as slice
```

## Type Compatibility

- Fixed-size arrays of the same element type and size are compatible
- Arrays can be converted to slices using the slice syntax `[:]`
- Slices cannot be converted back to arrays (size information is lost)

```asthra
let arr1: [5]i32 = [0; 5];
let arr2: [5]i32 = arr1;           // OK: same type
let slice: []i32 = arr1[:];        // OK: array to slice

let arr3: [10]i32 = [0; 10];
// let arr4: [5]i32 = arr3;        // Error: different sizes
```

## Best Practices

1. **Use fixed-size arrays when the size is known at compile time**
   - Better performance (no indirection)
   - Stack allocation (usually)
   - Compile-time bounds checking opportunities

2. **Use slices for flexible-size data and function parameters**
   - More flexible APIs
   - Can work with arrays of any size
   - Supports sub-slicing

3. **Initialize arrays with repeated element syntax**
   - Clear and concise
   - Guaranteed initialization
   - Optimized by the compiler

4. **Be mindful of large arrays**
   - Large arrays may cause stack overflow
   - Consider heap allocation for very large data

## Examples

### Buffer Management

```asthra
struct Buffer {
    data: [4096]u8,
    used: usize
}

impl Buffer {
    pub fn new(none) -> Buffer {
        return Buffer {
            data: [0; 4096],
            used: 0
        };
    }
    
    pub fn get_contents(self) -> []u8 {
        return self.data[:self.used];
    }
}
```

### Matrix Operations

```asthra
pub fn transpose(matrix: [3][3]i32) -> [3][3]i32 {
    let result: [3][3]i32 = [[0; 3]; 3];
    
    for i in 0..3 {
        for j in 0..3 {
            result[j][i] = matrix[i][j];
        }
    }
    
    return result;
}
```

### Safe Subarray Access

```asthra
pub fn get_subarray(data: []i32, start: usize, end: usize) -> Result<[]i32, string> {
    if start > end || end > data.length {
        return Err("Invalid range");
    }
    
    return Ok(data[start:end]);
}
```

## Implementation Notes

- Arrays are stored contiguously in memory
- Slices are fat pointers (pointer + length)
- Array indexing is bounds-checked in debug mode
- Slice operations create new slice descriptors, not copy data
- The compiler optimizes repeated element initialization

## Future Enhancements

- Runtime bounds checking options
- Array/slice iterators
- Built-in array algorithms (sort, search, etc.)
- SIMD operations on arrays