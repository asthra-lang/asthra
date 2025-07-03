@edge_cases @parser @negative
Feature: Type Annotation Syntax Validation
  As a language designer
  I want to ensure type annotations use correct Asthra syntax
  So that AI doesn't hallucinate syntax from other languages

  Background:
    Given the Asthra compiler is available

  # Negative test cases - Common mistakes from other languages

  Scenario: Reject Go-style type annotations without colon
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x i32 = 42;  // Error: Missing ':' before type
        return x;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '=' but found 'i32'"

  Scenario: Reject C-style type before variable name
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        i32 x = 42;  // Error: Type must come after variable name
        return x;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'let' or 'mut' but found 'i32'"

  Scenario: Reject var keyword instead of let
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        var x: i32 = 42;  // Error: Use 'let' not 'var'
        return x;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected statement but found 'var'"

  Scenario: Reject const for local variables
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        const x: i32 = 42;  // Error: 'const' is only for top-level
        return x;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "'const' declarations are only allowed at top level"

  Scenario: Reject auto/decltype keywords
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        auto x = 42;  // Error: No 'auto' keyword
        return x;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected statement but found 'auto'"

  Scenario: Reject question mark for nullable types
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32? = null;  // Error: Use Option<i32> not i32?
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '=' but found '?'"

  Scenario: Reject TypeScript union types
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 | string = 42;  // Error: No union types
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '=' but found '|'"

  Scenario: Reject pointer types with asterisk before type
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let p: *i32;  // Error: Asterisk goes after type: i32*
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected type but found '*'"

  Scenario: Reject reference types with ampersand
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 = 42;
        let r: &i32 = &x;  // Error: Use pointer types (i32*) not references
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected type but found '&'"

  Scenario: Reject missing type in mutable declaration
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        mut x = 42;  // Error: 'mut' must follow 'let'
        return x;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'let' before 'mut'"

  # Positive test cases - Correct syntax
  
  Scenario: Correct type annotation syntax
    Given I have the following Asthra code:
      """
      package test;
      pub const MAX: i32 = 100;
      pub fn main(none) -> i32 {
        let x: i32 = 42;
        let mut y: i32 = 0;
        let ptr: i32* = &x;
        let opt: Option<i32> = Option.Some(42);
        y = 42;
        return y;
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42