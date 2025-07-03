@edge_cases @parser @negative
Feature: Function Syntax Validation
  As a language designer
  I want to ensure functions use correct Asthra syntax
  So that AI doesn't hallucinate syntax from other languages

  Background:
    Given the Asthra compiler is available

  # Negative test cases - Common mistakes from other languages

  Scenario: Reject Go-style function syntax without return type arrow
    Given I have the following Asthra code:
      """
      package test;
      pub fn add(a: i32, b: i32) i32 {  // Error: Missing '->' before return type
        return a + b;
      }
      pub fn main(none) -> i32 {
        return add(1, 2);
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '->' but found 'i32'"

  Scenario: Reject Rust-style function syntax with fn keyword in type
    Given I have the following Asthra code:
      """
      package test;
      pub fn apply(f: fn(i32) -> i32, x: i32) -> i32 {  // Error: 'fn' not allowed in type position
        return f(x);
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected type but found 'fn'"

  Scenario: Reject C-style function syntax with type before name
    Given I have the following Asthra code:
      """
      package test;
      pub i32 calculate(i32 x) {  // Error: Type before function name
        return x * 2;
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'fn' but found 'i32'"

  Scenario: Reject TypeScript-style optional parameters
    Given I have the following Asthra code:
      """
      package test;
      pub fn greet(name?: string) -> string {  // Error: No optional parameters
        return "Hello";
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected ':' but found '?'"

  Scenario: Reject JavaScript-style default parameters
    Given I have the following Asthra code:
      """
      package test;
      pub fn multiply(a: i32, b: i32 = 2) -> i32 {  // Error: No default values
        return a * b;
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected ')' but found '='"

  Scenario: Reject void as parameter type
    Given I have the following Asthra code:
      """
      package test;
      pub fn process(void) -> i32 {  // Error: Use 'none' not 'void'
        return 42;
      }
      pub fn main(none) -> i32 {
        return process();
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected parameter name but found ')'"

  Scenario: Reject empty parentheses for no parameters
    Given I have the following Asthra code:
      """
      package test;
      pub fn get_value() -> i32 {  // Error: Must use 'none' for no parameters
        return 42;
      }
      pub fn main(none) -> i32 {
        return get_value();
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected parameter or 'none'"

  Scenario: Reject mutable parameters
    Given I have the following Asthra code:
      """
      package test;
      pub fn increment(mut x: i32) -> i32 {  // Error: Parameters are always immutable
        x = x + 1;
        return x;
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "parameters cannot be declared mutable"

  # Positive test case - Correct syntax
  
  Scenario: Correct function syntax with various features
    Given I have the following Asthra code:
      """
      package test;
      pub fn add(a: i32, b: i32) -> i32 {
        return a + b;
      }
      pub fn no_params(none) -> i32 {
        return 42;
      }
      pub fn void_return(none) -> void {
        return ();
      }
      pub fn main(none) -> i32 {
        return add(20, 22);
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42