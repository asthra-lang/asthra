@edge_cases @parser @negative
Feature: Method and Associated Function Syntax Validation
  As a language designer
  I want to ensure methods and associated functions use correct Asthra syntax
  So that AI doesn't hallucinate syntax from other languages

  Background:
    Given the Asthra compiler is available

  # Negative test cases - Common mistakes from other languages

  Scenario: Reject Rust-style impl with type parameters on impl
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point<T> {
        x: T,
        y: T
      }
      impl<T> Point<T> {  // Error: Type params go on struct, not impl
        pub fn new(x: T, y: T) -> Point<T> {
          return Point { x: x, y: y };
        }
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected struct name after 'impl'"

  Scenario: Reject missing visibility modifier on methods
    Given I have the following Asthra code:
      """
      package test;
      pub struct Counter {
        value: i32
      }
      impl Counter {
        fn increment(self) -> i32 {  // Error: Missing pub/priv
          return self.value + 1;
        }
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "methods must have explicit visibility"

  Scenario: Reject mutable self parameter
    Given I have the following Asthra code:
      """
      package test;
      pub struct Counter {
        value: i32
      }
      impl Counter {
        pub fn increment(mut self) -> void {  // Error: self is always immutable
          self.value = self.value + 1;
          return ();
        }
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "'self' parameter cannot be mutable"

  Scenario: Reject &self or &mut self syntax
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      impl Point {
        pub fn distance(&self) -> i32 {  // Error: No reference syntax for self
          return self.x + self.y;
        }
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'self' but found '&'"

  Scenario: Reject static keyword for associated functions
    Given I have the following Asthra code:
      """
      package test;
      pub struct Math {
        none
      }
      impl Math {
        pub static fn add(a: i32, b: i32) -> i32 {  // Error: No 'static' keyword
          return a + b;
        }
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'fn' but found 'static'"

  Scenario: Reject dot notation for associated function calls
    Given I have the following Asthra code:
      """
      package test;
      pub struct Calculator {
        none
      }
      impl Calculator {
        pub fn add(a: i32, b: i32) -> i32 {
          return a + b;
        }
      }
      pub fn main(none) -> i32 {
        return Calculator.add(20, 22);  // Error: Must use :: for associated functions
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "use '::' for associated function calls"

  Scenario: Reject this keyword instead of self
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      impl Point {
        pub fn get_x(this) -> i32 {  // Error: Use 'self' not 'this'
          return this.x;
        }
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected 'self' for instance method"

  Scenario: Reject methods outside impl blocks
    Given I have the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn Point.distance(self) -> i32 {  // Error: Methods must be in impl blocks
        return self.x + self.y;
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '(' but found '.'"

  Scenario: Reject C++ style method declaration with class::
    Given I have the following Asthra code:
      """
      package test;
      pub struct Vector {
        x: float,
        y: float
      }
      pub fn Vector::length(self) -> float {  // Error: Methods go in impl blocks
        return self.x + self.y;
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected '(' but found '::'"

  # Positive test cases - Correct syntax
  
  Scenario: Correct method and associated function syntax
    Given I have the following Asthra code:
      """
      package test;
      pub struct Calculator {
        value: i32
      }
      impl Calculator {
        pub fn new(v: i32) -> Calculator {
          return Calculator { value: v };
        }
        pub fn add(self, x: i32) -> i32 {
          return self.value + x;
        }
        priv fn internal(self) -> i32 {
          return self.value;
        }
      }
      pub fn main(none) -> i32 {
        let calc: Calculator = Calculator::new(40);
        return calc.add(2);
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 42