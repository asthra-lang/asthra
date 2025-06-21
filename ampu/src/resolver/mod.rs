pub mod import_parser;
pub mod access_control;

use crate::config::PackageType;
use crate::error::{BuildError, Result};
use std::collections::{HashMap, HashSet};
use std::path::PathBuf;

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ImportType {
    Stdlib(String),      // "stdlib/string", "stdlib/asthra/core"
    Internal(String),    // "internal/runtime/memory_allocator"
    ThirdParty(String),  // "github.com/user/package"
    Local(String),       // "./utils", "../common"
}

#[derive(Debug, Clone)]
pub struct DependencyNode {
    pub import_path: String,
    pub version: semver::Version,
    pub dependencies: HashMap<String, semver::VersionReq>,
}

#[derive(Debug)]
pub struct ResolvedDependencies {
    pub packages: HashMap<String, DependencyNode>,
}

pub struct ImportGraph {
    nodes: HashMap<String, PackageNode>,
    edges: Vec<DependencyEdge>,
}

#[derive(Debug, Clone)]
struct PackageNode {
    name: String,
    package_type: PackageType,
    imports: Vec<ImportType>,
}

#[derive(Debug, Clone)]
struct DependencyEdge {
    from: String,
    to: String,
}

impl ImportGraph {
    pub fn new() -> Self {
        Self {
            nodes: HashMap::new(),
            edges: Vec::new(),
        }
    }

    pub fn add_package(&mut self, name: String, package_type: PackageType, imports: Vec<ImportType>) {
        let node = PackageNode {
            name: name.clone(),
            package_type,
            imports,
        };
        self.nodes.insert(name, node);
    }

    pub fn build_from_imports(&mut self, imports: Vec<ImportType>) -> Result<()> {
        // Build dependency graph from imports
        for import in imports {
            match import {
                ImportType::ThirdParty(path) => {
                    self.add_dependency_edge("root".to_string(), path);
                }
                ImportType::Stdlib(path) => {
                    self.add_dependency_edge("root".to_string(), path);
                }
                ImportType::Internal(path) => {
                    self.add_dependency_edge("root".to_string(), path);
                }
                ImportType::Local(_) => {
                    // Local imports don't create external dependencies
                }
            }
        }
        Ok(())
    }

    fn add_dependency_edge(&mut self, from: String, to: String) {
        self.edges.push(DependencyEdge { from, to });
    }

    pub fn topological_sort(&self) -> Result<Vec<String>> {
        // Simple topological sort implementation
        let mut in_degree: HashMap<String, usize> = HashMap::new();
        let mut adj_list: HashMap<String, Vec<String>> = HashMap::new();
        
        // Initialize
        for node_name in self.nodes.keys() {
            in_degree.insert(node_name.clone(), 0);
            adj_list.insert(node_name.clone(), Vec::new());
        }
        
        // Build adjacency list and calculate in-degrees
        for edge in &self.edges {
            if let Some(neighbors) = adj_list.get_mut(&edge.from) {
                neighbors.push(edge.to.clone());
            }
            if let Some(degree) = in_degree.get_mut(&edge.to) {
                *degree += 1;
            }
        }
        
        // Kahn's algorithm
        let mut queue: Vec<String> = in_degree
            .iter()
            .filter(|(_, &degree)| degree == 0)
            .map(|(name, _)| name.clone())
            .collect();
        
        let mut result = Vec::new();
        
        while let Some(current) = queue.pop() {
            result.push(current.clone());
            
            if let Some(neighbors) = adj_list.get(&current) {
                for neighbor in neighbors {
                    if let Some(degree) = in_degree.get_mut(neighbor) {
                        *degree -= 1;
                        if *degree == 0 {
                            queue.push(neighbor.clone());
                        }
                    }
                }
            }
        }
        
        // Check for cycles
        if result.len() != self.nodes.len() {
            return Err(BuildError::CircularDependency {
                cycle: vec!["detected".to_string()], // TODO: extract actual cycle
            });
        }
        
        Ok(result)
    }

    pub fn detect_cycles(&self) -> Result<()> {
        let mut visited = HashSet::new();
        let mut rec_stack = HashSet::new();
        
        for node_name in self.nodes.keys() {
            if !visited.contains(node_name) {
                self.dfs_cycle_check(node_name, &mut visited, &mut rec_stack)?;
            }
        }
        
        Ok(())
    }

    fn dfs_cycle_check(
        &self,
        node: &str,
        visited: &mut HashSet<String>,
        rec_stack: &mut HashSet<String>,
    ) -> Result<()> {
        visited.insert(node.to_string());
        rec_stack.insert(node.to_string());
        
        // Find all edges from this node
        for edge in &self.edges {
            if edge.from == node {
                let neighbor = &edge.to;
                if !visited.contains(neighbor) {
                    self.dfs_cycle_check(neighbor, visited, rec_stack)?;
                } else if rec_stack.contains(neighbor) {
                    return Err(BuildError::CircularDependency {
                        cycle: vec![node.to_string(), neighbor.to_string()],
                    });
                }
            }
        }
        
        rec_stack.remove(node);
        Ok(())
    }
}

pub fn detect_package_type(package_path: &PathBuf) -> PackageType {
    let path_str = package_path.to_string_lossy();
    
    if path_str.contains("/stdlib/") || path_str.starts_with("stdlib/") {
        PackageType::Stdlib
    } else if path_str.contains("/internal/") || path_str.starts_with("internal/") {
        PackageType::Internal
    } else if path_str.contains("/.asthra/cache/") {
        PackageType::ThirdParty
    } else {
        PackageType::UserCode
    }
} 
