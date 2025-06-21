# Asthra Programming Language Website

This repository contains the official documentation website for the Asthra programming language, built with [Docusaurus](https://docusaurus.io/).

## 🌐 Live Website

Visit the live website at: [https://asthra-lang.github.io/website](https://asthra-lang.github.io/website)

## 📁 Repository Structure

```
├── docs/                    # Documentation content (synced from main repo)
│   ├── spec/               # Language specification
│   ├── user-manual/        # User manual and guides
│   ├── stdlib/             # Standard library documentation
│   └── contributor/        # Contributor guides
├── src/                    # Website source code
├── static/                 # Static assets
├── blog/                   # Blog posts
├── sidebars*.ts           # Navigation configuration
├── docusaurus.config.ts   # Docusaurus configuration
└── package.json           # Dependencies and scripts
```

## 🚀 Quick Start

### Prerequisites

- Node.js 18+ 
- npm or yarn

### Installation

```bash
npm install
```

### Development

```bash
npm start
```

This command starts a local development server and opens up a browser window. Most changes are reflected live without having to restart the server.

### Build

```bash
npm run build
```

This command generates static content into the `build` directory and can be served using any static contents hosting service.

## 📝 Content Synchronization

The documentation content in the `docs/` directory is automatically synchronized from the main [Asthra compiler repository](https://github.com/asthra-lang/asthra) using GitHub Actions.

### Source Directories

The content is sourced from these directories in the main repository:
- `docs/spec/` → `docs/spec/`
- `docs/user-manual/` → `docs/user-manual/`  
- `docs/stdlib/` → `docs/stdlib/`
- `docs/contributor/` → `docs/contributor/`

### Automatic Sync

- **Trigger**: Pushes to the main repository's `develop` branch
- **Process**: GitHub Actions workflow copies content and applies MDX transformations
- **Deployment**: Automatically builds and deploys to GitHub Pages

### Manual Sync

To manually sync content during development:

```bash
# This will be implemented via GitHub Actions
# See .github/workflows/sync-docs.yml
```

## 🔧 MDX Transformations

The documentation undergoes several transformations to be compatible with MDX:

1. **HTML Entity Escaping**: Special characters are converted to HTML entities
2. **JSX Compatibility**: Angle brackets and other JSX-conflicting syntax is escaped
3. **Link Processing**: Internal links are updated for the website structure

These transformations are handled by:
- `fix_mdx_issues.py` - Main MDX compatibility fixes
- `fix_peg_arrows.py` - PEG grammar arrow fixes  
- `fix_ffi_file.py` - FFI documentation fixes

## 🏗️ Development Workflow

### Local Development

1. Clone this repository
2. Install dependencies: `npm install`
3. Start development server: `npm start`
4. Make changes to website structure, styling, or configuration
5. Test locally
6. Commit and push changes

### Content Updates

Content updates should be made in the main Asthra repository. The sync process will automatically update this website.

### Website Structure Updates

For changes to navigation, styling, or website configuration:
1. Make changes in this repository
2. Test locally with `npm start`
3. Build with `npm run build` to verify
4. Commit and push changes

## 📋 Available Scripts

- `npm start` - Start development server
- `npm run build` - Build for production
- `npm run serve` - Serve production build locally
- `npm run clear` - Clear Docusaurus cache
- `npm run deploy` - Deploy to GitHub Pages (automated via Actions)

## 🤝 Contributing

1. Fork this repository
2. Create a feature branch
3. Make your changes
4. Test locally
5. Submit a pull request

For content contributions, please contribute to the main [Asthra repository](https://github.com/asthra-lang/asthra) instead.

## 📄 License

This project is licensed under the same license as the Asthra programming language.

## 🔗 Links

- [Asthra Compiler Repository](https://github.com/asthra-lang/asthra)
- [Docusaurus Documentation](https://docusaurus.io/)
- [GitHub Pages](https://pages.github.com/)
