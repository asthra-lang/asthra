Feature: Mutability System
  As an Asthra developer
  I want variables to be immutable by default with explicit mutability
  So that I can write safer and more predictable code

  Background:
    Given I have a clean test environment
    And I am testing "mutability_system"

  # Immutability by default
  Scenario: Variables are immutable by default
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let x: i32 = 42;
          log("Immutable variable works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Immutable variable works"

  Scenario: Cannot reassign immutable variable
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let x: i32 = 42;
          x = 100;
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "immutable"

  # Explicit mutability with 'mut'
  Scenario: Mutable variable declaration
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let mut x: i32 = 42;
          x = 100;
          log("Mutable variable works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Mutable variable works"

  Scenario: Multiple reassignments to mutable variable
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let mut count: i32 = 0;
          count = count + 1;
          count = count + 1;
          count = count + 1;
          log("Multiple reassignments work");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Multiple reassignments work"

  # Function parameters are always immutable
  Scenario: Function parameters are immutable
    Given the following Asthra code:
      """
      package test;

      pub fn process(value: i32) -> void {
          value = 100;
          log("Should not compile");
          return ();
      }

      pub fn main(none) -> void {
          process(42);
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "parameter"

  Scenario: Cannot use mut on function parameters
    Given the following Asthra code:
      """
      package test;

      pub fn process(mut value: i32) -> void {
          log("Should not compile");
          return ();
      }

      pub fn main(none) -> void {
          process(42);
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "parameter"

  # Struct field mutability follows container
  @wip
  Scenario: Mutable struct allows field modification
    Given the following Asthra code:
      """
      package test;

      pub struct Point {
          pub x: i32,
          pub y: i32
      }

      pub fn main(none) -> void {
          let mut p: Point = Point { x: 10, y: 20 };
          p.x = 30;
          p.y = 40;
          log("Mutable struct fields work");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Mutable struct fields work"

  Scenario: Immutable struct prevents field modification
    Given the following Asthra code:
      """
      package test;

      pub struct Point {
          pub x: i32,
          pub y: i32
      }

      pub fn main(none) -> void {
          let p: Point = Point { x: 10, y: 20 };
          p.x = 30;
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "immutable"

  # Array element mutability
  @wip
  Scenario: Mutable array allows element modification
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let mut arr: [3]i32 = [1, 2, 3];
          arr[0] = 10;
          arr[1] = 20;
          arr[2] = 30;
          log("Mutable array elements work");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Mutable array elements work"

  Scenario: Immutable array prevents element modification
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let arr: [3]i32 = [1, 2, 3];
          arr[0] = 10;
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "immutable"

  # Nested mutability
  Scenario: Nested struct mutability
    Given the following Asthra code:
      """
      package test;

      pub struct Inner {
          pub value: i32
      }

      pub struct Outer {
          pub inner: Inner
      }

      pub fn main(none) -> void {
          let mut o: Outer = Outer { inner: Inner { value: 42 } };
          o.inner.value = 100;
          log("Nested mutability works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Nested mutability works"

  # Pointer mutability
  Scenario: Mutable pointer allows dereferenced assignment
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let mut x: i32 = 42;
          let ptr: *mut i32 = &x;
          unsafe {
              *ptr = 100;
          }
          log("Mutable pointer works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Mutable pointer works"

  Scenario: Const pointer prevents dereferenced assignment
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let x: i32 = 42;
          let ptr: *const i32 = &x;
          unsafe {
              *ptr = 100;
          }
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "const"

  # Variable shadowing
  @wip
  Scenario: Immutable variable can be shadowed
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let x: i32 = 42;
          let x: i32 = 100;
          log("Variable shadowing works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Variable shadowing works"

  @wip
  Scenario: Shadowing can change mutability
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let x: i32 = 42;
          let mut x: i32 = x;
          x = 100;
          log("Mutability change via shadowing works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Mutability change via shadowing works"

  # Complex assignment patterns
  Scenario: Compound assignment to mutable variable
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let mut sum: i32 = 0;
          sum = sum + 10;
          sum = sum * 2;
          sum = sum - 5;
          log("Compound assignments work");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Compound assignments work"

  # Method self parameter is always immutable
  Scenario: Method self parameter is immutable
    Given the following Asthra code:
      """
      package test;

      pub struct Counter {
          pub value: i32
      }

      impl Counter {
          pub fn try_modify(self) -> void {
              self.value = 100;
              return ();
          }
      }

      pub fn main(none) -> void {
          let c: Counter = Counter { value: 0 };
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "self"