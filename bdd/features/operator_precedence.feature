Feature: Operator Precedence
  As a developer using Asthra
  I want operators to follow well-defined precedence rules
  So that complex expressions evaluate correctly without excessive parentheses

  Background:
    Given a new compilation context

  # Logical operators (lowest precedence)
  Scenario: Logical OR has lowest precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let c: bool = true;
        let result: bool = a && b || c;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the expression should evaluate as "(a && b) || c"

  Scenario: Logical AND has higher precedence than OR
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let result: bool = false || true && false;
        if result {
          return 0;
        } else {
          return 1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the expression should evaluate as "false || (true && false)"

  # Bitwise operators
  Scenario: Bitwise OR precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0b1100;
        let b: i32 = 0b1010;
        let c: i32 = 0b0011;
        let result: i32 = a & b | c;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 11
    And the expression should evaluate as "(a & b) | c"

  Scenario: Bitwise XOR precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0b1111;
        let b: i32 = 0b1010;
        let c: i32 = 0b0101;
        let result: i32 = a & b ^ c;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 15
    And the expression should evaluate as "(a & b) ^ c"

  # Comparison operators
  Scenario: Equality operators precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 5;
        let b: i32 = 3;
        let c: i32 = 2;
        let result: bool = a > b == c < a;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the expression should evaluate as "(a > b) == (c < a)"

  @wip
  Scenario: Relational operators precedence over bitwise
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 8;
        let b: i32 = 4;
        let c: i32 = 2;
        let result: bool = a & b > c;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0
    And the expression should evaluate as "a & (b > c)"

  # Shift operators
  Scenario: Shift operators precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 16;
        let b: i32 = 2;
        let c: i32 = 1;
        let result: i32 = a >> b + c;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 2
    And the expression should evaluate as "a >> (b + c)"

  Scenario: Shift operators between relational and arithmetic
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 32;
        let b: i32 = 2;
        let c: i32 = 3;
        let result: bool = a >> b < c;
        if result {
          return 0;
        } else {
          return 1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the expression should evaluate as "(a >> b) < c"

  # Arithmetic operators
  Scenario: Multiplication has higher precedence than addition
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 2;
        let b: i32 = 3;
        let c: i32 = 4;
        let result: i32 = a + b * c;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 14
    And the expression should evaluate as "a + (b * c)"

  Scenario: Addition and subtraction are left-associative
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 10;
        let b: i32 = 5;
        let c: i32 = 3;
        let result: i32 = a - b + c;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 8
    And the expression should evaluate as "(a - b) + c"

  Scenario: Division and modulo same precedence as multiplication
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 20;
        let b: i32 = 4;
        let c: i32 = 3;
        let result: i32 = a / b % c;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 2
    And the expression should evaluate as "(a / b) % c"

  # Unary operators
  Scenario: Unary operators have highest precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 5;
        let b: i32 = 3;
        let result: i32 = -a + b;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be -2
    And the expression should evaluate as "(-a) + b"

  Scenario: Logical NOT precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let result: bool = !a && b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0
    And the expression should evaluate as "(!a) && b"

  Scenario: Bitwise NOT precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0b1010;
        let b: i32 = 0b1100;
        let result: i32 = ~a & b;
        return result & 0xFF;
      }
      """
    When I compile and run the code
    Then the return value should be 4
    And the expression should evaluate as "(~a) & b"

  # Complex expressions
  Scenario: Complex expression with multiple operators
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 10;
        let b: i32 = 5;
        let c: i32 = 2;
        let d: i32 = 3;
        let result: bool = a + b * c > d << 2 && a - b < c + d;
        if result {
          return 0;
        } else {
          return 1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the expression should evaluate as "((a + (b * c)) > (d << 2)) && ((a - b) < (c + d))"

  Scenario: Parentheses override precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 2;
        let b: i32 = 3;
        let c: i32 = 4;
        let result1: i32 = a + b * c;
        let result2: i32 = (a + b) * c;
        return result2 - result1;
      }
      """
    When I compile and run the code
    Then the return value should be 6

  # Postfix operators
  Scenario: Function call precedence
    Given the following Asthra code:
      """
      package test;
      pub fn square(x: i32) -> i32 {
        return x * x;
      }
      pub fn main() -> i32 {
        let a: i32 = 3;
        let result: i32 = square(a) + 1;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 10
    And the expression should evaluate as "(square(a)) + 1"

  @wip
  Scenario: Field access precedence
    Given the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main() -> i32 {
        let p: Point = Point { x: 5, y: 3 };
        let result: i32 = p.x + p.y * 2;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 11
    And the expression should evaluate as "(p.x) + ((p.y) * 2)"

  @wip
  Scenario: Array indexing precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let arr: [3]i32 = [10, 20, 30];
        let i: i32 = 1;
        let result: i32 = arr[i] + arr[i + 1];
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 50
    And the expression should evaluate as "(arr[i]) + (arr[i + 1])"

  # Mixed logical and bitwise
  @wip
  Scenario: Logical operators have lower precedence than bitwise
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0b1100;
        let b: i32 = 0b1010;
        let c: bool = true;
        let result: bool = a & b != 0 && c;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the expression should evaluate as "((a & b) != 0) && c"

  # Await operator precedence
  Scenario: Await operator precedence
    Given the following Asthra code:
      """
      package test;
      pub fn async_value() -> i32 {
        return 42;
      }
      pub fn main() -> i32 {
        let result: i32 = await async_value() + 8;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 50
    And the expression should evaluate as "(await async_value()) + 8"

  # Type cast and sizeof
  @wip
  Scenario: Sizeof operator precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let size: usize = sizeof(i32) * 2;
        return size as i32;
      }
      """
    When I compile and run the code
    Then the return value should be 8
    And the expression should evaluate as "(sizeof(i32)) * 2"

  # Comprehensive precedence ladder
  Scenario: Full precedence hierarchy validation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 100;
        let b: i32 = 10;
        let c: i32 = 5;
        let d: i32 = 2;
        let e: bool = true;
        let f: bool = false;
        
        let result: bool = a / b - c << d == 20 >> 1 && e || !f;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the expression should evaluate with proper precedence

  # Edge case: Multiple unary operators
  @wip
  Scenario: Chained unary operators are left-to-right
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 5;
        let result: i32 = -~a;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 6
    And the expression should evaluate as "-(~a)"

  # Associativity validation
  Scenario: Left-associative operators chain correctly
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 100;
        let b: i32 = 50;
        let c: i32 = 20;
        let d: i32 = 10;
        let result: i32 = a - b - c - d;
        return result;
      }
      """
    When I compile and run the code
    Then the return value should be 20
    And the expression should evaluate as "((a - b) - c) - d"