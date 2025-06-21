use crate::error::Result;
// Remove unused imports - these are not used in this file currently
use std::env;
use std::path::Path;
use tracing::{info, warn, error};

pub async fn handle_init(name: Option<String>, lib: bool) -> Result<()> {
    let current_dir = env::current_dir()?;
    
    let project_name = match name {
        Some(n) => n,
        None => {
            // Use current directory name as project name
            current_dir
                .file_name()
                .and_then(|n| n.to_str())
                .unwrap_or("asthra-project")
                .to_string()
        }
    };
    
    info!("Initializing new Asthra project: {}", project_name);
    
    if lib {
        info!("Creating library project");
    } else {
        info!("Creating binary project");
    }
    
    crate::init_project(&project_name, &current_dir, lib).await?;
    
    info!("✅ Project initialized successfully!");
    info!("Run 'ampu build' to build your project");
    
    Ok(())
}

pub async fn handle_add(package: String, dev: bool, version: Option<String>) -> Result<()> {
    info!("Adding dependency: {}", package);
    
    if dev {
        info!("Adding as dev dependency");
    }
    
    if let Some(ref v) = version {
        info!("Using version: {}", v);
    }
    
    let current_dir = env::current_dir()?;
    let asthra_toml_path = current_dir.join("asthra.toml");
    
    if !asthra_toml_path.exists() {
        return Err(crate::error::BuildError::Compiler(
            "No asthra.toml found. Run 'ampu init' first.".to_string()
        ));
    }
    
    // Load current asthra.toml
    let content = tokio::fs::read_to_string(&asthra_toml_path).await?;
    let mut asthra_mod: crate::config::AsthraMod = toml::from_str(&content)?;
    
    // Add dependency
    let version_req = version.unwrap_or_else(|| "*".to_string());
    if dev {
        asthra_mod.dev_dependencies.insert(package.clone(), version_req);
    } else {
        asthra_mod.dependencies.insert(package.clone(), version_req);
    }
    
    // Save updated asthra.toml
    let updated_content = toml::to_string_pretty(&asthra_mod)?;
    tokio::fs::write(&asthra_toml_path, updated_content).await?;
    
    info!("✅ Added dependency: {}", package);
    info!("Run 'ampu build' to download and build the dependency");
    
    Ok(())
}

pub async fn handle_remove(package: String, dev: bool) -> Result<()> {
    info!("Removing dependency: {}", package);
    
    let current_dir = env::current_dir()?;
    let asthra_toml_path = current_dir.join("asthra.toml");
    
    if !asthra_toml_path.exists() {
        return Err(crate::error::BuildError::Compiler(
            "No asthra.toml found.".to_string()
        ));
    }
    
    // Load current asthra.toml
    let content = tokio::fs::read_to_string(&asthra_toml_path).await?;
    let mut asthra_mod: crate::config::AsthraMod = toml::from_str(&content)?;
    
    // Remove dependency
    let removed = if dev {
        asthra_mod.dev_dependencies.remove(&package).is_some()
    } else {
        asthra_mod.dependencies.remove(&package).is_some()
    };
    
    if !removed {
        warn!("Dependency '{}' not found in {}", package, if dev { "dev-dependencies" } else { "dependencies" });
        return Ok(());
    }
    
    // Save updated asthra.toml
    let updated_content = toml::to_string_pretty(&asthra_mod)?;
    tokio::fs::write(&asthra_toml_path, updated_content).await?;
    
    info!("✅ Removed dependency: {}", package);
    
    Ok(())
}

pub async fn handle_update(package: Option<String>) -> Result<()> {
    match package {
        Some(pkg) => info!("Updating dependency: {}", pkg),
        None => info!("Updating all dependencies"),
    }
    
    let current_dir = env::current_dir()?;
    let lock_file_path = current_dir.join("asthra.lock");
    
    // Remove lock file to force re-resolution
    if lock_file_path.exists() {
        tokio::fs::remove_file(&lock_file_path).await?;
        info!("Removed lock file to force dependency update");
    }
    
    // Re-resolve dependencies
    let mut resolver = crate::downloader::DependencyResolver::new();
    let asthra_mod = crate::get_project_info(&current_dir).await?;
    
    let resolved = resolver.resolve_dependencies(&asthra_mod.dependencies).await?;
    info!("✅ Updated {} dependencies", resolved.packages.len());
    
    Ok(())
}

