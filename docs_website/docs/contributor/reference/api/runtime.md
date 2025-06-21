# Runtime API

**Core Runtime Interface** (`src/runtime/core/asthra_runtime_core.h`):
```c
// Runtime initialization
bool asthra_runtime_init(RuntimeConfig* config);
void asthra_runtime_shutdown(void);

// Memory management
void* asthra_alloc(size_t size, MemoryZone zone);
void asthra_free(void* ptr, MemoryZone zone);
void asthra_gc_collect(void);

// Concurrency
AsthraTaskHandle asthra_spawn(TaskFunction function, void* args);
void* asthra_await(AsthraTaskHandle handle);
AsthraConcurrencyChannel* asthra_channel_create(size_t element_size, 
                                               size_t capacity, 
                                               const char* name);

// FFI
AsthraResult asthra_ffi_call(FFICall* call, void** args);
void* asthra_marshal_to_c(void* asthra_value, TypeInfo* type);

// Safety
bool asthra_validate_pointer(void* ptr);
bool asthra_check_bounds(void* array, size_t index);
void asthra_panic(const char* message, ...);
``` 