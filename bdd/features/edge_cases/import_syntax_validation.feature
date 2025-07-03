@edge_cases @parser @negative
Feature: Import and Module Syntax Validation
  As a language designer
  I want to ensure imports use correct Asthra syntax
  So that AI doesn't hallucinate syntax from other languages

  Background:
    Given the Asthra compiler is available

  # Negative test cases - Common mistakes from other languages

  Scenario: Reject Python-style import syntax
    Given I have the following Asthra code:
      """
      package test;
      from stdlib.io import println;  // Error: No 'from...import' syntax
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected declaration but found 'from'"

  Scenario: Reject JavaScript-style import with braces
    Given I have the following Asthra code:
      """
      package test;
      import { HashMap } from "stdlib/collections";  // Error: No destructuring imports
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected string literal but found '{'"

  Scenario: Reject Rust-style use statements
    Given I have the following Asthra code:
      """
      package test;
      use std::collections::HashMap;  // Error: Use 'import' not 'use'
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected declaration but found 'use'"

  Scenario: Reject C++ include syntax
    Given I have the following Asthra code:
      """
      package test;
      #include <iostream>  // Error: No preprocessor directives
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "unexpected '#'"

  Scenario: Reject Java-style import with dots
    Given I have the following Asthra code:
      """
      package test;
      import stdlib.io.File;  // Error: Must use string literal
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected string literal"

  Scenario: Reject import without quotes
    Given I have the following Asthra code:
      """
      package test;
      import stdlib/io;  // Error: Path must be quoted
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected string literal"

  Scenario: Reject wildcard imports
    Given I have the following Asthra code:
      """
      package test;
      import "stdlib/io/*";  // Error: No wildcard imports
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "wildcard imports are not allowed"

  Scenario: Reject import after other declarations
    Given I have the following Asthra code:
      """
      package test;
      pub const X: i32 = 42;
      import "stdlib/io";  // Error: Imports must come before declarations
      pub fn main(none) -> i32 {
        return X;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "imports must appear before other declarations"

  Scenario: Reject module declaration syntax
    Given I have the following Asthra code:
      """
      module test;  // Error: Use 'package' not 'module'
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'package' keyword"

  Scenario: Reject namespace syntax
    Given I have the following Asthra code:
      """
      namespace test {  // Error: Use 'package' declaration
        pub fn main(none) -> i32 {
          return 42;
        }
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'package' keyword"

  # Positive test cases - Correct syntax
  
  Scenario: Correct import and package syntax
    Given I have the following Asthra code:
      """
      package test;
      import "stdlib/io";
      import "stdlib/collections" as coll;
      import "./local_module";
      import "github.com/user/repo";
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I compile the program
    Then the compilation should succeed