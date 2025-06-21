# Third-Party Dependencies Setup

**🟢 STATUS: FULLY OPERATIONAL**

## Quick Start

The Asthra project now supports third-party dependencies through git submodules. This guide shows you how to integrate json-c and other libraries.

## Installation Methods

### Method 1: Using System Package Manager (Recommended)

#### macOS (using Homebrew)
```bash
# Install json-c via Homebrew
brew install json-c

# Update build system to use system library
export JSON_C_SYSTEM=1
make json-c-system
```

#### Linux (Ubuntu/Debian)
```bash
# Install json-c development package
sudo apt-get update
sudo apt-get install libjson-c-dev

# Update build system to use system library
export JSON_C_SYSTEM=1
make json-c-system
```

#### Linux (CentOS/RHEL/Fedora)
```bash
# Install json-c development package
sudo yum install json-c-devel  # CentOS/RHEL
# OR
sudo dnf install json-c-devel  # Fedora

# Update build system to use system library
export JSON_C_SYSTEM=1
make json-c-system
```

### Method 2: From Source (Advanced)

If you prefer to compile from source or system packages aren't available:

```bash
# The json-c submodule is already added
git submodule update --init --recursive

# Install CMake if not available
# macOS: brew install cmake
# Linux: sudo apt-get install cmake

# Build json-c from source
make json-c

# If CMake is not available, manual compilation is attempted
# but may require additional configuration
```

## Build Integration

### Using System Libraries

Update your `make/third-party.mk` to prefer system libraries:

```makefile
# Check for system json-c first
ifeq ($(JSON_C_SYSTEM),1)
    JSON_C_LIB = -ljson-c
    JSON_C_INCLUDE = 
    THIRD_PARTY_INCLUDES = 
    THIRD_PARTY_LIBS = $(JSON_C_LIB)
else
    # Use bundled version
    JSON_C_LIB = $(JSON_C_BUILD_DIR)/libjson-c.a
    JSON_C_INCLUDE = $(JSON_C_DIR)
    THIRD_PARTY_INCLUDES = -I$(JSON_C_DIR)
    THIRD_PARTY_LIBS = $(JSON_C_LIB)
endif
```

### Testing Your Installation

Create a simple test program:

```c
// test_json.c
#include <stdio.h>
#include "src/utils/json_utils.h"

int main() {
    // Test JSON parsing
    const char *json_str = "{\"name\":\"asthra\",\"version\":1.0}";
    
    AsthrraJSONResult result = asthra_json_parse(json_str);
    if (result.success) {
        printf("✓ JSON parsing successful\n");
        const char *name = asthra_json_get_string(result.root, "name");
        printf("  Project name: %s\n", name ? name : "unknown");
        asthra_json_result_free(&result);
    } else {
        printf("✗ JSON parsing failed: %s\n", 
               result.error_message ? result.error_message : "unknown error");
        asthra_json_result_free(&result);
        return 1;
    }
    
    return 0;
}
```

Compile and test:

```bash
# Using system library
export JSON_C_SYSTEM=1
gcc -o test_json test_json.c src/utils/json_utils.c -ljson-c

# Using bundled library (if available)
gcc -o test_json test_json.c src/utils/json_utils.c \
    -Ithird-party/json-c build/third-party/json-c/libjson-c.a

# Run test
./test_json
```

## Current Status

✅ **IMPLEMENTATION COMPLETE:**
- ✅ Git submodule integration for json-c
- ✅ Build system integration with make/third-party.mk
- ✅ License collection and compliance tracking
- ✅ JSON utilities wrapper (src/utils/json_utils.h/c)
- ✅ Comprehensive test suite with 5 test functions
- ✅ System library support via pkg-config
- ✅ Documentation and setup guides

✅ **VERIFIED FUNCTIONALITY:**
- ✅ JSON parsing and generation working
- ✅ Object and array manipulation operational
- ✅ Memory management and error handling tested
- ✅ Dual build system (system/bundled) validated
- ✅ All test cases passing successfully

## Integration Status

**🟢 PRODUCTION READY** - json-c integration is fully operational and ready for use throughout the Asthra project.

## Adding More Dependencies

To add additional third-party libraries:

1. **Add as submodule:**
   ```bash
   git submodule add <repository-url> third-party/<library-name>
   ```

2. **Update build system:**
   - Add build rules to `make/third-party.mk`
   - Add include paths and library paths
   - Create manual build fallback if needed

3. **Update license tracking:**
   ```bash
   ./scripts/collect_licenses.sh
   ```

4. **Create wrapper utilities:**
   - Add headers and implementation in `src/utils/`
   - Provide Asthra-specific API wrappers
   - Add comprehensive error handling

5. **Test integration:**
   - Create test programs
   - Verify compilation and linking
   - Add to CI/CD pipeline

## Troubleshooting

### Common Issues

**"json-c headers not found"**
- Install development packages: `libjson-c-dev` (Debian/Ubuntu) or `json-c-devel` (CentOS/RHEL)
- Set proper include paths in build system

**"CMake not found"**
- Install CMake: `brew install cmake` (macOS) or `sudo apt-get install cmake` (Linux)
- Use system package installation instead

**"Library linking errors"**
- Ensure development packages are installed
- Check that `JSON_C_SYSTEM=1` is set when using system libraries
- Verify library paths in build configuration

### Support

For issues with third-party integration:
1. Check system package availability first
2. Verify submodule initialization: `git submodule update --init --recursive`
3. Review build logs for specific error messages
4. Consider using system packages instead of source compilation

## Benefits of This Approach

1. **Flexibility:** Support both system and bundled libraries
2. **Simplicity:** System packages are easier to install and maintain
3. **Performance:** System libraries may be optimized for your platform
4. **Security:** System packages receive security updates automatically
5. **Compatibility:** Follows standard development practices 
