Feature: Unary Operators
  As a developer using Asthra
  I want to use unary operators (!, -, ~, *, &)
  So that I can perform logical negation, arithmetic negation, bitwise complement, and pointer operations

  Background:
    Given a new compilation context

  # Logical NOT operator (!) tests
  @wip
  Scenario: Basic logical NOT with true value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: bool = true;
        if !a {
          return 0;
        } else {
          return 1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Basic logical NOT with false value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: bool = false;
        if !a {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Double logical NOT
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: bool = true;
        let b: bool = !a;
        let c: bool = !b;
        if c {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Logical NOT in complex expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: bool = true;
        let b: bool = false;
        if !a || !b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Arithmetic negation operator (-) tests
  @wip
  Scenario: Basic arithmetic negation with positive integer
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let b: i32 = -a;
        return b;
      }
      """
    When I compile and run the code
    Then the exit code should be -42

  @wip
  Scenario: Basic arithmetic negation with negative integer
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = -42;
        let b: i32 = -a;
        return b;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Arithmetic negation with zero
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 0;
        let b: i32 = -a;
        return b;
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  @wip
  Scenario: Arithmetic negation with different integer types
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i8 = 10;
        let b: i8 = -a;
        if b == -10 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Arithmetic negation with floating point
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: f32 = 3.14;
        let b: f32 = -a;
        if b < 0.0 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Bitwise complement operator (~) tests
  @wip
  Scenario: Basic bitwise complement
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 0;
        let b: i32 = ~a;
        if b == -1 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Bitwise complement of -1
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = -1;
        let b: i32 = ~a;
        return b;
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  @wip
  Scenario: Bitwise complement with u8
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: u8 = 0;
        let b: u8 = ~a;
        if b == 255 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Bitwise complement in expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        let b: i32 = ~a & 0xFF;
        if b == 250 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Pointer dereference operator (*) tests
  @wip
  Scenario: Basic pointer dereference
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let p: *const i32 = &a;
        unsafe {
          let b: i32 = *p;
          return b;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Mutable pointer dereference
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut a: i32 = 10;
        let p: *mut i32 = &a;
        unsafe {
          *p = 20;
        }
        return a;
      }
      """
    When I compile and run the code
    Then the exit code should be 20

  @wip
  Scenario: Pointer dereference in expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 5;
        let pa: *const i32 = &a;
        let pb: *const i32 = &b;
        unsafe {
          let sum: i32 = *pa + *pb;
          return sum;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 15

  # Address-of operator (&) tests
  @wip
  Scenario: Basic address-of operator
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let p: *const i32 = &a;
        unsafe {
          if *p == 42 {
            return 1;
          } else {
            return 0;
          }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Address-of mutable variable
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut a: i32 = 10;
        let p: *mut i32 = &a;
        unsafe {
          *p = 20;
          if a == 20 {
            return 1;
          } else {
            return 0;
          }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Address-of in function calls
    Given the following Asthra code:
      """
      package test;
      pub fn modify_value(p: *mut i32) -> void {
        unsafe {
          *p = *p + 10;
        }
      }
      pub fn main(none) -> i32 {
        let mut a: i32 = 5;
        modify_value(&a);
        return a;
      }
      """
    When I compile and run the code
    Then the exit code should be 15

  # Combined unary operators
  @wip
  Scenario: Combining arithmetic negation and logical NOT
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        let b: bool = !(-a < 0);
        if b {
          return 0;
        } else {
          return 1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Combining bitwise complement and arithmetic negation
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        let b: i32 = -~a;
        return b;
      }
      """
    When I compile and run the code
    Then the exit code should be 6

  @wip
  Scenario: Pointer operations with arithmetic
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let p: *const i32 = &a;
        unsafe {
          let b: i32 = -*p;
          return b;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be -10

  # Edge cases
  @wip
  Scenario: Negation of minimum i32 value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut a: i32 = -2147483647;
        a = a - 1;  // This gives us the minimum i32 value
        let b: i32 = -a;
        if b == a {  // Overflow: negating minimum gives minimum
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Multiple dereferences
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let p1: *const i32 = &a;
        let p2: *const *const i32 = &p1;
        unsafe {
          let b: i32 = **p2;
          return b;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Operator precedence tests
  @wip
  Scenario: Unary operator precedence with multiplication
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        let b: i32 = -a * 2;  // Should be (-5) * 2 = -10
        return b;
      }
      """
    When I compile and run the code
    Then the exit code should be -10

  @wip
  Scenario: Unary operator precedence with addition
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        let b: i32 = -a + 3;  // Should be (-5) + 3 = -2
        if b == -2 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Type-specific tests
  @wip
  Scenario: Logical NOT with comparison result
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        let result: bool = !(a > b);
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Bitwise complement with different widths
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: u16 = 0;
        let b: u16 = ~a;
        if b == 65535 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Error prevention tests (grammar restrictions)
  @wip
  Scenario: Single unary operator per expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: bool = true;
        let b: bool = !a;  // Grammar prevents !!a
        let c: bool = !b;  // Must use separate statement
        if c {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Pointer operations require unsafe blocks
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 100;
        let p: *const i32 = &a;
        let result: i32 = 0;
        unsafe {
          result = *p;
        }
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 100