pub async fn handle_list(tree: bool) -> Result<()> {
    let current_dir = env::current_dir()?;
    let asthra_mod = crate::get_project_info(&current_dir).await?;
    
    if tree {
        info!("Dependency tree:");
        print_dependency_tree(&asthra_mod.dependencies, 0).await?;
    } else {
        info!("Dependencies:");
        for (name, version) in &asthra_mod.dependencies {
            println!("  {} = \"{}\"", name, version);
        }
        
        if !asthra_mod.dev_dependencies.is_empty() {
            info!("Dev dependencies:");
            for (name, version) in &asthra_mod.dev_dependencies {
                println!("  {} = \"{}\"", name, version);
            }
        }
    }
    
    Ok(())
}

async fn print_dependency_tree(dependencies: &std::collections::HashMap<String, String>, depth: usize) -> Result<()> {
    let indent = "  ".repeat(depth);
    for (name, version) in dependencies {
        println!("{}├── {} = \"{}\"", indent, name, version);
        // TODO: Recursively print transitive dependencies
    }
    Ok(())
}

pub async fn handle_clean(all: bool, profile: Option<String>, deps: bool, cache: bool) -> Result<()> {
    let current_dir = env::current_dir()?;
    
    if all {
        info!("Cleaning all build artifacts");
        let target_dir = current_dir.join("target");
        if target_dir.exists() {
            tokio::fs::remove_dir_all(&target_dir).await?;
            info!("✅ Cleaned all build artifacts");
        }
        return Ok(());
    }
    
    if let Some(profile_name) = profile {
        info!("Cleaning {} profile", profile_name);
        let profile_dir = current_dir.join("target").join(&profile_name);
        if profile_dir.exists() {
            tokio::fs::remove_dir_all(&profile_dir).await?;
            info!("✅ Cleaned {} profile", profile_name);
        }
        return Ok(());
    }
    
    if deps {
        info!("Cleaning dependency libraries");
        let deps_dir = current_dir.join("target").join("debug").join("deps");
        if deps_dir.exists() {
            tokio::fs::remove_dir_all(&deps_dir).await?;
        }
        let deps_dir = current_dir.join("target").join("release").join("deps");
        if deps_dir.exists() {
            tokio::fs::remove_dir_all(&deps_dir).await?;
        }
        info!("✅ Cleaned dependency libraries");
        return Ok(());
    }
    
    if cache {
        info!("Cleaning build cache");
        let cache_dir = current_dir.join("target").join("cache");
        if cache_dir.exists() {
            tokio::fs::remove_dir_all(&cache_dir).await?;
        }
        info!("✅ Cleaned build cache");
        return Ok(());
    }
    
    // Default clean behavior
    info!("Cleaning build artifacts");
    let target_dir = current_dir.join("target");
    if target_dir.exists() {
        tokio::fs::remove_dir_all(&target_dir).await?;
        info!("✅ Cleaned target directory");
    } else {
        info!("Target directory doesn't exist, nothing to clean");
    }
    
    Ok(())
}

pub async fn handle_check(access_control: bool) -> Result<()> {
    info!("Checking project");
    
    let current_dir = env::current_dir()?;
    
    // Validate that this is an Asthra project
    crate::validate_project(&current_dir).await?;
    info!("✅ Project structure is valid");
    
    if access_control {
        info!("Checking access control rules");
        let validator = crate::resolver::access_control::BuildValidator::new()?;
        validator.validate_project(&current_dir).await?;
        info!("✅ Access control validation passed");
    }
    
    info!("✅ All checks passed");
    Ok(())
}

pub async fn handle_deps(tree: bool, graph: bool, format: Option<String>) -> Result<()> {
    let current_dir = env::current_dir()?;
    let asthra_mod = crate::get_project_info(&current_dir).await?;
    
    if graph {
        let format = format.unwrap_or_else(|| "text".to_string());
        info!("Generating dependency graph in {} format", format);
        generate_dependency_graph(&asthra_mod, &format).await?;
    } else if tree {
        info!("Dependency tree:");
        print_dependency_tree(&asthra_mod.dependencies, 0).await?;
    } else {
        info!("Dependency information:");
        println!("Package: {} v{}", asthra_mod.package.name, asthra_mod.package.version);
        println!("Dependencies: {}", asthra_mod.dependencies.len());
        println!("Dev dependencies: {}", asthra_mod.dev_dependencies.len());
    }
    
    Ok(())
}

