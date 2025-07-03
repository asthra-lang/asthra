@edge_cases @parser @negative
Feature: Struct Syntax Validation
  As a language designer
  I want to ensure structs use correct Asthra syntax
  So that AI doesn't hallucinate syntax from other languages

  Background:
    Given the Asthra compiler is available

  # Negative test cases - Common mistakes from other languages

  Scenario: Reject Go-style struct without braces
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point  // Error: Missing braces
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '{' but found 'pub'"

  Scenario: Reject C-style struct with semicolon after fields
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32;  // Error: No semicolons after fields
        y: i32;
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected ',' or '}' but found ';'"

  Scenario: Reject TypeScript-style interface keyword
    Given I have the following Asthra code:
      """
      package test;
      pub interface Shape {  // Error: No 'interface' keyword
        area: i32
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected declaration but found 'interface'"

  Scenario: Reject Rust-style pub keyword on struct fields
    Given I have the following Asthra code:
      """
      package test;
      pub struct User {
        pub name: string,  // Error: pub/priv go before field name
        age: i32
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "visibility modifier must come before field name"

  Scenario: Reject mutable struct fields
    Given I have the following Asthra code:
      """
      package test;
      pub struct Counter {
        mut count: i32  // Error: Struct fields cannot have mut modifier
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "struct fields cannot be declared mutable"

  Scenario: Reject struct initialization with equals sign
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let p: Point = Point { x = 10, y = 20 };  // Error: Use ':' not '='
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected ':' but found '='"

  Scenario: Reject struct initialization without type name
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let p: Point = { x: 10, y: 20 };  // Error: Must include struct name
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "struct literal must include type name"

  Scenario: Reject C++-style struct ending semicolon
    Given I have the following Asthra code:
      """
      package test;
      pub struct Vector {
        x: float,
        y: float
      };  // Error: No semicolon after struct definition
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "unexpected ';' after struct definition"

  Scenario: Reject missing none keyword for empty struct
    Given I have the following Asthra code:
      """
      package test;
      pub struct Empty {}  // Error: Must use 'none' for empty structs
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "empty struct must contain 'none'"

  # Positive test cases - Correct syntax
  
  Scenario: Correct struct syntax with various features
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub struct Empty {
        none
      }
      pub struct Mixed {
        pub value: i32,
        priv internal: string
      }
      pub fn main(none) -> i32 {
        let p: Point = Point { x: 42, y: 0 };
        return p.x;
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42