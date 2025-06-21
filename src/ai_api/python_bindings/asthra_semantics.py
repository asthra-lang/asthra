"""
Python bindings for Asthra Semantic API
Provides Pythonic interface for AI tools to query Asthra semantic information
"""

import ctypes
from ctypes import c_char_p, c_void_p, c_size_t, c_bool, POINTER, Structure
from typing import List, Optional, Dict, Any
from dataclasses import dataclass
from pathlib import Path
import os

# Try to find the Asthra semantic library
def _find_library():
    """Find the Asthra semantic library in various locations."""
    possible_paths = [
        "libasthra_semantic.so",
        "./libasthra_semantic.so", 
        "../../../build/libasthra_semantic.so",
        "build/libasthra_semantic.so"
    ]
    
    for path in possible_paths:
        if os.path.exists(path):
            try:
                return ctypes.CDLL(path)
            except OSError:
                continue
    
    # If no shared library found, we'll create a mock implementation
    return None

# Load the Asthra semantic library (or create mock)
_lib = _find_library()

@dataclass
class FieldInfo:
    """Information about a struct field."""
    name: str
    type_name: str
    is_public: bool
    is_mutable: bool

@dataclass
class StructInfo:
    """Information about a struct type."""
    name: str
    fields: List[FieldInfo]
    methods: List[str]
    is_generic: bool
    type_parameters: List[str]

@dataclass
class SymbolInfo:
    """Information about a symbol."""
    name: str
    type_name: str
    kind: str  # "variable", "function", "struct", "enum"
    is_public: bool
    is_mutable: bool
    documentation: str

@dataclass
class TypeInfo:
    """Information about a type."""
    type_name: str
    category: str  # "primitive", "struct", "enum", "slice", "pointer", "function"
    is_mutable: bool
    is_generic: bool
    generic_params: List[str]
    size_bytes: int
    alignment: int
    is_ffi_compatible: bool
    is_copy_type: bool

