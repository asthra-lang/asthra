use crate::error::Result;
use crate::resolver::ImportType;
use regex::Regex;
use std::path::Path;
use tokio::fs;

pub struct ImportParser {
    stdlib_pattern: Regex,
    internal_pattern: Regex,
    third_party_pattern: Regex,
    import_statement_pattern: Regex,
}

impl ImportParser {
    pub fn new() -> Result<Self> {
        Ok(Self {
            stdlib_pattern: Regex::new(r"^stdlib/")?,
            internal_pattern: Regex::new(r"^internal/")?,
            third_party_pattern: Regex::new(r"^[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}/")?,
            // Match import statements like: import "path/to/module"
            import_statement_pattern: Regex::new(r#"import\s+"([^"]+)""#)?,
        })
    }

    pub async fn parse_imports_from_file(&self, file_path: &Path) -> Result<Vec<ImportType>> {
        let content = fs::read_to_string(file_path).await?;
        self.parse_imports_from_content(&content)
    }

    pub fn parse_imports_from_content(&self, content: &str) -> Result<Vec<ImportType>> {
        let mut imports = Vec::new();
        
        for captures in self.import_statement_pattern.captures_iter(content) {
            if let Some(import_path) = captures.get(1) {
                let import_type = self.classify_import(import_path.as_str());
                imports.push(import_type);
            }
        }
        
        Ok(imports)
    }

    pub fn classify_import(&self, import_path: &str) -> ImportType {
        if self.stdlib_pattern.is_match(import_path) {
            ImportType::Stdlib(import_path.to_string())
        } else if self.internal_pattern.is_match(import_path) {
            ImportType::Internal(import_path.to_string())
        } else if self.third_party_pattern.is_match(import_path) {
            ImportType::ThirdParty(import_path.to_string())
        } else if import_path.starts_with("./") || import_path.starts_with("../") {
            ImportType::Local(import_path.to_string())
        } else {
            // Default to local for relative paths without explicit prefix
            ImportType::Local(import_path.to_string())
        }
    }

    pub async fn find_all_imports_in_directory(&self, dir_path: &Path) -> Result<Vec<(std::path::PathBuf, Vec<ImportType>)>> {
        let mut all_imports = Vec::new();
        
        for entry in walkdir::WalkDir::new(dir_path) {
            let entry = entry?;
            let path = entry.path();
            
            if path.is_file() && path.extension().map_or(false, |ext| ext == "asthra") {
                let imports = self.parse_imports_from_file(path).await?;
                if !imports.is_empty() {
                    all_imports.push((path.to_path_buf(), imports));
                }
            }
        }
        
        Ok(all_imports)
    }

    pub fn extract_package_name_from_import(&self, import_path: &str) -> Option<String> {
        match self.classify_import(import_path) {
            ImportType::ThirdParty(path) => {
                // Extract package name from "github.com/user/repo/subpath"
                let parts: Vec<&str> = path.split('/').collect();
                if parts.len() >= 3 {
                    Some(format!("{}/{}/{}", parts[0], parts[1], parts[2]))
                } else {
                    Some(path)
                }
            }
            ImportType::Stdlib(path) => {
                // Extract stdlib module name from "stdlib/module/submodule"
                let parts: Vec<&str> = path.split('/').collect();
                if parts.len() >= 2 {
                    Some(format!("{}/{}", parts[0], parts[1]))
                } else {
                    Some(path)
                }
            }
            ImportType::Internal(path) => {
                // Extract internal module name
                let parts: Vec<&str> = path.split('/').collect();
                if parts.len() >= 2 {
                    Some(format!("{}/{}", parts[0], parts[1]))
                } else {
                    Some(path)
                }
            }
            ImportType::Local(_) => None, // Local imports don't have package names
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_import_classification() {
        let parser = ImportParser::new().unwrap();
        
        assert!(matches!(
            parser.classify_import("stdlib/string"),
            ImportType::Stdlib(_)
        ));
        
        assert!(matches!(
            parser.classify_import("internal/runtime/memory"),
            ImportType::Internal(_)
        ));
        
        assert!(matches!(
            parser.classify_import("github.com/user/repo"),
            ImportType::ThirdParty(_)
        ));
        
        assert!(matches!(
            parser.classify_import("./utils"),
            ImportType::Local(_)
        ));
        
        assert!(matches!(
            parser.classify_import("../common"),
            ImportType::Local(_)
        ));
    }

    #[test]
    fn test_import_parsing() {
        let parser = ImportParser::new().unwrap();
        let content = r#"
            import "stdlib/string"
            import "github.com/user/json"
            import "./utils"
            import "internal/runtime"
        "#;
        
        let imports = parser.parse_imports_from_content(content).unwrap();
        assert_eq!(imports.len(), 4);
        
        assert!(matches!(imports[0], ImportType::Stdlib(_)));
        assert!(matches!(imports[1], ImportType::ThirdParty(_)));
        assert!(matches!(imports[2], ImportType::Local(_)));
        assert!(matches!(imports[3], ImportType::Internal(_)));
    }

    #[test]
    fn test_package_name_extraction() {
        let parser = ImportParser::new().unwrap();
        
        assert_eq!(
            parser.extract_package_name_from_import("github.com/user/repo/submodule"),
            Some("github.com/user/repo".to_string())
        );
        
        assert_eq!(
            parser.extract_package_name_from_import("stdlib/string/utils"),
            Some("stdlib/string".to_string())
        );
        
        assert_eq!(
            parser.extract_package_name_from_import("./local"),
            None
        );
    }
} 
