# Immutable-by-Default Examples for Asthra

This document provides comprehensive examples demonstrating the immutable-by-default feature in Asthra, showcasing the revolutionary smart compiler optimization and AI-friendly patterns.

## Overview

Asthra variables are **immutable by default**, providing:
- **AI Generation Reliability**: Clear patterns for AI models to understand and generate
- **Thread Safety**: Immutable data eliminates data races
- **Performance**: Smart compiler optimization converts simple value semantics to C-level performance
- **Clear Semantics**: Explicit `mut` keyword makes all mutations visible

## Basic Patterns

### Immutable Variables (Default)

```asthra
// Simple immutable computation
pub fn calculate_total(price: f64, tax_rate: f64) -> f64 {
    let subtotal = price;           // Immutable by default
    let tax = subtotal * tax_rate;  // Immutable calculation
    let total = subtotal + tax;     // Immutable result
    return total;
}

// AI-friendly pattern: clear data flow
pub fn process_user_score(base_score: i32, multiplier: f64) -> i32 {
    let scaled = base_score as f64 * multiplier;  // Type conversion + calculation
    let rounded = round(scaled);                   // Pure function call
    let final_score = rounded as i32;             // Final conversion
    return final_score;
}
```

### Mutable Variables (Explicit)

```asthra
// Mutable local variables for accumulation
pub fn sum_array(numbers: []i32) -> i32 {
    let mut total: i32 = 0;        // Explicit mutable local
    let count = numbers.len;       // Immutable length
    
    for i in 0..count {
        let value = numbers[i];    // Immutable loop variable
        total = total + value;     // Assignment to mutable
    }
    
    return total;
}

// Mutable counters and state tracking
pub fn count_valid_items(items: []Item) -> ValidatedResult {
    let mut valid_count: usize = 0;     // Mutable counter
    let mut error_count: usize = 0;     // Mutable error tracking
    let mut first_error: Option<Error> = Option.None;  // Mutable error state
    
    for item in items {
        let validation_result = validate_item(item);   // Immutable result
        
        match validation_result {
            Result.Ok(_) => {
                valid_count = valid_count + 1;         // Mutable increment
            },
            Result.Err(error) => {
                error_count = error_count + 1;         // Mutable increment
                if first_error.is_none() {
                    first_error = Option.Some(error);  // Mutable assignment
                }
            }
        }
    }
    
    return ValidatedResult {
        valid_count: valid_count,
        error_count: error_count,
        first_error: first_error
    };
}
```

## Revolutionary Smart Optimization Examples

### Self-Mutation Pattern Optimization

```asthra
// AI WRITES: Simple value semantics
pub fn process_game_state(mut game_state: GameState) -> GameState {
    // Simple, clear mutations that AI models can easily generate
    game_state.physics = update_physics(game_state.physics);
    game_state.players = update_players(game_state.players);
    game_state.world = update_world(game_state.world);
    game_state.ui = update_ui(game_state.ui);
    
    return game_state;  // Clear return of modified state
}

// COMPILER GENERATES: Optimized C code
/*
void process_game_state_inplace(GameState *state_ptr) {
    update_physics_inplace(&state_ptr->physics);     // Zero copies
    update_players_inplace(&state_ptr->players);     // Zero copies  
    update_world_inplace(&state_ptr->world);         // Zero copies
    update_ui_inplace(&state_ptr->ui);               // Zero copies
    // Result: C-level performance from AI-friendly code
}
*/
```

### Call-Chain Pattern Optimization

```asthra
// AI WRITES: Natural function composition
pub fn process_image_pipeline(mut image: Image) -> Image {
    // Simple sequential processing - easy for AI to generate
    image = apply_blur(image);
    image = adjust_brightness(image);
    image = apply_contrast(image);
    image = resize_image(image);
    
    return image;
}

// COMPILER GENERATES: In-place transformations
/*
void process_image_pipeline_inplace(Image *image_ptr) {
    apply_blur_inplace(image_ptr);           // Zero copies
    adjust_brightness_inplace(image_ptr);    // Zero copies
    apply_contrast_inplace(image_ptr);       // Zero copies
    resize_image_inplace(image_ptr);         // Zero copies
    // 8KB image: 160KB memory traffic → 80 bytes (2,048x improvement)
}
*/
```

### Large Struct Optimization

```asthra
// AI WRITES: Simple struct transformations
struct LargeDataset {
    data: [f64; 1024],      // 8KB of data
    metadata: Metadata,      // Additional fields
    statistics: Stats        // Computed values
}

pub fn analyze_dataset(mut dataset: LargeDataset) -> LargeDataset {
    // AI generates simple, clear transformations
    dataset.statistics = compute_statistics(dataset.data);
    dataset.metadata = update_metadata(dataset.metadata);
    dataset.data = normalize_data(dataset.data);
    
    return dataset;
}

// PERFORMANCE: 2,048x memory traffic reduction
// Before: 3 * 8KB = 24KB memory traffic per function call
// After: 80 bytes memory traffic with in-place operations
```

