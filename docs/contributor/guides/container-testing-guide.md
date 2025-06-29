# Container Testing Guide

This guide explains how to run BDD tests in a containerized environment that replicates the CI setup.

## Overview

The `run-bdd-tests-podman.sh` script allows you to test BDD scenarios in an isolated container environment that matches the GitHub Actions CI configuration. This helps catch platform-specific issues before pushing changes.

## Prerequisites

- **Podman** installed on your system (compatible with Docker)
- Sufficient disk space for container images (~2GB)
- Internet connection for downloading base images

## Quick Start

```bash
# Run all BDD tests in container
./run-bdd-tests-podman.sh
```

## What the Script Does

1. **Builds a container image** with Ubuntu 24.04 and LLVM 18
2. **Copies your source** to an isolated location inside the container
3. **Configures and builds** Asthra with BDD tests enabled
4. **Runs all BDD tests** and reports results
5. **Cleans up** automatically when done

## Container Environment

The container replicates the CI environment:
- **OS**: Ubuntu 24.04 LTS
- **Compiler**: Clang 18
- **LLVM**: Version 18 tools
- **Ruby**: For Cucumber tests
- **Build Type**: Debug with verbose output

## Safety Features

- **Read-only mount**: Your source is mounted read-only (`:ro`)
- **Isolated build**: All builds happen in `/home/runner/asthra-test`
- **Auto-cleanup**: Container is removed after completion (`--rm`)
- **No host pollution**: Your macOS/Linux build directory remains untouched

## Output Files

The script generates several log files inside the container:
- `cmake-configure.log` - CMake configuration output
- `cmake-build.log` - Build process output
- `bdd-logs/*.log` - Individual test results

These files exist only within the container and are removed after the run.

## Troubleshooting

### Permission Denied Errors
If you see "Permission denied" errors, ensure Podman has proper permissions:
```bash
podman system prune  # Clean up old containers
podman system reset  # Reset Podman (warning: removes all containers)
```

### Build Failures
The script shows the last 50 lines of build output on failure. For more details:
```bash
# Run with output to file for analysis
./run-bdd-tests-podman.sh 2>&1 | tee container-test.log
```

### Container Image Issues
If the container image is corrupted:
```bash
# Remove the existing image
podman rmi asthra-bdd-test

# Run the script again to rebuild
./run-bdd-tests-podman.sh
```

## Customization

### Using Different Ubuntu Version
Edit `Dockerfile.bdd-test`:
```dockerfile
FROM ubuntu:22.04  # Change to desired version
```

### Running Specific Tests
Modify the test loop in `run-bdd-tests-podman.sh`:
```bash
# Run only specific test
for test in bdd/bin/bdd_unit_compiler_basic; do
    # ... test execution ...
done
```

### Debugging Inside Container
To get an interactive shell:
```bash
podman run --rm -it \
    -v "$(pwd):/workspace:ro" \
    asthra-bdd-test \
    /bin/bash
```

## Integration with CI

This setup mirrors the GitHub Actions workflow in `.github/workflows/bdd.yml`. Key similarities:
- Same Ubuntu version
- Same LLVM/Clang version
- Same build configuration
- Same test execution pattern

## Best Practices

1. **Run before pushing**: Always run container tests before pushing BDD-related changes
2. **Check both environments**: Ensure tests pass both locally (macOS/Linux) and in container
3. **Update container**: When CI environment changes, update `Dockerfile.bdd-test` to match
4. **Clean regularly**: Run `podman system prune` periodically to free disk space

## Related Documentation

- [BDD Testing Guide](./bdd-testing-guide.md) - Writing and running BDD tests
- [Test Build Strategy](./test-build-strategy.md) - Building and running tests efficiently
- [CI/CD Documentation](../ci-cd.md) - Understanding the CI pipeline