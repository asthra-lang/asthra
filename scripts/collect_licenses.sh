#!/bin/bash

# Collect all third-party licenses
LICENSES_DIR="third-party/licenses"
mkdir -p "$LICENSES_DIR"

echo "Collecting third-party license files..."

# Copy json-c license
if [ -f "third-party/json-c/COPYING" ]; then
    cp "third-party/json-c/COPYING" "$LICENSES_DIR/json-c-LICENSE"
    echo "✓ Collected json-c license"
else
    echo "⚠ json-c license file not found"
fi

# Generate combined license file
cat > "$LICENSES_DIR/README.md" << EOF
# Third-Party Licenses

This directory contains license files for all third-party dependencies used in the Asthra project.

## Dependencies

- **json-c**: JSON implementation in C
  - License: MIT-style (see json-c-LICENSE)
  - Source: https://github.com/json-c/json-c
  - Version: $(cd third-party/json-c && git describe --tags 2>/dev/null || echo "unknown")

## License Compliance

All dependencies have been reviewed for license compatibility with the Asthra project license.

### License Summary

| Dependency | License Type | Compatible | Notes |
|------------|-------------|------------|-------|
| json-c     | MIT-style   | ✓ Yes      | Permissive license, compatible with project |

### License Requirements

- All dependency licenses must be preserved and distributed with Asthra
- License text must be included in distribution packages
- Attribution requirements must be maintained
- Any modifications to dependencies must comply with their respective licenses

Last updated: $(date)
EOF

echo "✓ License collection complete"
echo "📁 License files available in: $LICENSES_DIR"
echo "📄 License summary generated: $LICENSES_DIR/README.md" 
