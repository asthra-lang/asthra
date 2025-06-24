; ModuleID = 'test.asthra'
source_filename = "test.asthra"
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

define i32 @main() {
entry:
  ret i32 42
}
