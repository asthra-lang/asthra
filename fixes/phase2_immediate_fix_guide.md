# Phase 2 Immediate Fix Guide - test_elf_writer_main

## Quick Analysis

The `test_elf_writer_main` test is failing because it uses the minimal `$(CODEGEN_ELF_DEPS)` which is missing critical dependencies that we've already resolved in the comprehensive approach.

## Step-by-Step Implementation

### Step 1: Examine Current ELF Dependencies

Current `CODEGEN_ELF_DEPS` in configuration.mk (lines 134-139):
```makefile
CODEGEN_ELF_DEPS := \
    $(CODEGEN_CORE_DEPS) \
    build/src/codegen/elf_writer_core.o \
    build/src/codegen/elf_writer_sections.o \
    build/src/codegen/elf_debug_validation.o
```

Current `CODEGEN_CORE_DEPS` in configuration.mk (lines 126-132):
```makefile
CODEGEN_CORE_DEPS := \
    build/src/codegen/code_generator.o \
    build/src/codegen/code_generator_statements.o \
    build/src/codegen/code_generator_expressions.o \
    build/src/codegen/code_generator_utils.o \
    build/src/codegen/code_generator_abi.o \
    build/src/codegen/instruction_buffer.o \
    build/src/codegen/register_allocator.o
```

### Step 2: Identify Missing Dependencies

From the error analysis, missing symbols include:
- `_add_function_parameter` → likely in symbol_table.o or code_generator_abi.o
- `_ast_node_list_get`, `_ast_node_list_size` → from ast_node_list.o
- `_create_*_instruction` functions → from instruction_factory.o
- `_elf_*` functions → additional ELF modules
- Test framework functions → test support files

### Step 3: Update CODEGEN_CORE_DEPS

Add missing core dependencies to configuration.mk:

```makefile
# Core codegen dependencies (UPDATED)
CODEGEN_CORE_DEPS := \
    build/src/parser/ast_node_list.o \
    build/src/codegen/code_generator.o \
    build/src/codegen/code_generator_statements.o \
    build/src/codegen/code_generator_expressions.o \
    build/src/codegen/code_generator_utils.o \
    build/src/codegen/code_generator_abi.o \
    build/src/codegen/instruction_factory.o \
    build/src/codegen/instruction_buffer.o \
    build/src/codegen/register_allocator.o \
    build/src/codegen/symbol_table.o \
    build/src/codegen/label_manager.o
```

### Step 4: Update CODEGEN_ELF_DEPS

Add missing ELF-specific dependencies:

```makefile
# ELF writer dependencies (UPDATED)
CODEGEN_ELF_DEPS := \
    $(CODEGEN_CORE_DEPS) \
    build/src/codegen/elf_writer_core.o \
    build/src/codegen/elf_writer_sections.o \
    build/src/codegen/elf_writer_utils.o \
    build/src/codegen/elf_symbol_manager.o \
    build/src/codegen/elf_relocation_manager.o \
    build/src/codegen/elf_debug_validation.o \
    build/src/codegen/elf_runtime_init.o \
    build/src/codegen/elf_utils.o \
    build/src/codegen/elf_section_population.o
```

### Step 5: Add Test Framework Support

Check if `elf_writer_test_common.c` exists and add test framework:

```makefile
# ELF test framework dependencies (NEW)
CODEGEN_ELF_TEST_DEPS := \
    $(CODEGEN_ELF_DEPS) \
    tests/framework/test_assertions_common.c \
    tests/framework/test_assertions_equality.c \
    tests/framework/test_context.c \
    tests/framework/test_statistics.c
```

### Step 6: Update Build Target

Modify the build target in build-targets.mk to use the enhanced dependencies:

```makefile
# Fix test_elf_writer_main with proper dependencies (UPDATED)
build/tests/codegen/test_elf_writer_main: tests/codegen/test_elf_writer_main.c $(CODEGEN_ELF_TEST_DEPS) | build/tests/codegen
    @echo "Building test_elf_writer_main..."
    $(CC) $(CODEGEN_SAFE_CFLAGS) -o $@ $< $(CODEGEN_ELF_TEST_DEPS) $(LDFLAGS)
```

### Step 7: Test the Fix

```bash
# Clean build to ensure fresh compilation
make clean-codegen-tests

# Try building the specific test
make build/tests/codegen/test_elf_writer_main

# If successful, run the test
./build/tests/codegen/test_elf_writer_main
```

### Step 8: Verify No Regressions

```bash
# Ensure working tests still work
./build/tests/codegen/test_code_generator_comprehensive
./build/tests/codegen/test_register_allocation_minimal  
./build/tests/codegen/test_optimization_pipeline_creation
```

## Expected Results

After implementing these changes:

1. **test_elf_writer_main should compile successfully** with all missing symbols resolved
2. **No regressions** in the 3 currently working tests
3. **Progress toward 4/5 tests working** (80% success rate)

## Fallback Strategy

If the enhanced dependencies cause issues:

1. **Revert changes** to configuration.mk
2. **Try incremental approach** - add dependencies one at a time
3. **Use comprehensive approach** - copy the working compilation-codegen.mk template

## Next Steps After Success

1. **Implement Phase 3** for test_immutable_by_default_phase3
2. **Consider unification** of the two working approaches
3. **Document the final dependency structure**

## Quick Command Reference

```bash
# Edit configuration file
vim make/tests/codegen/configuration.mk

# Test compilation only
make build/tests/codegen/test_elf_writer_main

# Run if compilation succeeds  
./build/tests/codegen/test_elf_writer_main

# Check all tests
make test-codegen
```

This guide provides the specific steps needed to immediately fix the test_elf_writer_main test by applying the lessons learned from the comprehensive dependency resolution. 
