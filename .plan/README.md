# Development Documentation

This directory contains documentation focused on the development of the Asthra compiler and language implementation. These documents are primarily for contributors who are working on the compiler itself, build systems, and internal development processes.

## Directory Structure

### Core Development Documents

- **CONTRIBUTING.md** - Main contributor guide for the Asthra project
- **COMPILER_ARCHITECTURE_DOCUMENTATION_PLAN.md** - Plan for documenting compiler architecture
- **build_tools_overview.md** - Overview of build tools and infrastructure

### Build System & Infrastructure

- **BUILD_SYSTEM_CONSOLIDATION_PLAN.md** - Plan for consolidating the build system
- **BUILD_SYSTEM_MIGRATION_GUIDE.md** - Guide for migrating to the unified build system
- **RUNTIME_MODULARIZATION_PLAN.md** - Plan for modularizing the runtime system
- **RUNTIME_HEADER_MODULARIZATION_PLAN.md** - Plan for modularizing runtime headers

### Grammar & Implementation Alignment

- **GRAMMAR_CONFORMANCE_PROJECT_COMPLETION.md** - Grammar conformance project completion report
- **GRAMMAR_CONFORMANCE_ONBOARDING.md** - Onboarding guide for grammar conformance work
- **GRAMMAR_MAINTENANCE_GUIDE.md** - Guide for maintaining grammar consistency
- **GRAMMAR_FIX_BACKUP_MANIFEST.md** - Manifest of grammar fix backups
- **GRAMMAR_CONFORMANCE_PROGRESS_REPORT.md** - Progress reports on grammar conformance
- **GRAMMAR_CONFORMANCE_AUDIT_REPORT.md** - Audit reports and data on grammar conformance
- **GRAMMAR_IMPLEMENTATION_ALIGNMENT_PLAN.md** - Plan for aligning grammar and implementation
- **GRAMMAR_IMPLEMENTATION_ALIGNMENT_SUMMARY.md** - Summary of alignment work

### Development Process & Planning

- **POST_TEST_CATEGORY_STRATEGIC_PLAN.md** - Strategic plan for post-test category development
- **TEST_CATEGORY_FIX_PLAN.md** - Plan for fixing test categories
- **CODE_DUPLICATION_ELIMINATION_PLAN.md** - Plan for eliminating code duplication

### Compiler Implementation

- **SEMANTIC_TYPE_RESOLUTION_IMPLEMENTATION.md** - Implementation details for semantic type resolution
- **CLANG_DEFAULT_IMPLEMENTATION_SUMMARY.md** - Summary of clang default implementation
- **CLANG_DEFAULT_COMPILER_PLAN.md** - Plan for clang default compiler support
- **log-parser-remove-plan.md** - Plan for removing redundant log parser infrastructure

### Subdirectories

- **internal/** - Internal development documentation including stdlib planning, build system details, and FFI documentation
- **reports/** - Technical analysis reports covering various aspects of the compiler and language design

## For Contributors

If you're contributing to the Asthra compiler:

1. Start with `CONTRIBUTING.md` for the general contribution guidelines
2. Review the `internal/` directory for deep technical documentation
3. Check the `reports/` directory for analysis reports on specific features
4. Consult the grammar and build system documentation for implementation alignment

## Note on Plans vs Development Documentation

This directory contains **development-focused documents** but excludes **implementation plans**. Implementation plans remain in the main `docs/` directory for easier access during active development work. 
