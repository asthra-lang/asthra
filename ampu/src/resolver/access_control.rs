use crate::config::PackageType;
use crate::error::{AccessViolation, BuildError, ImportError, Result};
use crate::resolver::ImportType;
use std::path::PathBuf;

pub struct AccessController;

impl AccessController {
    pub fn validate_import(
        importing_package: PackageType,
        imported_package: &ImportType,
    ) -> std::result::Result<(), AccessViolation> {
        match (importing_package, imported_package) {
            // ✅ Allowed combinations
            (PackageType::UserCode, ImportType::Stdlib(_)) => Ok(()),
            (PackageType::UserCode, ImportType::ThirdParty(_)) => Ok(()),
            (PackageType::UserCode, ImportType::Local(_)) => Ok(()),
            (PackageType::Stdlib, ImportType::Stdlib(_)) => Ok(()),
            (PackageType::Stdlib, ImportType::Internal(_)) => Ok(()),
            (PackageType::Stdlib, ImportType::ThirdParty(_)) => Ok(()),
            (PackageType::Stdlib, ImportType::Local(_)) => Ok(()),
            (PackageType::Internal, ImportType::Stdlib(_)) => Ok(()),
            (PackageType::Internal, ImportType::Internal(_)) => Ok(()),
            (PackageType::Internal, ImportType::ThirdParty(_)) => Ok(()),
            (PackageType::Internal, ImportType::Local(_)) => Ok(()),
            (PackageType::ThirdParty, ImportType::Stdlib(_)) => Ok(()),
            (PackageType::ThirdParty, ImportType::ThirdParty(_)) => Ok(()),
            (PackageType::ThirdParty, ImportType::Local(_)) => Ok(()),

            // ❌ Forbidden combinations
            (PackageType::UserCode, ImportType::Internal(package)) => {
                Err(AccessViolation::UserToInternal { 
                    package: package.clone() 
                })
            }
            (PackageType::ThirdParty, ImportType::Internal(package)) => {
                Err(AccessViolation::ThirdPartyToInternal { 
                    package: package.clone() 
                })
            }
        }
    }

    pub fn validate_all_imports(
        &self,
        package_imports: &[(PackageType, Vec<ImportType>)],
    ) -> std::result::Result<(), Vec<AccessViolation>> {
        let mut violations = Vec::new();

        for (package_type, imports) in package_imports {
            for import in imports {
                if let Err(violation) = Self::validate_import(*package_type, import) {
                    violations.push(violation);
                }
            }
        }

        if violations.is_empty() {
            Ok(())
        } else {
            Err(violations)
        }
    }

    pub fn validate_imports_with_locations(
        &self,
        file_imports: &[(PathBuf, PackageType, Vec<ImportType>)],
    ) -> std::result::Result<(), Vec<ImportError>> {
        let mut errors = Vec::new();

        for (file_path, package_type, imports) in file_imports {
            for (line_num, import) in imports.iter().enumerate() {
                if let Err(violation) = Self::validate_import(*package_type, import) {
                    errors.push(ImportError {
                        file: file_path.clone(),
                        line: line_num + 1, // 1-indexed
                        column: 1, // Could be enhanced to find actual column
                        violation,
                    });
                }
            }
        }

        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }

    pub fn get_allowed_imports_for_package_type(package_type: PackageType) -> Vec<&'static str> {
        match package_type {
            PackageType::UserCode => vec!["stdlib/*", "third-party/*", "local/*"],
            PackageType::Stdlib => vec!["stdlib/*", "internal/*"],
            PackageType::Internal => vec!["stdlib/*", "internal/*"],
            PackageType::ThirdParty => vec!["stdlib/*", "third-party/*", "local/*"],
        }
    }

    pub fn is_import_allowed(package_type: PackageType, import: &ImportType) -> bool {
        Self::validate_import(package_type, import).is_ok()
    }
}

pub struct BuildValidator {
    access_controller: AccessController,
    import_parser: crate::resolver::import_parser::ImportParser,
}

