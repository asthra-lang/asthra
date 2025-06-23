Feature: Basic Compiler Functionality
  As a developer
  I want the Asthra compiler to correctly compile valid programs
  So that I can build reliable applications

  Background:
    Given the Asthra compiler is available

  Scenario: Compile a simple Hello World program
    Given I have a file "hello.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          println("Hello, World!");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created

  Scenario: Handle syntax errors gracefully
    Given I have a file "syntax_error.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          println("Missing semicolon")
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected ';'"

  Scenario: Compile and run Hello World program
    Given I have a file "hello_run.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          log("Hello, World!");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Hello, World!"
    And the exit code should be 0

  @wip
  Scenario: Optimize code with -O2 flag
    Given I have a valid Asthra source file
    When I compile with optimization level 2
    Then the output should be optimized
    And the binary size should be smaller than unoptimized