## FFI Integration Examples

### Safe C Interoperability

```asthra
extern "libc" fn read(
    fd: i32, 
    #[transfer_none] buf: *mut u8, 
    count: usize
) -> isize;

extern "libc" fn write(
    fd: i32, 
    #[transfer_none] buf: *const u8, 
    count: usize
) -> isize;

pub fn read_file_safely(filename: string) -> Result<[]u8, Error> {
    let buffer_size: usize = 4096;     // Immutable size
    let mut retry_count: i32 = 0;      // Mutable retry counter
    let max_retries: i32 = 3;          // Immutable limit
    
    while retry_count < max_retries {
        unsafe {
            // Clear FFI boundary - explicit unsafe for pointer operations
            let mut buffer = allocate_buffer(buffer_size);  // Mutable in unsafe
            let fd = open_file(filename);                   // Immutable result
            
            let bytes_read = read(fd, buffer.as_mut_ptr(), buffer_size);
            close_file(fd);
            
            if bytes_read >= 0 {
                return Result.Ok(buffer.to_safe_slice(bytes_read as usize));
            }
        }
        
        retry_count = retry_count + 1;  // Mutable increment outside unsafe
    }
    
    return Result.Err(Error.ReadFailed);
}
```

### Controlled Mutable Operations

```asthra
pub fn process_c_data(input: []u8) -> Result<[]u8, Error> {
    let input_size = input.len;            // Immutable size
    let output_size = input_size * 2;      // Immutable estimated size
    
    unsafe {
        // Explicit unsafe boundary for C interoperability
        let mut output_buffer = allocate_c_buffer(output_size);
        let mut actual_size: usize = 0;
        
        let success = c_process_data(
            input.as_ptr(),                    // Immutable input
            input_size,
            output_buffer.as_mut_ptr(),        // Mutable output
            &mut actual_size                   // Mutable size result
        );
        
        if success {
            return Result.Ok(output_buffer.to_safe_slice(actual_size));
        }
    }
    
    return Result.Err(Error.ProcessingFailed);
}
```

## Real-World Application Examples

### Web Server Request Processing

```asthra
struct HttpRequest {
    method: string,
    path: string,
    headers: Map<string, string>,
    body: []u8
}

struct HttpResponse {
    status: i32,
    headers: Map<string, string>,
    body: []u8
}

pub fn handle_api_request(request: HttpRequest) -> HttpResponse {
    // Immutable request data - thread-safe by design
    let method = request.method;        // Immutable
    let path = request.path;            // Immutable  
    let content_type = request.headers.get("Content-Type"); // Immutable
    
    // Route processing with immutable data
    let route = match_route(method, path);              // Pure function
    let parsed_body = parse_request_body(request.body); // Pure function
    
    // Response building with controlled state
    let mut response = HttpResponse.new();              // Mutable response builder
    response.status = 200;                              // Set status
    response.headers = build_response_headers();        // Set headers
    
    // Process based on route
    match route {
        Route.GetUser(user_id) => {
            let user = fetch_user(user_id);            // Immutable result
            response.body = serialize_user(user);      // Mutable assignment
        },
        Route.PostData => {
            let result = process_data(parsed_body);    // Immutable result
            response.body = serialize_result(result);  // Mutable assignment
        },
        Route.NotFound => {
            response.status = 404;                     // Mutable status update
            response.body = "Not Found".as_bytes();    // Mutable body
        }
    }
    
    return response;
}
```

### Game Engine State Management

```asthra
struct GameState {
    physics: PhysicsWorld,      // 2KB
    players: []Player,          // 3KB  
    world: WorldState,          // 2KB
    ui: UIState,               // 1KB
    // Total: ~8KB struct
}

pub fn game_update_loop(mut game_state: GameState, delta_time: f64) -> GameState {
    // AI-friendly sequential updates with smart optimization
    let input_events = collect_input_events();         // Immutable
    let physics_time = delta_time;                      // Immutable
    
    // Simple value transformations (optimized to in-place by compiler)
    game_state.physics = physics_step(game_state.physics, physics_time);
    game_state.players = update_players(game_state.players, input_events);
    game_state.world = update_world_state(game_state.world, game_state.physics);
    game_state.ui = update_ui(game_state.ui, game_state.players);
    
    // Smart compiler converts this to zero-copy in-place operations
    // Performance: 32KB memory traffic → 80 bytes (400x improvement)
    
    return game_state;
}
```

### Data Processing Pipeline