async fn generate_dependency_graph(asthra_mod: &crate::config::AsthraMod, format: &str) -> Result<()> {
    match format {
        "dot" => {
            println!("digraph dependencies {{");
            println!("  \"{}\" [shape=box];", asthra_mod.package.name);
            for (dep, _) in &asthra_mod.dependencies {
                println!("  \"{}\" -> \"{}\";", asthra_mod.package.name, dep);
            }
            println!("}}");
        }
        "json" => {
            let graph = serde_json::json!({
                "nodes": [asthra_mod.package.name],
                "edges": asthra_mod.dependencies.keys().collect::<Vec<_>>()
            });
            println!("{}", serde_json::to_string_pretty(&graph)?);
        }
        _ => {
            // Text format
            println!("{}", asthra_mod.package.name);
            for (dep, version) in &asthra_mod.dependencies {
                println!("├── {} ({})", dep, version);
            }
        }
    }
    Ok(())
}

pub async fn handle_vendor() -> Result<()> {
    info!("Vendoring dependencies locally");
    
    let current_dir = env::current_dir()?;
    let vendor_dir = current_dir.join("vendor");
    
    // Create vendor directory
    tokio::fs::create_dir_all(&vendor_dir).await?;
    
    let asthra_mod = crate::get_project_info(&current_dir).await?;
    let mut resolver = crate::downloader::DependencyResolver::new();
    let resolved = resolver.resolve_dependencies(&asthra_mod.dependencies).await?;
    
    // Copy dependencies to vendor directory
    for (name, _package) in &resolved.packages {
        let source_path = current_dir.join(".asthra/cache").join(name);
        let dest_path = vendor_dir.join(name);
        
        if source_path.exists() {
            copy_dir_recursive(&source_path, &dest_path).await?;
            info!("Vendored: {}", name);
        }
    }
    
    info!("✅ Vendored {} dependencies to ./vendor/", resolved.packages.len());
    Ok(())
}

fn copy_dir_recursive<'a>(src: &'a Path, dst: &'a Path) -> std::pin::Pin<Box<dyn std::future::Future<Output = Result<()>> + Send + 'a>> {
    Box::pin(async move {
        tokio::fs::create_dir_all(dst).await?;
        
        let mut entries = tokio::fs::read_dir(src).await?;
        while let Some(entry) = entries.next_entry().await? {
            let src_path = entry.path();
            let dst_path = dst.join(entry.file_name());
            
            if src_path.is_dir() {
                copy_dir_recursive(&src_path, &dst_path).await?;
            } else {
                tokio::fs::copy(&src_path, &dst_path).await?;
            }
        }
        
        Ok(())
    })
}

pub async fn handle_why(package: String) -> Result<()> {
    info!("Explaining why package '{}' is included", package);
    
    let current_dir = env::current_dir()?;
    let asthra_mod = crate::get_project_info(&current_dir).await?;
    
    // Check direct dependencies
    if asthra_mod.dependencies.contains_key(&package) {
        println!("'{}' is a direct dependency", package);
        if let Some(version) = asthra_mod.dependencies.get(&package) {
            println!("  Required version: {}", version);
        }
        return Ok(());
    }
    
    if asthra_mod.dev_dependencies.contains_key(&package) {
        println!("'{}' is a direct dev dependency", package);
        if let Some(version) = asthra_mod.dev_dependencies.get(&package) {
            println!("  Required version: {}", version);
        }
        return Ok(());
    }
    
    // TODO: Check transitive dependencies
    println!("'{}' is not found in dependencies", package);
    println!("It might be a transitive dependency (analysis not yet implemented)");
    
    Ok(())
}

pub async fn handle_graph(format: String, output: Option<String>) -> Result<()> {
    info!("Generating dependency graph in {} format", format);
    
    let current_dir = env::current_dir()?;
    let asthra_mod = crate::get_project_info(&current_dir).await?;
    
    let graph_content = match format.as_str() {
        "dot" => {
            let mut content = String::from("digraph dependencies {\n");
            content.push_str(&format!("  \"{}\" [shape=box];\n", asthra_mod.package.name));
            for (dep, _) in &asthra_mod.dependencies {
                content.push_str(&format!("  \"{}\" -> \"{}\";\n", asthra_mod.package.name, dep));
            }
            content.push_str("}\n");
            content
        }
        "json" => {
            let graph = serde_json::json!({
                "nodes": [asthra_mod.package.name],
                "edges": asthra_mod.dependencies.keys().collect::<Vec<_>>()
            });
            serde_json::to_string_pretty(&graph)?
        }
        _ => {
            let mut content = format!("{}\n", asthra_mod.package.name);
            for (dep, version) in &asthra_mod.dependencies {
                content.push_str(&format!("├── {} ({})\n", dep, version));
            }
            content
        }
    };
    
    if let Some(output_path) = output {
        tokio::fs::write(&output_path, &graph_content).await?;
        info!("✅ Graph saved to: {}", output_path);
    } else {
        print!("{}", graph_content);
    }
    
    Ok(())
}

