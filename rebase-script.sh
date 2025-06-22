#!/bin/bash
# Interactive rebase helper script

# Start interactive rebase
git rebase -i main

# The rebase plan:
# 1. Keep the main feature commit: "feat(types): complete Result<T, E> built-in type implementation"
# 2. Squash all Result/Option test fixes into one commit
# 3. Squash all security/deprecated function fixes into one commit  
# 4. Squash all build/CI improvements into one commit
# 5. Squash all test framework fixes into one commit
# 6. Squash all miscellaneous fixes into one commit
# 7. Drop the merge commit and trigger commits