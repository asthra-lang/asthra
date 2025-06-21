# Third-Party Dependencies

This directory contains external libraries integrated into the Asthra project.

## Current Dependencies

### json-c
- **Purpose**: JSON parsing and generation for Asthra's JSON API and configuration
- **Version**: Latest stable
- **License**: MIT-style (see licenses/json-c-LICENSE)
- **Homepage**: https://github.com/json-c/json-c
- **Integration**: Git submodule

## Adding New Dependencies

1. Add as git submodule: `git submodule add <repo-url> third-party/<name>`
2. Update `make/third-party.mk` with build rules
3. Run `scripts/collect_licenses.sh` to update license collection
4. Update this README with dependency information
5. Test build integration with `make third-party`

## Building Dependencies

```bash
# Build all third-party dependencies
make third-party

# Build specific dependency
make json-c

# Clean third-party builds
make clean-third-party

# Get help with third-party targets
make help-third-party
```

## License Compliance

All third-party dependencies must:
- Have compatible licenses with Asthra's license
- Include original license files in `licenses/` directory
- Be documented in this README
- Have license compatibility verified

## Directory Structure

```
third-party/
├── README.md                # This file
├── json-c/                  # json-c library source (git submodule)
└── licenses/                # Collected license files
    └── json-c-LICENSE       # Copy of json-c license
```

## Integration Notes

- All third-party libraries are built as static libraries for easier distribution
- Include paths are automatically added to compiler flags
- Libraries are linked automatically when building Asthra
- Use `#include <json-c/json.h>` to include json-c headers in Asthra source code 
