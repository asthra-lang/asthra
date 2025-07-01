Feature: Numeric Literals
  As a developer using Asthra
  I want to use various numeric literal formats
  So that I can express numbers in the most appropriate base and format

  Background:
    Given a new compilation context

  # Decimal integer literals
  @wip
  Scenario: Simple decimal integer
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Zero decimal integer
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0;
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  @wip
  Scenario: Large decimal integer
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 = 123456;
        return x % 100;
      }
      """
    When I compile and run the code
    Then the exit code should be 56

  # Hexadecimal literals
  @wip
  Scenario: Lowercase hex literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0x2a;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Uppercase hex literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0X2A;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Mixed case hex literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0xFf;
      }
      """
    When I compile and run the code
    Then the exit code should be 255

  @wip
  Scenario: Multi-digit hex literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 = 0xDEADBEEF;
        return x & 0xFF;
      }
      """
    When I compile and run the code
    Then the exit code should be 239

  # Binary literals
  @wip
  Scenario: Lowercase binary literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0b101010;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Uppercase binary literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0B101010;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Single bit binary literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0b1;
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Multi-bit binary literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0b11111111;
      }
      """
    When I compile and run the code
    Then the exit code should be 255

  # Octal literals
  @wip
  Scenario: Simple octal literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0o52;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Single digit octal literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0o7;
      }
      """
    When I compile and run the code
    Then the exit code should be 7

  @wip
  Scenario: Multi-digit octal literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return 0o377;
      }
      """
    When I compile and run the code
    Then the exit code should be 255

  # Float literals
  @wip
  Scenario: Simple float literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let f: f32 = 42.0;
        return f as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Float with fractional part
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let f: f32 = 42.5;
        return f as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Float without trailing digits
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let f: f32 = 42.;
        return f as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Float starting with decimal point
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let f: f32 = .42;
        let result: i32 = (f * 100.0) as i32;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mixed numeric operations
  @wip
  Scenario: Arithmetic with different literal formats
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let dec: i32 = 10;
        let hex: i32 = 0x10;
        let bin: i32 = 0b10000;
        let oct: i32 = 0o6;
        return dec + hex + bin + oct;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Type inference with literals
  @wip
  Scenario: Integer literal type inference
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i8 = 42;
        let y: i16 = 42;
        let z: i32 = 42;
        return (x as i32) + (y as i32) + z - 84;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Float literal type inference
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: f32 = 21.0;
        let y: f64 = 21.0;
        return (x as i32) + (y as i32);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases
  @wip
  Scenario: Maximum u8 value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let max: u8 = 255;
        return (max as i32) - 213;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Negative decimal literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let neg: i32 = -42;
        return -neg;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Negative hex literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let neg: i32 = -0x2A;
        return -neg;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Literal comparisons
  @wip
  Scenario: Compare different literal formats
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        if 42 == 0x2A && 0x2A == 0o52 && 0o52 == 0b101010 {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Literals in expressions
  @wip
  Scenario: Complex expression with literals
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        return (0xFF & 0b11111111) - (0o377 - 0x2A);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Const expressions with literals
  @wip
  Scenario: Const with decimal literal
    Given the following Asthra code:
      """
      package test;
      pub const ANSWER: i32 = 42;
      pub fn main(none) -> i32 {
        return ANSWER;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Const with hex literal
    Given the following Asthra code:
      """
      package test;
      pub const HEX_ANSWER: i32 = 0x2A;
      pub fn main(none) -> i32 {
        return HEX_ANSWER;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Zero literals in different bases
  @wip
  Scenario: Zero in all bases
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let dec_zero: i32 = 0;
        let hex_zero: i32 = 0x0;
        let bin_zero: i32 = 0b0;
        let oct_zero: i32 = 0o0;
        if dec_zero == hex_zero && hex_zero == bin_zero && bin_zero == oct_zero {
          return 42;
        } else {
          return 1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42