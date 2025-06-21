#!/bin/bash

# Fix register_allocator_create calls (remove parameter)
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/register_allocator_create([^)]*)/register_allocator_create()/g' {} \;

# Fix interference_graph_create calls (remove parameter)
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/interference_graph_create([^)]*)/interference_graph_create()/g' {} \;

# Fix label_manager_create calls (add parameter)
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/label_manager_create()/label_manager_create(16)/g' {} \;

# Fix liveness_analysis_create calls (remove parameter)
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/liveness_analysis_create([^)]*)/liveness_analysis_create()/g' {} \;

# Fix spill_manager_create calls (remove parameter)
find tests/codegen -name "*.c" -type f -exec sed -i '' \
    's/spill_manager_create([^)]*)/spill_manager_create()/g' {} \;

echo "Fixed codegen test function calls"