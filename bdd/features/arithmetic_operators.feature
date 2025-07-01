Feature: Arithmetic Operators
  As a developer using Asthra
  I want to use arithmetic operators (+, -, *, /, %)
  So that I can perform mathematical calculations correctly

  Background:
    Given a new compilation context

  # Addition operator tests
  Scenario: Basic integer addition
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 5;
        let b: i32 = 3;
        let result: i32 = a + b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 8

  Scenario: Addition with negative numbers
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = -10;
        let b: i32 = 7;
        let result: i32 = a + b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -3

  @wip
  Scenario: Floating point addition
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: f32 = 3.14;
        let b: f32 = 2.86;
        let result: f32 = a + b;
        if result > 5.99 && result < 6.01 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Addition with different integer types
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i16 = 100;
        let b: i16 = 50;
        let result: i16 = a + b;
        return result as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 150

  # Subtraction operator tests
  Scenario: Basic integer subtraction
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 10;
        let b: i32 = 3;
        let result: i32 = a - b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 7

  Scenario: Subtraction resulting in negative
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 5;
        let b: i32 = 12;
        let result: i32 = a - b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -7

  Scenario: Subtraction with unsigned integers
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: u32 = 100;
        let b: u32 = 25;
        let result: u32 = a - b;
        return result as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 75

  @wip
  Scenario: Floating point subtraction
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: f64 = 10.5;
        let b: f64 = 3.2;
        let result: f64 = a - b;
        if result > 7.29 && result < 7.31 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Multiplication operator tests
  Scenario: Basic integer multiplication
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 6;
        let b: i32 = 7;
        let result: i32 = a * b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Multiplication with zero
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 1000;
        let b: i32 = 0;
        let result: i32 = a * b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Multiplication with negative numbers
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = -4;
        let b: i32 = -5;
        let result: i32 = a * b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 20

  @wip
  Scenario: Floating point multiplication
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: f32 = 2.5;
        let b: f32 = 4.0;
        let result: f32 = a * b;
        if result > 9.99 && result < 10.01 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Division operator tests
  Scenario: Basic integer division
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 20;
        let b: i32 = 4;
        let result: i32 = a / b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 5

  Scenario: Integer division with truncation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 7;
        let b: i32 = 3;
        let result: i32 = a / b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 2

  Scenario: Division with negative numbers
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = -15;
        let b: i32 = 3;
        let result: i32 = a / b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -5

  @wip
  Scenario: Floating point division
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: f32 = 10.0;
        let b: f32 = 4.0;
        let result: f32 = a / b;
        if result > 2.49 && result < 2.51 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Division by zero error
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 10;
        let b: i32 = 0;
        let result: i32 = a / b;
        return result;
      }
      """
    When I compile and run the code
    Then the program should panic or return an error

  # Modulo operator tests
  Scenario: Basic modulo operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 17;
        let b: i32 = 5;
        let result: i32 = a % b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 2

  Scenario: Modulo with even division
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 20;
        let b: i32 = 5;
        let result: i32 = a % b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Modulo with negative dividend
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = -17;
        let b: i32 = 5;
        let result: i32 = a % b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -2

  Scenario: Modulo with unsigned integers
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: u32 = 23;
        let b: u32 = 7;
        let result: u32 = a % b;
        return result as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 2

  # Mixed operations tests
  Scenario: Multiple additions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 1;
        let b: i32 = 2;
        let c: i32 = 3;
        let d: i32 = 4;
        let result: i32 = a + b + c + d;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 10

  Scenario: Mixed arithmetic operations
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 10;
        let b: i32 = 5;
        let c: i32 = 2;
        let result: i32 = a + b * c - a / b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 18

  Scenario: Arithmetic with parentheses
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 10;
        let b: i32 = 5;
        let c: i32 = 2;
        let result: i32 = (a + b) * c;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 30

  # Arithmetic with variables and assignments
  Scenario: Arithmetic with mutable variables
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut sum: i32 = 0;
        sum = sum + 10;
        sum = sum + 20;
        sum = sum - 5;
        return sum;
      }
      """
    When I compile and run the code
    Then the exit code should be 25

  Scenario: Compound arithmetic expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 100;
        let b: i32 = 25;
        let c: i32 = 10;
        let result: i32 = (a - b) / (c - 5);
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 15

  # Type conversion in arithmetic
  Scenario: Arithmetic with type casting
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i16 = 100;
        let b: i32 = 200;
        let result: i32 = (a as i32) + b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 300

  # Arithmetic with constants
  Scenario: Arithmetic with const values
    Given the following Asthra code:
      """
      package test;
      const PI: f32 = 3.14159;
      const RADIUS: i32 = 10;
      
      pub fn main() -> i32 {
        let circumference: f32 = 2.0 * PI * (RADIUS as f32);
        if circumference > 62.8 && circumference < 62.9 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Edge cases
  Scenario: Large number arithmetic
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i64 = 1000000;
        let b: i64 = 2000000;
        let result: i64 = a * b;
        if result == 2000000000000 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Arithmetic with i8 boundary values
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i8 = 127;
        let b: i8 = -128;
        let result: i16 = (a as i16) - (b as i16);
        return result as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 255

  # Unary minus operator
  Scenario: Unary minus on positive number
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 42;
        let result: i32 = -a;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -42

  Scenario: Unary minus on negative number
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = -15;
        let result: i32 = -a;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 15

  Scenario: Multiple unary minus
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 10;
        let result: i32 = -(-a);
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 10

  # Complex arithmetic expressions
  Scenario: Nested arithmetic expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 50;
        let b: i32 = 10;
        let c: i32 = 5;
        let d: i32 = 2;
        let result: i32 = ((a - b) / c) * d + (b % c);
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 16

  Scenario: Arithmetic in function calls
    Given the following Asthra code:
      """
      package test;
      pub fn calculate(x: i32, y: i32) -> i32 {
        return x * 2 + y * 3;
      }
      
      pub fn main() -> i32 {
        let result: i32 = calculate(5 + 2, 10 - 3);
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 35