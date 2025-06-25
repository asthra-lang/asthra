; ModuleID = 'asthra_module'
source_filename = "asthra_module"
target triple = "arm64-apple-darwin24.5.0"

declare ptr @asthra_alloc(i64, i32)

declare void @asthra_free(ptr, i32)

declare void @asthra_gc_register_root(ptr)

declare void @asthra_gc_unregister_root(ptr)

declare void @asthra_panic(ptr)

declare void @asthra_log(i32, ptr, ...)

declare ptr @asthra_string_create(ptr, i64)

declare void @asthra_string_free(ptr)

declare i32 @printf(ptr, ...)
