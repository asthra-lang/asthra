Feature: Import System
  As an Asthra developer
  I want to import modules and packages
  So that I can reuse code and organize my project

  Background:
    Given the Asthra compiler is available

  Scenario: Import standard library module
    Given I have a file "import_stdlib.asthra" with:
      """
      package main;
      import "stdlib/io";
      
      pub fn main(none) -> void {
          log("Import works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Import works"
    And the exit code should be 0

  Scenario: Import with alias
    Given I have a file "import_alias.asthra" with:
      """
      package main;
      import "stdlib/collections" as col;
      
      pub fn main(none) -> void {
          log("Import alias works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Import alias works"
    And the exit code should be 0

  Scenario: Multiple imports
    Given I have a file "multiple_imports.asthra" with:
      """
      package main;
      import "stdlib/io";
      import "stdlib/math";
      import "stdlib/strings";
      
      pub fn main(none) -> void {
          log("Multiple imports work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Multiple imports work"
    And the exit code should be 0

  Scenario: Import relative path
    Given I have a file "import_relative.asthra" with:
      """
      package main;
      import "./utils";
      
      pub fn main(none) -> void {
          log("Relative import works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Relative import works"
    And the exit code should be 0

  Scenario: Import GitHub package
    Given I have a file "import_github.asthra" with:
      """
      package main;
      import "github.com/user/package";
      
      pub fn main(none) -> void {
          log("GitHub import works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "GitHub import works"
    And the exit code should be 0

  @wip
  Scenario: Import without quotes
    Given I have a file "import_no_quotes.asthra" with:
      """
      package main;
      import stdlib/io;
      
      pub fn main(none) -> void {
          log("No quotes");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected string literal"

  Scenario: Import without semicolon
    Given I have a file "import_no_semicolon.asthra" with:
      """
      package main;
      import "stdlib/io"
      
      pub fn main(none) -> void {
          log("No semicolon");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected ';'"

  Scenario: Import before package declaration
    Given I have a file "import_before_package.asthra" with:
      """
      import "stdlib/io";
      package main;
      
      pub fn main(none) -> void {
          log("Import before package");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected 'package'"

  @wip
  Scenario: Import with invalid path
    Given I have a file "import_invalid_path.asthra" with:
      """
      package main;
      import "invalid://path";
      
      pub fn main(none) -> void {
          log("Invalid path");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "invalid import path"

  @wip
  Scenario: Duplicate imports
    Given I have a file "duplicate_imports.asthra" with:
      """
      package main;
      import "stdlib/io";
      import "stdlib/io";
      
      pub fn main(none) -> void {
          log("Duplicate imports");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "duplicate import"

  @wip
  Scenario: Import with conflicting aliases
    Given I have a file "conflicting_aliases.asthra" with:
      """
      package main;
      import "stdlib/io" as utils;
      import "stdlib/math" as utils;
      
      pub fn main(none) -> void {
          log("Conflicting aliases");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "alias 'utils' already defined"