class AsthraSemanticsAPI:
    """
    Python interface to Asthra's semantic analysis engine.
    Enables AI tools to query symbol tables, type information, and AST structure.
    """
    
    def __init__(self, source_file: Optional[Path] = None):
        """Initialize semantic analysis for the given source file."""
        self._api = None
        self._mock_mode = _lib is None
        
        if not self._mock_mode and source_file:
            # Real implementation with C library
            self._setup_ffi()
            self._api = _lib.asthra_ai_create_api(str(source_file).encode('utf-8'))
            if not self._api:
                raise RuntimeError(f"Failed to initialize semantic API for {source_file}")
        else:
            # Mock implementation for testing
            self._setup_mock_data()
    
    def _setup_ffi(self):
        """Setup FFI function signatures."""
        if not _lib:
            return
            
        # Define C function signatures
        _lib.asthra_ai_create_api.argtypes = [c_char_p]
        _lib.asthra_ai_create_api.restype = c_void_p

        _lib.asthra_ai_destroy_api.argtypes = [c_void_p]
        _lib.asthra_ai_destroy_api.restype = None

        _lib.asthra_ai_get_struct_info.argtypes = [c_void_p, c_char_p]
        _lib.asthra_ai_get_struct_info.restype = c_void_p

        _lib.asthra_ai_infer_expression_type.argtypes = [c_void_p, c_char_p]
        _lib.asthra_ai_infer_expression_type.restype = c_char_p
    
    def _setup_mock_data(self):
        """Setup mock data for testing when C library is not available."""
        self._mock_structs = {
            "User": StructInfo(
                name="User",
                fields=[
                    FieldInfo("name", "string", True, False),
                    FieldInfo("email", "string", False, False),
                    FieldInfo("age", "i32", True, True)
                ],
                methods=["new", "validate_email", "get_display_name"],
                is_generic=False,
                type_parameters=[]
            ),
            "Point": StructInfo(
                name="Point",
                fields=[
                    FieldInfo("x", "f64", True, True),
                    FieldInfo("y", "f64", True, True)
                ],
                methods=["new", "distance", "move_by"],
                is_generic=False,
                type_parameters=[]
            ),
            "Vec": StructInfo(
                name="Vec",
                fields=[
                    FieldInfo("data", "T*", False, True),
                    FieldInfo("length", "usize", True, False),
                    FieldInfo("capacity", "usize", False, False)
                ],
                methods=["new", "push", "pop", "get", "len"],
                is_generic=True,
                type_parameters=["T"]
            )
        }
        
        self._mock_types = {
            "i32": TypeInfo("i32", "primitive", False, False, [], 4, 4, True, True),
            "f64": TypeInfo("f64", "primitive", False, False, [], 8, 8, True, True),
            "string": TypeInfo("string", "struct", False, False, [], 24, 8, True, False),
            "bool": TypeInfo("bool", "primitive", False, False, [], 1, 1, True, True),
            "usize": TypeInfo("usize", "primitive", False, False, [], 8, 8, True, True)
        }
    
    def __del__(self):
        """Clean up resources."""
        if hasattr(self, '_api') and self._api and not self._mock_mode:
            _lib.asthra_ai_destroy_api(self._api)
    
    def get_struct_info(self, struct_name: str) -> Optional[StructInfo]:
        """
        Get comprehensive information about a struct type.
        
        Args:
            struct_name: Name of the struct to query
            
        Returns:
            StructInfo object with fields, methods, and generic information,
            or None if struct not found
        """
        if self._mock_mode:
            return self._mock_structs.get(struct_name)
        
        if not self._api:
            return None
            
        info_ptr = _lib.asthra_ai_get_struct_info(self._api, struct_name.encode('utf-8'))
        if not info_ptr:
            return None
        
        # Convert C struct to Python object
        return self._convert_struct_info(info_ptr)
    
    def infer_expression_type(self, expression: str) -> Optional[str]:
        """
        Infer the type of an Asthra expression.
        
        Args:
            expression: Asthra expression code
            
        Returns:
            Type name as string, or None if inference failed
        """
        if self._mock_mode:
            # Mock type inference
            if expression == "42":
                return "i32"
            elif expression == "3.14":
                return "f64"
            elif expression == '"hello"':
                return "string"
            elif expression == "true" or expression == "false":
                return "bool"
            elif expression.startswith("user."):
                field = expression.split(".", 1)[1]
                user_struct = self._mock_structs.get("User")
                if user_struct:
                    for field_info in user_struct.fields:
                        if field_info.name == field:
                            return field_info.type_name
            return "unknown"
        
        if not self._api:
            return None
            
        result = _lib.asthra_ai_infer_expression_type(self._api, expression.encode('utf-8'))
        if result:
            return result.decode('utf-8')
        return None
    
    def get_available_methods(self, type_name: str) -> List[str]:
        """
        Get all methods available on a given type.
        
        Args:
            type_name: Name of the type to query
            
        Returns:
            List of method names
        """
        if self._mock_mode:
            struct_info = self._mock_structs.get(type_name)
            return struct_info.methods if struct_info else []
        
        # Implementation using C API would go here
        return []
    
    def check_type_compatibility(self, expected: str, actual: str) -> bool:
        """
        Check if two types are compatible.
        
        Args:
            expected: Expected type name
            actual: Actual type name
            
        Returns:
            True if types are compatible
        """
        if self._mock_mode:
            # Mock compatibility rules
            if expected == actual:
                return True
            # Integer promotion rules
            if expected == "i64" and actual in ["i32", "i16", "i8"]:
                return True
            if expected == "i32" and actual in ["i16", "i8"]:
                return True
            # Float promotion rules
            if expected == "f64" and actual == "f32":
                return True
            return False
        
        # Implementation using C API would go here
        return False
    
    def get_type_info(self, type_name: str) -> Optional[TypeInfo]:
        """
        Get detailed information about a type.
        
        Args:
            type_name: Name of the type to query
            
        Returns:
            TypeInfo object with detailed type information
        """
        if self._mock_mode:
            return self._mock_types.get(type_name)
        
        # Implementation using C API would go here
        return None
    
    def validate_code_snippet(self, code: str) -> Dict[str, Any]:
        """
        Validate an Asthra code snippet for correctness.
        
        Args:
            code: Asthra code to validate
            
        Returns:
            Validation result with errors, warnings, and suggestions
        """
        if self._mock_mode:
            # Mock validation
            errors = []
            warnings = []
            suggestions = []
            
            # Simple validation rules
            if "let" in code and ":" not in code:
                errors.append("Missing type annotation")
                suggestions.append("Add explicit type annotation: let var: type = value;")
            
            if code.count("{") != code.count("}"):
                errors.append("Mismatched braces")
            
            return {
                "valid": len(errors) == 0,
                "errors": errors,
                "warnings": warnings,
                "suggestions": suggestions
            }
        
        # Implementation using C API would go here
        return {"valid": True, "errors": [], "warnings": [], "suggestions": []}
    
    def _convert_struct_info(self, info_ptr) -> StructInfo:
        """Convert C AIStructInfo to Python StructInfo."""
        # Implementation details for C to Python conversion
        # This would parse the C structure and convert to Python objects
        pass

# Example usage for AI tools
def example_ai_integration():
    """Example of how AI tools would use the semantic API."""
    
    # Initialize semantic API (mock mode if no C library)
    api = AsthraSemanticsAPI()
    
    # Query struct information
    user_struct = api.get_struct_info("User")
    if user_struct:
        print(f"User struct has {len(user_struct.fields)} fields:")
        for field in user_struct.fields:
            visibility = "public" if field.is_public else "private"
            mutability = "mutable" if field.is_mutable else "immutable"
            print(f"  {field.name}: {field.type_name} ({visibility}, {mutability})")
        
        print(f"Available methods: {', '.join(user_struct.methods)}")
    
    # Infer expression types
    expressions = ["42", "3.14", '"hello"', "true", "user.name", "user.age"]
    for expr in expressions:
        expr_type = api.infer_expression_type(expr)
        print(f"Expression '{expr}' has type: {expr_type}")
    
    # Check type compatibility
    compatibility_tests = [
        ("i32", "i16"),
        ("i64", "i32"),
        ("f64", "f32"),
        ("string", "i32")
    ]
    
    for expected, actual in compatibility_tests:
        compatible = api.check_type_compatibility(expected, actual)
        print(f"Types {actual} -> {expected}: {'compatible' if compatible else 'incompatible'}")
    
    # Validate code snippet
    code_snippets = [
        "let x = 42;",
        "let x: i32 = 42;",
        "let y = { 1, 2, 3 };",
        "struct Point { x: f64, y: f64 }"
    ]
    
    for code in code_snippets:
        result = api.validate_code_snippet(code)
        status = "✓" if result["valid"] else "✗"
        print(f"{status} '{code}': {result}")

if __name__ == "__main__":
    example_ai_integration() 