impl BuildValidator {
    pub fn new() -> Result<Self> {
        Ok(Self {
            access_controller: AccessController,
            import_parser: crate::resolver::import_parser::ImportParser::new()?,
        })
    }

    pub async fn validate_project(&self, project_root: &std::path::Path) -> Result<()> {
        let file_imports = self.import_parser.find_all_imports_in_directory(project_root).await?;
        let mut validation_data = Vec::new();

        for (file_path, imports) in file_imports {
            let package_type = crate::resolver::detect_package_type(&file_path);
            validation_data.push((file_path, package_type, imports));
        }

        match self.access_controller.validate_imports_with_locations(&validation_data) {
            Ok(()) => Ok(()),
            Err(errors) => {
                let error_messages: Vec<String> = errors.iter().map(|e| e.to_string()).collect();
                Err(BuildError::AccessViolation {
                    message: format!("Import access violations found:\n{}", error_messages.join("\n")),
                })
            }
        }
    }

    pub async fn validate_single_file(&self, file_path: &std::path::Path) -> Result<()> {
        let imports = self.import_parser.parse_imports_from_file(file_path).await?;
        let package_type = crate::resolver::detect_package_type(&file_path.to_path_buf());

        let validation_data = vec![(file_path.to_path_buf(), package_type, imports)];
        
        match self.access_controller.validate_imports_with_locations(&validation_data) {
            Ok(()) => Ok(()),
            Err(errors) => {
                let error_messages: Vec<String> = errors.iter().map(|e| e.to_string()).collect();
                Err(BuildError::AccessViolation {
                    message: format!("Import access violations in {}:\n{}", 
                        file_path.display(), 
                        error_messages.join("\n")
                    ),
                })
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_access_control_rules() {
        // User code can import stdlib and third-party
        assert!(AccessController::validate_import(
            PackageType::UserCode,
            &ImportType::Stdlib("stdlib/string".to_string())
        ).is_ok());

        assert!(AccessController::validate_import(
            PackageType::UserCode,
            &ImportType::ThirdParty("github.com/user/repo".to_string())
        ).is_ok());

        // User code cannot import internal
        assert!(AccessController::validate_import(
            PackageType::UserCode,
            &ImportType::Internal("internal/runtime".to_string())
        ).is_err());

        // Third-party cannot import internal
        assert!(AccessController::validate_import(
            PackageType::ThirdParty,
            &ImportType::Internal("internal/runtime".to_string())
        ).is_err());

        // Stdlib can import internal
        assert!(AccessController::validate_import(
            PackageType::Stdlib,
            &ImportType::Internal("internal/runtime".to_string())
        ).is_ok());

        // Internal can import internal
        assert!(AccessController::validate_import(
            PackageType::Internal,
            &ImportType::Internal("internal/other".to_string())
        ).is_ok());
    }

    #[test]
    fn test_allowed_imports_listing() {
        let user_allowed = AccessController::get_allowed_imports_for_package_type(PackageType::UserCode);
        assert!(user_allowed.contains(&"stdlib/*"));
        assert!(user_allowed.contains(&"third-party/*"));
        assert!(!user_allowed.iter().any(|&s| s.contains("internal")));

        let stdlib_allowed = AccessController::get_allowed_imports_for_package_type(PackageType::Stdlib);
        assert!(stdlib_allowed.contains(&"stdlib/*"));
        assert!(stdlib_allowed.contains(&"internal/*"));
    }

    #[test]
    fn test_import_validation_batch() {
        let controller = AccessController;
        
        let package_imports = vec![
            (PackageType::UserCode, vec![
                ImportType::Stdlib("stdlib/string".to_string()),
                ImportType::Internal("internal/runtime".to_string()), // Should fail
            ]),
            (PackageType::Stdlib, vec![
                ImportType::Internal("internal/memory".to_string()), // Should pass
            ]),
        ];

        let result = controller.validate_all_imports(&package_imports);
        assert!(result.is_err());
        
        let violations = result.unwrap_err();
        assert_eq!(violations.len(), 1);
        assert!(matches!(violations[0], AccessViolation::UserToInternal { .. }));
    }
} 
