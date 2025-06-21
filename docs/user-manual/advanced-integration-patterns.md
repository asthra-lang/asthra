# IDE Integration Guide

**Purpose**: Complete setup guide for Asthra development environments  
**Target Audience**: Developers setting up Asthra development workflow  
**Prerequisites**: Basic familiarity with text editors and development tools

This guide covers IDE and editor integration for optimal Asthra development experience with syntax highlighting, intelligent code completion, and debugging support.

## Table of Contents

- [Language Server Protocol (LSP) Support](#language-server-protocol-lsp-support)
- [VS Code Integration](#vs-code-integration)
- [Vim/Neovim Configuration](#vimneovim-configuration)
- [Emacs Setup](#emacs-setup)
- [IntelliJ/CLion Integration](#intellijclion-integration)
- [Build System Integration](#build-system-integration)
- [Debugging Configuration](#debugging-configuration)

## Language Server Protocol (LSP) Support

Asthra provides full LSP support through the `asthra-lsp` server for cross-editor compatibility.

### Installation

```bash
# Install Asthra LSP server
cargo install asthra-lsp

# Verify installation
asthra-lsp --version
# Expected: asthra-lsp 0.1.0 (compatible with Asthra v1.25)
```

### LSP Capabilities

The Asthra LSP server provides:

- **Syntax Highlighting**: Full grammar v1.25 support with semantic tokens
- **Code Completion**: Intelligent suggestions for keywords, types, and identifiers
- **Error Diagnostics**: Real-time syntax and semantic error reporting
- **Go to Definition**: Navigate to symbol definitions across files
- **Hover Information**: Type information and documentation on hover
- **Code Formatting**: Integration with `ampu fmt` for consistent style
- **Refactoring**: Safe renaming and code transformation

### Configuration

Create `asthra-lsp.toml` in your project root:

```toml
[language_server]
name = "asthra-lsp"
version = "0.1.0"

[capabilities]
syntax_highlighting = true
code_completion = true
error_diagnostics = true
goto_definition = true
hover_information = true
code_formatting = true
refactoring = true

[grammar]
version = "v1.25"
strict_mode = true
ai_friendly_suggestions = true

[formatting]
tool = "ampu fmt"
on_save = true
line_width = 100
indent_style = "spaces"
indent_size = 4

[diagnostics]
max_errors = 50
include_warnings = true
include_hints = true
real_time_checking = true

[performance]
completion_timeout_ms = 1000
diagnostics_debounce_ms = 300
max_file_size_mb = 10
```

## VS Code Integration

### Extension Installation

Install the official Asthra extension from the VS Code marketplace:

```bash
# Search for "asthra" in VS Code extensions
# Or install via command line
code --install-extension asthra-lang.asthra-vscode
```

### Workspace Configuration

Create `.vscode/settings.json` in your project:

```json
{
  "asthra.languageServer.enabled": true,
  "asthra.languageServer.path": "asthra-lsp",
  "asthra.formatting.provider": "ampu",
  "asthra.formatting.formatOnSave": true,
  "asthra.diagnostics.enableSemanticHighlighting": true,
  "asthra.completion.enableSnippets": true,
  "asthra.completion.enableAutoImport": true,
  
  "files.associations": {
    "*.asthra": "asthra"
  },
  
  "editor.tabSize": 4,
  "editor.insertSpaces": true,
  "editor.formatOnSave": true,
  "editor.codeActionsOnSave": {
    "source.organizeImports": true,
    "source.fixAll": true
  },
  
  "asthra.build.autoDetect": "on",
  "asthra.build.defaultTarget": "debug",
  "asthra.testing.autoDiscover": true
}
```

### Tasks Configuration

Create `.vscode/tasks.json` for build integration:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build Asthra Project",
      "type": "shell",
      "command": "make",
      "args": ["clean", "&&", "make"],
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared"
      },
      "problemMatcher": {
        "owner": "asthra",
        "fileLocation": "relative",
        "pattern": {
          "regexp": "^(.*):(\\d+):(\\d+):\\s+(warning|error):\\s+(.*)$",
          "file": 1,
          "line": 2,
          "column": 3,
          "severity": 4,
          "message": 5
        }
      }
    },
    {
      "label": "Run Tests",
      "type": "shell", 
      "command": "make",
      "args": ["test"],
      "group": "test",
      "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared"
      }
    },
    {
      "label": "Format Code",
      "type": "shell",
      "command": "ampu",
      "args": ["fmt", "${workspaceFolder}"],
      "group": "build"
    }
  ]
}
```

### Launch Configuration

Create `.vscode/launch.json` for debugging:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug Asthra Application",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/bin/main",
      "args": [],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "lldb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ],
      "preLaunchTask": "Build Asthra Project"
    }
  ]
}
```

## Vim/Neovim Configuration

### Using coc.nvim

Install the Asthra language server integration:

```vim
" Add to ~/.vimrc or ~/.config/nvim/init.vim

" Install coc.nvim if not already installed
Plug 'neoclide/coc.nvim', {'branch': 'release'}

" Asthra-specific configuration
autocmd FileType asthra setlocal tabstop=4 shiftwidth=4 expandtab
autocmd FileType asthra setlocal commentstring=//\ %s

" CocConfig for Asthra LSP
let g:coc_user_config = {
\   'languageserver': {
\     'asthra': {
\       'command': 'asthra-lsp',
\       'filetypes': ['asthra'],
\       'rootPatterns': ['Makefile', 'asthra.toml', '.git/'],
\       'settings': {
\         'asthra': {
\           'completion': {'enableSnippets': v:true},
\           'diagnostics': {'enable': v:true},
\           'formatting': {'provider': 'ampu'}
\         }
\       }
\     }
\   }
\ }

" Keybindings for Asthra development
autocmd FileType asthra nnoremap <buffer> <leader>gd :call CocAction('jumpDefinition')<CR>
autocmd FileType asthra nnoremap <buffer> <leader>gr :call CocAction('jumpReferences')<CR>
autocmd FileType asthra nnoremap <buffer> <leader>rn :call CocAction('rename')<CR>
autocmd FileType asthra nnoremap <buffer> <leader>f :call CocAction('format')<CR>

" Build and test shortcuts
autocmd FileType asthra nnoremap <buffer> <F5> :!make clean && make<CR>
autocmd FileType asthra nnoremap <buffer> <F6> :!make test<CR>
```

### Using native LSP (Neovim 0.5+)

```lua
-- Add to ~/.config/nvim/init.lua

local lspconfig = require('lspconfig')
local configs = require('lspconfig.configs')

-- Configure Asthra LSP
if not configs.asthra_lsp then
  configs.asthra_lsp = {
    default_config = {
      cmd = {'asthra-lsp'},
      filetypes = {'asthra'},
      root_dir = lspconfig.util.root_pattern('Makefile', 'asthra.toml', '.git'),
      settings = {
        asthra = {
          completion = {enableSnippets = true},
          diagnostics = {enable = true},
          formatting = {provider = 'ampu'}
        }
      }
    }
  }
end

lspconfig.asthra_lsp.setup{
  on_attach = function(client, bufnr)
    -- Enable completion triggered by <c-x><c-o>
    vim.api.nvim_buf_set_option(bufnr, 'omnifunc', 'v:lua.vim.lsp.omnifunc')
    
    -- Mappings
    local opts = { noremap=true, silent=true }
    vim.api.nvim_buf_set_keymap(bufnr, 'n', 'gD', '<cmd>lua vim.lsp.buf.declaration()<CR>', opts)
    vim.api.nvim_buf_set_keymap(bufnr, 'n', 'gd', '<cmd>lua vim.lsp.buf.definition()<CR>', opts)
    vim.api.nvim_buf_set_keymap(bufnr, 'n', 'K', '<cmd>lua vim.lsp.buf.hover()<CR>', opts)
    vim.api.nvim_buf_set_keymap(bufnr, 'n', '<leader>rn', '<cmd>lua vim.lsp.buf.rename()<CR>', opts)
    vim.api.nvim_buf_set_keymap(bufnr, 'n', '<leader>ca', '<cmd>lua vim.lsp.buf.code_action()<CR>', opts)
    vim.api.nvim_buf_set_keymap(bufnr, 'n', '<leader>f', '<cmd>lua vim.lsp.buf.formatting()<CR>', opts)
  end,
  flags = {
    debounce_text_changes = 150,
  }
}

-- Asthra-specific settings
vim.cmd([[
  autocmd FileType asthra setlocal tabstop=4 shiftwidth=4 expandtab
  autocmd FileType asthra setlocal commentstring=//\ %s
]])
```

## Build System Integration

### Make Integration

Asthra projects use a sophisticated Make-based build system. Configure your IDE to leverage this:

#### Build Targets

```bash
# Core build commands
make                    # Default build
make clean && make      # Clean build
make debug             # Debug build
make release           # Release build

# Testing
make test              # Run all tests
make test-category CATEGORY=parser  # Run specific category
make test-fast         # Quick smoke tests

# Code quality
make lint              # Run linters
make format            # Format code
make check             # Static analysis
```

#### IDE Integration Examples

**VS Code Build Tasks** (add to tasks.json):
```json
{
  "label": "Quick Build",
  "type": "shell",
  "command": "make",
  "group": "build",
  "presentation": {"reveal": "silent"}
},
{
  "label": "Full Clean Build", 
  "type": "shell",
  "command": "make",
  "args": ["clean", "&&", "make", "release"],
  "group": "build"
}
```

**Vim/Neovim Shortcuts**:
```vim
" Quick build
nnoremap <F5> :!make<CR>
" Clean build  
nnoremap <S-F5> :!make clean && make<CR>
" Run tests
nnoremap <F6> :!make test<CR>
```

### Ampu Integration

The `ampu` tool provides project management and advanced features:

```bash
# Project initialization
ampu new my_project --template=cli
ampu new my_lib --template=library

# Build management
ampu build                    # Smart build
ampu build --release         # Release build
ampu build --target=wasm     # Cross-compilation

# Code quality
ampu fmt                     # Format code
ampu check                   # Type checking
ampu test                    # Run tests
ampu bench                   # Benchmarks

# Documentation
ampu doc                     # Generate docs
ampu doc --serve             # Serve docs locally
```

### CI/CD Integration

Example GitHub Actions workflow (`.github/workflows/ci.yml`):

```yaml
name: Asthra CI

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    
    - name: Install Asthra
      run: |
        curl -sSL https://asthra-lang.org/install.sh | sh
        echo "$HOME/.asthra/bin" >> $GITHUB_PATH
    
    - name: Build
      run: make clean && make
      
    - name: Test
      run: make test
      
    - name: Format Check
      run: ampu fmt --check
      
    - name: Lint
      run: make lint
```

## Debugging Configuration

### LLDB Integration

Asthra generates debug-friendly C code that works well with LLDB:

```bash
# Build with debug symbols
make debug

# Debug with LLDB
lldb ./bin/main
(lldb) breakpoint set --name main
(lldb) run
(lldb) step
(lldb) print variable_name
```

### GDB Integration

```bash
# Debug with GDB
gdb ./bin/main
(gdb) break main
(gdb) run
(gdb) step
(gdb) print variable_name
```

### IDE Debugging Setup

Most IDEs can debug Asthra programs through the generated C code:

1. **Build Type**: Use `make debug` for debug symbols
2. **Debugger**: Configure for C/C++ debugging (GDB/LLDB)
3. **Source Maps**: Point to generated C files in `build/` directory
4. **Breakpoints**: Set in Asthra source files (LSP will map to C code)

## Development Workflow Optimization

### Recommended Workflow

1. **Project Setup**:
   ```bash
   ampu new my_project
   cd my_project
   code .  # Open in VS Code
   ```

2. **Daily Development**:
   ```bash
   # Edit code with LSP support
   # Use Ctrl+S to auto-format and check
   make test     # Run tests frequently
   make          # Build when ready
   ```

3. **Code Quality**:
   ```bash
   ampu fmt      # Format before commits
   make lint     # Check for issues
   make test     # Ensure tests pass
   ```

4. **Debugging**:
   - Use IDE integrated debugging for interactive sessions
   - Use `make debug` for debug builds
   - Set breakpoints in Asthra source files

### Performance Tips

- **LSP Performance**: Configure appropriate timeouts for large projects
- **Build Speed**: Use `make -j$(nproc)` for parallel builds
- **Editor Settings**: Enable incremental compilation where available
- **File Watching**: Configure editors to watch `.asthra` files specifically

This guide provides comprehensive IDE integration for productive Asthra development with modern tooling support. 