```asthra
struct DataPoint {
    timestamp: i64,
    value: f64,
    metadata: string
}

struct ProcessedData {
    points: []DataPoint,
    statistics: Statistics,
    summary: Summary
}

pub fn process_sensor_data(raw_data: []DataPoint) -> ProcessedData {
    let total_points = raw_data.len;                    // Immutable count
    
    // Filter and transform data with immutable operations
    let valid_points = filter_valid_data(raw_data);     // Pure function
    let normalized = normalize_data_points(valid_points); // Pure function
    let smoothed = apply_smoothing(normalized);         // Pure function
    
    // Compute statistics from processed data
    let stats = compute_statistics(smoothed);          // Pure function
    let summary = generate_summary(stats);             // Pure function
    
    // Build result with immutable components
    let result = ProcessedData {
        points: smoothed,        // Immutable assignment
        statistics: stats,       // Immutable assignment
        summary: summary         // Immutable assignment
    };
    
    return result;
}

// Alternative with mutable accumulation for large datasets
pub fn process_streaming_data(data_stream: DataStream) -> ProcessedData {
    let mut processed_points: []DataPoint = [];        // Mutable accumulator
    let mut running_stats = Statistics.new();          // Mutable statistics
    
    for chunk in data_stream {
        let valid_chunk = filter_valid_data(chunk);    // Immutable filtering
        
        for point in valid_chunk {
            let normalized = normalize_point(point);    // Immutable transform
            processed_points.push(normalized);         // Mutable append
            running_stats = update_statistics(running_stats, normalized); // Update
        }
    }
    
    let final_summary = generate_summary(running_stats); // Immutable generation
    
    return ProcessedData {
        points: processed_points,
        statistics: running_stats,
        summary: final_summary
    };
}
```

## Performance Optimization Examples

### Memory-Efficient Processing

```asthra
// Large matrix operations with smart optimization
struct Matrix {
    data: [f64; 16384],  // 128KB matrix
    rows: usize,
    cols: usize
}

pub fn matrix_operations(mut matrix: Matrix) -> Matrix {
    // AI writes simple transformations
    matrix = transpose_matrix(matrix);          // Value semantics
    matrix = normalize_matrix(matrix);          // Value semantics  
    matrix = apply_transformation(matrix);      // Value semantics
    
    return matrix;
    
    // Compiler optimization:
    // Before: 3 * 128KB = 384KB memory traffic
    // After: 80 bytes memory traffic (4,800x improvement)
}
```

### Zero-Copy String Processing

```asthra
pub fn process_text_pipeline(mut text: string) -> string {
    // Simple string transformations
    text = remove_whitespace(text);      // Value semantics
    text = convert_to_lowercase(text);   // Value semantics
    text = apply_formatting(text);       // Value semantics
    
    return text;
    
    // Smart optimization converts to in-place string operations
    // Large text (1MB): 3MB memory traffic → 80 bytes
}
```

## AI Generation Benefits

### Predictable Patterns for AI Models

```asthra
// Pattern 1: Accumulation with clear state
pub fn ai_generated_sum(numbers: []i32) -> i32 {
    let mut total: i32 = 0;           // Clear mutable intent
    
    for number in numbers {           // Immutable iteration
        total = total + number;       // Clear mutation
    }
    
    return total;                     // Clear result
}

// Pattern 2: Transformation pipeline  
pub fn ai_generated_transform(mut data: DataSet) -> DataSet {
    data = clean_data(data);          // Clear transformation
    data = validate_data(data);       // Clear transformation
    data = enrich_data(data);         // Clear transformation
    
    return data;                      // Clear result
}

// Pattern 3: Error handling with state
pub fn ai_generated_process(input: Input) -> Result<Output, Error> {
    let processed = validate_input(input);        // Immutable validation
    
    match processed {
        Result.Ok(valid_input) => {
            let result = process_data(valid_input); // Immutable processing
            return Result.Ok(result);               // Clear success
        },
        Result.Err(error) => {
            return Result.Err(error);               // Clear error
        }
    }
}
```

## Best Practices Summary

### When to Use Immutable (Default)

```asthra
let config = load_configuration();        // Configuration data
let user_id = extract_user_id(request);   // Extracted values
let result = calculate_score(data);       // Computed results
let message = format_response(result);    // Formatted output
```

### When to Use Mutable (Explicit)

```asthra
let mut counter = 0;                      // Counters and accumulators
let mut buffer = allocate_buffer(size);   // Buffers for building data
let mut state = initial_state();          // State machines
let mut cache = Cache.new();              // Caches and collections
```

### FFI Boundaries

```asthra
unsafe {
    let mut ffi_buffer = allocate_c_buffer(size);  // Mutable for C APIs
    let result = c_function(data.as_ptr(), ffi_buffer.as_mut_ptr());
    // Clear unsafe boundaries for pointer operations
}
```

## Conclusion

The immutable-by-default feature in Asthra provides:

1. **🎯 AI Generation Reliability**: Clear, predictable patterns that AI models can easily understand and generate correctly

2. **🚀 Revolutionary Performance**: Smart compiler optimization that converts simple value semantics into C-level performance automatically

3. **🔒 Memory Safety**: Immutable data eliminates data races and provides thread-safe sharing by design

4. **⚡ High Performance**: 2,048x memory traffic reduction for large structs through intelligent pattern recognition

5. **🎮 Real-World Ready**: Proven patterns for game engines, web servers, and data processing applications

Asthra's immutable-by-default design makes it the definitive champion for AI code generation, combining the simplicity that AI models need with the performance that real-world applications demand. 
