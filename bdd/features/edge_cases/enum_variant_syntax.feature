@edge_cases @parser @negative
Feature: Enum Variant Syntax Validation
  As a language designer
  I want to ensure enum variants use correct dot notation syntax
  So that the language maintains consistent grammar rules

  Background:
    Given the Asthra compiler is available

  # Positive test cases - correct dot notation

  Scenario: Correct enum variant construction with dot notation
    Given I have the following Asthra code:
      """
      package test;
      pub enum Status {
        Active,
        Inactive
      }
      pub fn main(none) -> i32 {
        let s: Status = Status.Active;  // Correct: dot notation
        match s {
          Status.Active => { return 42; }
          Status.Inactive => { return 0; }
        }
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42

  Scenario: Correct enum variant with data using dot notation
    Given I have the following Asthra code:
      """
      package test;
      pub enum Option<T> {
        Some(T),
        None
      }
      pub fn main(none) -> i32 {
        let opt: Option<i32> = Option.Some(42);  // Correct: dot notation
        match opt {
          Option.Some(value) => { return value; }
          Option.None => { return 0; }
        }
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42

  # Negative test cases - incorrect double colon syntax

  Scenario: Reject enum variant construction with double colon
    Given I have the following Asthra code:
      """
      package test;
      pub enum Status {
        Active,
        Inactive
      }
      pub fn main(none) -> i32 {
        let s: Status = Status::Active;  // Error: double colon not allowed
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "Invalid postfix '::' usage"

  Scenario: Reject enum variant with data using double colon
    Given I have the following Asthra code:
      """
      package test;
      pub enum Option<T> {
        Some(T),
        None
      }
      pub fn main(none) -> i32 {
        let opt: Option<i32> = Option::Some(42);  // Error: double colon not allowed
        return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "Invalid postfix '::' usage"

  Scenario: Reject enum pattern matching with double colon
    Given I have the following Asthra code:
      """
      package test;
      pub enum Result<T, E> {
        Ok(T),
        Err(E)
      }
      pub fn main(none) -> i32 {
        let r: Result<i32, string> = Result.Ok(42);
        match r {
          Result::Ok(value) => { return value; }  // Error: double colon in pattern
          Result::Err(_) => { return 0; }
        }
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '=>' but found ':'"

  Scenario: Reject if-let pattern with double colon
    Given I have the following Asthra code:
      """
      package test;
      pub enum Option<T> {
        Some(T),
        None
      }
      pub fn main(none) -> i32 {
        let opt: Option<i32> = Option.Some(42);
        if let Option::Some(value) = opt {  // Error: double colon in pattern
          return value;
        }
        return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '=' but found ':'"

  # Mixed scenarios to ensure correct behavior

  Scenario: Accept dot notation for enums while using double colon for types
    Given I have the following Asthra code:
      """
      package test;
      pub enum MyEnum {
        Value(i32)
      }
      pub struct MyStruct {
        none
      }
      impl MyStruct {
        pub fn new(none) -> MyStruct {
          return MyStruct { none };
        }
      }
      pub fn main(none) -> i32 {
        let s: MyStruct = MyStruct::new(none);  // OK: :: for associated functions
        let e: MyEnum = MyEnum.Value(42);       // OK: . for enum variants
        match e {
          MyEnum.Value(n) => { return n; }
        }
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42

  Scenario: Reject mixed syntax in same expression
    Given I have the following Asthra code:
      """
      package test;
      pub enum Color {
        RGB(i32, i32, i32),
        Named(string)
      }
      pub fn main(none) -> i32 {
        // Mixing correct and incorrect syntax
        let c1: Color = Color.RGB(255, 0, 0);     // Correct
        let c2: Color = Color::Named("red");      // Error
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "Invalid postfix '::' usage"

  # Edge cases with generic enums

  Scenario: Correct generic enum variant syntax
    Given I have the following Asthra code:
      """
      package test;
      pub enum Result<T, E> {
        Ok(T),
        Err(E)
      }
      pub fn main(none) -> i32 {
        let r1: Result<i32, string> = Result.Ok(42);           // Correct
        let r2: Result<i32, string> = Result<i32, string>.Ok(42);  // Also correct
        match r1 {
          Result.Ok(value) => { return value; }
          Result.Err(_) => { return 0; }
        }
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42

  Scenario: Reject generic enum with double colon
    Given I have the following Asthra code:
      """
      package test;
      pub enum Result<T, E> {
        Ok(T),
        Err(E)
      }
      pub fn main(none) -> i32 {
        let r: Result<i32, string> = Result<i32, string>::Ok(42);  // Error
        return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "Invalid postfix '::' usage"

  # Clear error messages

  Scenario: Provide helpful error message for common mistake
    Given I have the following Asthra code:
      """
      package test;
      pub enum Option<T> {
        Some(T),
        None
      }
      pub fn main(none) -> i32 {
        // Common mistake from Rust users
        let opt = Option::Some(42);
        return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "Use '::' only for type-level associated functions"
    And the error message should suggest using dot notation for enum variants