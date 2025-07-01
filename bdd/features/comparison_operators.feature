Feature: Comparison Operators
  As a developer using Asthra
  I want to use comparison operators (==, !=, <, <=, >, >=)
  So that I can compare values and make decisions in my code

  Background:
    Given a new compilation context

  # Equality operator (==) tests
  Scenario: Basic equality comparison with integers
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let b: i32 = 42;
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Equality comparison with different integer values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Equality comparison with boolean values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: bool = true;
        let b: bool = true;
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Equality comparison with strings
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: string = "hello";
        let b: string = "hello";
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Equality comparison with floats
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: f32 = 3.14;
        let b: f32 = 3.14;
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Inequality operator (!=) tests

  Scenario: Basic inequality comparison
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        if a != b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Inequality comparison with equal values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let b: i32 = 42;
        if a != b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Inequality comparison with booleans
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: bool = true;
        let b: bool = false;
        if a != b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Less than operator (<) tests

  Scenario: Basic less than comparison
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        if a < b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Less than comparison with equal values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let b: i32 = 42;
        if a < b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Less than comparison with greater value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 20;
        let b: i32 = 10;
        if a < b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Less than comparison with negative numbers
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = -20;
        let b: i32 = -10;
        if a < b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Less than or equal operator (<=) tests

  Scenario: Less than or equal with less value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        if a <= b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Less than or equal with equal values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let b: i32 = 42;
        if a <= b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Less than or equal with greater value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 20;
        let b: i32 = 10;
        if a <= b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  # Greater than operator (>) tests

  Scenario: Basic greater than comparison
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 20;
        let b: i32 = 10;
        if a > b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Greater than comparison with equal values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let b: i32 = 42;
        if a > b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Greater than comparison with smaller value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        if a > b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  # Greater than or equal operator (>=) tests

  Scenario: Greater than or equal with greater value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 20;
        let b: i32 = 10;
        if a >= b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Greater than or equal with equal values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 42;
        let b: i32 = 42;
        if a >= b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Greater than or equal with smaller value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        if a >= b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  # Chained comparisons

  Scenario: Multiple comparisons in one expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        let c: i32 = 30;
        if a < b && b < c {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Complex comparison expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 = 15;
        let y: i32 = 10;
        let z: i32 = 20;
        if x > y && x < z {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Different numeric types

  Scenario: Comparison with u8 types
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: u8 = 100;
        let b: u8 = 200;
        if a < b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Comparison with i64 types
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i64 = 1000000000000;
        let b: i64 = 2000000000000;
        if a < b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Comparison with f64 types
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: f64 = 3.14159265359;
        let b: f64 = 2.71828182846;
        if a > b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Edge cases

  Scenario: Comparison with zero
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 0;
        let b: i32 = 0;
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Comparison with maximum i32 value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 2147483647;
        let b: i32 = 2147483647;
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Comparison with minimum i32 value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = -2147483648;
        let b: i32 = -2147483648;
        if a == b {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Mixed operators

  Scenario: Equality and inequality in same expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 10;
        let c: i32 = 20;
        if a == b && b != c {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: All comparison operators in complex expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        let b: i32 = 10;
        let c: i32 = 10;
        let d: i32 = 15;
        if a < b && b <= c && c >= b && d > c {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Use in control flow

  Scenario: Comparison in nested if statements
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 = 25;
        if x > 20 {
          if x < 30 {
            return 1;
          }
        }
        return 0;
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Comparison in for loop
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut count: i32 = 0;
        for i in range(10) {
          if i >= 5 {
            count = count + 1;
          }
        }
        return count;
      }
      """
    When I compile and run the code
    Then the exit code should be 5

  # Assignment and comparison

  Scenario: Comparison result assignment
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 20;
        let result: bool = a < b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Multiple comparison results
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 = 15;
        let is_positive: bool = x > 0;
        let is_less_than_20: bool = x < 20;
        let in_range: bool = is_positive && is_less_than_20;
        if in_range {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Operator precedence

  Scenario: Comparison precedence with arithmetic
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        let b: i32 = 3;
        if a + b > 7 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Comparison precedence with parentheses
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let a: i32 = 10;
        let b: i32 = 5;
        let c: i32 = 3;
        if (a > b) == (b > c) {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Function calls in comparisons

  Scenario: Comparison with function results
    Given the following Asthra code:
      """
      package test;
      pub fn double(x: i32) -> i32 {
        return x * 2;
      }
      pub fn main(none) -> i32 {
        let a: i32 = 5;
        if double(a) == 10 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Floating point special cases

  Scenario: Floating point NaN comparison
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let nan: f32 = 0.0 / 0.0;
        if nan == nan {
          return 0;
        } else {
          return 1;  // NaN is not equal to itself
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Floating point infinity comparison
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let inf: f32 = 1.0 / 0.0;
        let large: f32 = 1000000.0;
        if inf > large {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1