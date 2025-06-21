# Asthra CI/CD Workflows

This directory contains the GitHub Actions workflows for building, testing, and deploying the Asthra programming language compiler.

## Workflow Structure

The CI/CD pipeline is divided into multiple modular workflows:

- **ci.yml**: Main orchestrator workflow that calls all other workflows
- **build-and-test.yml**: Builds and tests Asthra across multiple platforms using CMake
- **comprehensive-tests.yml**: Runs comprehensive test suite with detailed coverage
- **cross-platform-check.yml**: Analyzes cross-platform compatibility of builds
- **security.yml**: Performs comprehensive security scanning using CodeQL and other tools
- **docs.yml**: Generates and validates documentation
- **performance.yml**: Runs performance benchmarks and regression detection
- **release-prep.yml**: Prepares release packages when code is pushed to main branch
- **release.yml**: Handles full release creation and distribution
- **maintenance.yml**: Automated code maintenance and housekeeping
- **notify-results.yml**: Collects and reports results from all workflows
- **claude.yml**: AI-powered code assistance
- **claude-code-review.yml**: Automated code review using Claude

## Key Changes (Updated for CMake)

### ✅ **Simplified for "Start Small" Approach**
- Removed Windows builds to focus on Unix platforms (Linux/macOS) initially
- Simplified test suites to essential functionality
- Removed obsolete and redundant workflows

### ✅ **CMake Migration**
- All workflows now use CMake instead of make
- Updated build commands: `cmake --build . --config Release`
- Updated test commands: `ctest --output-on-failure`
- Added required dependencies: `cmake`, `libjson-c-dev`

### ✅ **Removed Redundant Workflows**
- `security-scan.yml` (basic) - replaced by comprehensive `security.yml`
- `docs-and-format.yml` (basic) - replaced by comprehensive `docs.yml`
- `cmake-ci.yml` - obsolete and using outdated actions
- `sync-docs.yml` - referenced non-existent external repositories
- `trigger-website-sync.yml` - used non-existent tokens
- `grammar-conformance.yml` - referenced non-existent scripts

### ✅ **Updated GitHub Actions**
- All workflows now use `actions/checkout@v4`
- Updated to recent versions of other actions
- Improved error handling and artifact management

## Workflow Execution Flow

1. The `ci.yml` workflow is triggered by:
   - Pushes to `main` or `develop` branches
   - Pull requests targeting `main` or `develop` branches
   - Weekly scheduled runs (Sundays at 2 AM UTC)
   - Manual workflow dispatch with optional parameters

2. The main workflow then calls individual workflow files:
   - `build-and-test.yml` runs first with CMake builds
   - `comprehensive-tests.yml` runs only on main branch pushes or scheduled runs
   - `cross-platform-check.yml` runs after successful builds
   - `notify-results.yml` runs at the end to summarize results

## Build Matrix

The build-and-test workflow uses a matrix strategy to test across:

- **Linux**: Ubuntu 20.04 and 22.04 with GCC and Clang
- **macOS**: Intel (13) and ARM (14) with Clang

## CMake Build Process

All builds now follow this pattern:

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
ctest --output-on-failure
```

## Workflow Inputs

The main workflow accepts these optional inputs when manually triggered:

- `run_performance_tests`: Enables or disables performance benchmarks
- `run_sanitizer_tests`: Enables or disables sanitizer tests

## Standalone Workflows

Some workflows can run independently:

- **security.yml**: Comprehensive security analysis with CodeQL, static analysis, sanitizers, and fuzzing
- **docs.yml**: Documentation generation and validation
- **performance.yml**: Performance monitoring and regression detection
- **maintenance.yml**: Automated code maintenance

## Adding New Workflows

When adding new workflows:

1. Create a dedicated YAML file for the workflow
2. Use `workflow_call` trigger to make it callable from the main workflow
3. Use CMake commands for building and testing
4. Add a call to the workflow in ci.yml with appropriate conditions
5. Update this README to document the new workflow's purpose

## Dependencies

All workflows now include these common dependencies:

- `build-essential` - Essential build tools
- `cmake` - Build system
- `libjson-c-dev` - JSON library for C
- `pkg-config` - Package configuration
- `gperf` - Perfect hash function generator

Platform-specific dependencies are added as needed.

## Artifacts

Workflows generate these artifacts:

- **Build artifacts**: Binaries and libraries from each platform/compiler combination
- **Test results**: CTest outputs and coverage reports
- **Security reports**: Static analysis, sanitizer results, and vulnerability scans
- **Documentation**: Generated docs and quality reports
- **Performance data**: Benchmark results and profiling information

## Best Practices

### For Development
1. **Local Testing**: Use the same CMake commands locally before pushing
2. **Incremental Changes**: Start with basic functionality, add complexity gradually
3. **Platform Testing**: Focus on Linux first, then expand to macOS
4. **Security**: Run security workflows on important changes

### For Maintenance
- **Weekly Reviews**: Check scheduled workflow results
- **Dependency Updates**: Keep GitHub Actions and system dependencies current
- **Performance Monitoring**: Watch for regressions in performance workflows
- **Documentation**: Keep workflows and this README synchronized

## Troubleshooting

### Common Issues

1. **CMake Configuration Errors**: Check that all required dependencies are installed
2. **Build Failures**: Verify CMakeLists.txt is properly configured
3. **Test Failures**: Use `ctest --output-on-failure --verbose` for detailed output
4. **Missing Artifacts**: Check that build produces expected binaries

### Debug Tips

1. **Enable Verbose Output**: Add `--verbose` flags to CMake and CTest commands
2. **Check Build Directory**: Verify expected files are created in `build/`
3. **Review Artifacts**: Download workflow artifacts for detailed logs
4. **Local Reproduction**: Run the same commands locally to debug issues

## Security Considerations

- **Secrets Management**: Use GitHub Secrets for sensitive data
- **Minimal Permissions**: Workflows use only required permissions
- **Dependency Scanning**: Security workflow checks for vulnerabilities
- **Sanitizer Testing**: Memory safety testing with AddressSanitizer and UBSan

---

For more information about specific workflows, refer to the individual workflow files and their inline documentation. 
