Feature: Const Declarations
  As an Asthra developer
  I want to declare compile-time constants
  So that I can use immutable values throughout my program

  Background:
    Given I have a clean test environment
    And I am testing "const_declarations"

  # Basic const declarations with various types
  Scenario: Basic const declaration with integer
    Given the following Asthra code:
      """
      package test;

      pub const MAX_SIZE: i32 = 100;

      pub fn main(none) -> void {
          let x: i32 = MAX_SIZE;
          log("Basic const integer works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Basic const integer works"

  Scenario: Const declaration with float
    Given the following Asthra code:
      """
      package test;

      pub const PI: f64 = 3.14159;

      pub fn main(none) -> void {
          let area: f64 = PI * 2.0 * 2.0;
          log("Const float works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Const float works"

  Scenario: Const declaration with string
    Given the following Asthra code:
      """
      package test;

      pub const VERSION: string = "1.0.0";

      pub fn main(none) -> void {
          log("Version: " + VERSION);
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Version: 1.0.0"

  Scenario: Const declaration with boolean
    Given the following Asthra code:
      """
      package test;

      pub const DEBUG_MODE: bool = true;

      pub fn main(none) -> void {
          if DEBUG_MODE {
              log("Debug mode is enabled");
          }
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Debug mode is enabled"

  # Const expressions
  Scenario: Const with binary arithmetic expression
    Given the following Asthra code:
      """
      package test;

      pub const SIZE: i32 = 10;
      pub const DOUBLE_SIZE: i32 = SIZE * 2;
      pub const TOTAL: i32 = DOUBLE_SIZE + 5;

      pub fn main(none) -> void {
          let result: i32 = TOTAL;
          log("Const binary expression works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Const binary expression works"

  Scenario: Const with unary expression
    Given the following Asthra code:
      """
      package test;

      pub const POSITIVE: i32 = 42;
      pub const NEGATIVE: i32 = -POSITIVE;

      pub fn main(none) -> void {
          let neg: i32 = NEGATIVE;
          log("Const unary expression works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Const unary expression works"

  Scenario: Const with sizeof expression
    Given the following Asthra code:
      """
      package test;

      pub const INT_SIZE: usize = sizeof(i32);
      pub const FLOAT_SIZE: usize = sizeof(f64);

      pub fn main(none) -> void {
          log("Const sizeof works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Const sizeof works"

  # Using consts in array sizes
  Scenario: Const used as array size
    Given the following Asthra code:
      """
      package test;

      pub const BUFFER_SIZE: usize = 10;

      pub fn main(none) -> void {
          let buffer: [BUFFER_SIZE]i32 = [0; BUFFER_SIZE];
          log("Const array size works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Const array size works"

  # Visibility modifiers
  Scenario: Private const declaration
    Given the following Asthra code:
      """
      package test;

      priv const SECRET_KEY: i32 = 12345;

      pub fn get_key(none) -> i32 {
          return SECRET_KEY;
      }

      pub fn main(none) -> void {
          let key: i32 = get_key();
          log("Private const works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Private const works"

  # Numeric literal types
  Scenario: Const with hex literal
    Given the following Asthra code:
      """
      package test;

      pub const HEX_VALUE: i32 = 0xFF;

      pub fn main(none) -> void {
          let hex: i32 = HEX_VALUE;
          log("Hex literal const works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Hex literal const works"

  Scenario: Const with binary literal
    Given the following Asthra code:
      """
      package test;

      pub const BINARY_VALUE: i32 = 0b11111111;

      pub fn main(none) -> void {
          let bin: i32 = BINARY_VALUE;
          log("Binary literal const works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Binary literal const works"

  Scenario: Const with octal literal
    Given the following Asthra code:
      """
      package test;

      pub const OCTAL_VALUE: i32 = 0o777;

      pub fn main(none) -> void {
          let oct: i32 = OCTAL_VALUE;
          log("Octal literal const works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Octal literal const works"

  # Error scenarios
  Scenario: Const without type annotation
    Given the following Asthra code:
      """
      package test;

      pub const VALUE = 42;

      pub fn main(none) -> void {
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type annotation"

  Scenario: Const with non-const expression
    Given the following Asthra code:
      """
      package test;

      pub fn get_value(none) -> i32 {
          return 42;
      }

      pub const VALUE: i32 = get_value();

      pub fn main(none) -> void {
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "constant expression"

  Scenario: Duplicate const declaration
    Given the following Asthra code:
      """
      package test;

      pub const VALUE: i32 = 42;
      pub const VALUE: i32 = 100;

      pub fn main(none) -> void {
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "already defined"

  Scenario: Const reassignment attempt
    Given the following Asthra code:
      """
      package test;

      pub const VALUE: i32 = 42;

      pub fn main(none) -> void {
          VALUE = 100;
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "cannot assign"

  # Complex const expressions
  Scenario: Const with multiple arithmetic operations
    Given the following Asthra code:
      """
      package test;

      pub const BASE: i32 = 10;
      pub const MULTIPLIER: i32 = 3;
      pub const OFFSET: i32 = 5;
      pub const RESULT: i32 = BASE * MULTIPLIER + OFFSET;

      pub fn main(none) -> void {
          let val: i32 = RESULT;
          log("Complex const arithmetic works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Complex const arithmetic works"

  Scenario: Const with character literal
    Given the following Asthra code:
      """
      package test;

      pub const NEWLINE: char = '\n';
      pub const TAB: char = '\t';
      pub const LETTER_A: char = 'A';

      pub fn main(none) -> void {
          log("Char literal const works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Char literal const works"