pub async fn handle_doc(open: bool, format: Option<String>, serve: bool, port: u16) -> Result<()> {
    let current_dir = env::current_dir()?;
    let docs_dir = current_dir.join("docs");
    
    if serve {
        info!("Starting documentation server on port {}", port);
        start_doc_server(&docs_dir, port).await?;
        return Ok(());
    }
    
    info!("Generating documentation");
    
    let format = format.unwrap_or_else(|| "html".to_string());
    
    match format.as_str() {
        "html" => generate_html_docs(&current_dir, &docs_dir).await?,
        "markdown" => generate_markdown_docs(&current_dir, &docs_dir).await?,
        _ => {
            error!("Unsupported documentation format: {}", format);
            return Err(crate::error::BuildError::Compiler(
                format!("Unsupported documentation format: {}", format)
            ));
        }
    }
    
    info!("✅ Documentation generated in: {}", docs_dir.display());
    
    if open {
        let index_path = docs_dir.join("index.html");
        if index_path.exists() {
            info!("Opening documentation in browser");
            open_in_browser(&index_path).await?;
        }
    }
    
    Ok(())
}

async fn generate_html_docs(project_dir: &Path, docs_dir: &Path) -> Result<()> {
    tokio::fs::create_dir_all(docs_dir).await?;
    
    let asthra_mod = crate::get_project_info(project_dir).await?;
    
    let html_content = format!(
        r#"<!DOCTYPE html>
<html>
<head>
    <title>{} Documentation</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 40px; }}
        h1 {{ color: #333; }}
        .dependency {{ margin: 10px 0; padding: 10px; background: #f5f5f5; }}
    </style>
</head>
<body>
    <h1>{} v{}</h1>
    <p>Asthra package documentation</p>
    
    <h2>Dependencies</h2>
    {}
    
    <h2>Dev Dependencies</h2>
    {}
</body>
</html>"#,
        asthra_mod.package.name,
        asthra_mod.package.name,
        asthra_mod.package.version,
        format_dependencies_html(&asthra_mod.dependencies),
        format_dependencies_html(&asthra_mod.dev_dependencies)
    );
    
    let index_path = docs_dir.join("index.html");
    tokio::fs::write(&index_path, html_content).await?;
    
    Ok(())
}

fn format_dependencies_html(deps: &std::collections::HashMap<String, String>) -> String {
    if deps.is_empty() {
        return "<p>No dependencies</p>".to_string();
    }
    
    let mut html = String::new();
    for (name, version) in deps {
        html.push_str(&format!(
            r#"<div class="dependency"><strong>{}</strong> = "{}"</div>"#,
            name, version
        ));
    }
    html
}

async fn generate_markdown_docs(project_dir: &Path, docs_dir: &Path) -> Result<()> {
    tokio::fs::create_dir_all(docs_dir).await?;
    
    let asthra_mod = crate::get_project_info(project_dir).await?;
    
    let markdown_content = format!(
        r#"# {} v{}

Asthra package documentation

## Dependencies

{}

## Dev Dependencies

{}
"#,
        asthra_mod.package.name,
        asthra_mod.package.version,
        format_dependencies_markdown(&asthra_mod.dependencies),
        format_dependencies_markdown(&asthra_mod.dev_dependencies)
    );
    
    let readme_path = docs_dir.join("README.md");
    tokio::fs::write(&readme_path, markdown_content).await?;
    
    Ok(())
}

fn format_dependencies_markdown(deps: &std::collections::HashMap<String, String>) -> String {
    if deps.is_empty() {
        return "No dependencies".to_string();
    }
    
    let mut markdown = String::new();
    for (name, version) in deps {
        markdown.push_str(&format!("- **{}** = \"{}\"\n", name, version));
    }
    markdown
}

async fn start_doc_server(docs_dir: &Path, port: u16) -> Result<()> {
    info!("Documentation server would start on http://localhost:{}", port);
    info!("Serving files from: {}", docs_dir.display());
    
    // TODO: Implement actual HTTP server
    warn!("Documentation server not yet implemented");
    
    Ok(())
}

async fn open_in_browser(path: &Path) -> Result<()> {
    let url = format!("file://{}", path.display());
    
    #[cfg(target_os = "macos")]
    {
        std::process::Command::new("open").arg(&url).spawn()?;
    }
    
    #[cfg(target_os = "linux")]
    {
        std::process::Command::new("xdg-open").arg(&url).spawn()?;
    }
    
    #[cfg(target_os = "windows")]
    {
        std::process::Command::new("cmd").args(&["/c", "start", &url]).spawn()?;
    }
    
    Ok(())
} 
