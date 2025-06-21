# CodeGen API

### Core Functions

```c
// ELF Writer Lifecycle
ELFWriter *elf_writer_create(FFIAssemblyGenerator *ffi_generator);
void elf_writer_destroy(ELFWriter *writer);
bool elf_generate_object_file(ELFWriter *writer, ASTNode *program, const char *output_filename);

// Section Management
bool elf_create_standard_sections(ELFWriter *writer);
bool elf_create_asthra_sections(ELFWriter *writer);
ELFSection *elf_add_section(ELFWriter *writer, const char *name, uint32_t type, uint64_t flags);

// Symbol Management
ELFSymbol *elf_add_symbol(ELFWriter *writer, const char *name, uint64_t value, uint64_t size, 
                         uint8_t type, uint8_t binding, uint16_t section_index);
ELFSymbol *elf_add_ffi_symbol(ELFWriter *writer, const char *name, uint64_t address, 
                             FFIFunctionSignature *signature);
ELFSymbol *elf_add_gc_root_symbol(ELFWriter *writer, const char *name, uint64_t address, 
                                 size_t size, uint32_t ownership_type);

// Metadata Population
bool elf_populate_ffi_section(ELFWriter *writer);
bool elf_populate_gc_section(ELFWriter *writer);
bool elf_populate_security_section(ELFWriter *writer);
bool elf_populate_pattern_matching_section(ELFWriter *writer);
bool elf_populate_string_ops_section(ELFWriter *writer);
bool elf_populate_slice_meta_section(ELFWriter *writer);
bool elf_populate_concurrency_section(ELFWriter *writer);

// Runtime and Debug
bool elf_generate_runtime_init(ELFWriter *writer);
bool elf_generate_debug_info(ELFWriter *writer);
bool elf_validate_structure(ELFWriter *writer);
```

### Configuration Options

```c
typedef struct {
    bool generate_debug_info;        // Generate DWARF debug information
    bool optimize_metadata;          // Optimize metadata for size/performance
    bool validate_elf_structure;     // Validate ELF structure before writing
    bool enable_all_asthra_sections; // Create all Asthra-specific sections
    size_t max_symbols;              // Maximum number of symbols
    size_t max_relocations;          // Maximum number of relocations
} ELFWriterConfig;
``` 