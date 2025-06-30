Feature: Package Declaration Syntax
  As an Asthra developer
  I want to declare packages for my code
  So that I can organize my codebase effectively

  Background:
    Given the Asthra compiler is available

  Scenario: Simple package declaration
    Given I have a file "simple_package.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          log("Package declaration works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Package declaration works"
    And the exit code should be 0

  Scenario: Package declaration with identifier
    Given I have a file "named_package.asthra" with:
      """
      package myapp;
      
      pub fn main(none) -> void {
          log("Named package works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Named package works"
    And the exit code should be 0

  Scenario: Package declaration with underscored identifier
    Given I have a file "underscore_package.asthra" with:
      """
      package my_app;
      
      pub fn main(none) -> void {
          log("Underscore package works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Underscore package works"
    And the exit code should be 0

  Scenario: Missing package declaration
    Given I have a file "no_package.asthra" with:
      """
      pub fn main(none) -> void {
          log("No package");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected 'package'"

  Scenario: Package declaration without semicolon
    Given I have a file "no_semicolon.asthra" with:
      """
      package main
      
      pub fn main(none) -> void {
          log("Missing semicolon");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected ';'"

  Scenario: Package declaration with invalid characters
    Given I have a file "invalid_package.asthra" with:
      """
      package my-app;
      
      pub fn main(none) -> void {
          log("Invalid package name");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "invalid package name"

  Scenario: Multiple package declarations
    Given I have a file "multiple_packages.asthra" with:
      """
      package first;
      package second;
      
      pub fn main(none) -> void {
          log("Multiple packages");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "multiple package declarations"

  Scenario: Package declaration not at beginning
    Given I have a file "package_after_code.asthra" with:
      """
      pub fn helper(none) -> void {
          return ();
      }
      
      package main;
      
      pub fn main(none) -> void {
          log("Package after code");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected 'package'"
