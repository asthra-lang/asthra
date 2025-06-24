Feature: Primitive Types
  As an Asthra developer
  I want to use primitive data types
  So that I can work with basic values in my programs

  Background:
    Given the Asthra compiler is available

  Scenario: Integer types - i32
    Given I have a file "int32_type.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = 42;
          let y: i32 = -100;
          let sum: i32 = x + y;
          log("i32 type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "i32 type works"
    And the exit code should be 0

  @wip
  Scenario: All signed integer types
    Given I have a file "signed_integers.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i8 = 127;
          let b: i16 = 32767;
          let c: i32 = 2147483647;
          let d: i64 = 9223372036854775807;
          let e: i128 = 170141183460469231731687303715884105727;
          let f: isize = 1000;
          log("All signed integers work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "All signed integers work"
    And the exit code should be 0

  @wip
  Scenario: All unsigned integer types
    Given I have a file "unsigned_integers.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: u8 = 255;
          let b: u16 = 65535;
          let c: u32 = 4294967295;
          let d: u64 = 18446744073709551615;
          let e: u128 = 340282366920938463463374607431768211455;
          let f: usize = 1000;
          log("All unsigned integers work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "All unsigned integers work"
    And the exit code should be 0

  Scenario: Floating point types
    Given I have a file "float_types.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: f32 = 3.14;
          let y: f64 = 2.718281828;
          let sum: f64 = y + 1.0;
          log("Float types work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Float types work"
    And the exit code should be 0

  Scenario: Boolean type
    Given I have a file "bool_type.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let is_true: bool = true;
          let is_false: bool = false;
          let result: bool = is_true && !is_false;
          log("Bool type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Bool type works"
    And the exit code should be 0

  Scenario: String type
    Given I have a file "string_type.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let greeting: string = "Hello, World!";
          let name: string = "Asthra";
          log(greeting);
          log(name);
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Hello, World!"
    And the output should contain "Asthra"
    And the exit code should be 0

  Scenario: Void type in function return
    Given I have a file "void_type.asthra" with:
      """
      package main;
      
      pub fn do_nothing(none) -> void {
          return ();
      }
      
      pub fn main(none) -> void {
          do_nothing();
          log("Void type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Void type works"
    And the exit code should be 0

  Scenario: Integer literal inference
    Given I have a file "int_inference.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = 42;
          let y: i64 = 42;
          let z: u32 = 42;
          log("Integer inference works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Integer inference works"
    And the exit code should be 0

  Scenario: Float literal inference
    Given I have a file "float_inference.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: f32 = 3.14;
          let y: f64 = 3.14;
          log("Float inference works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Float inference works"
    And the exit code should be 0

  @wip
  Scenario: Type mismatch error
    Given I have a file "type_mismatch.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = "not a number";
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "type mismatch"

  @wip
  Scenario: Integer overflow error
    Given I have a file "int_overflow.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i8 = 128;
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "integer overflow"

  Scenario: Invalid type name
    Given I have a file "invalid_type.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: integer = 42;
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "unknown type"

  @wip
  Scenario: Binary literals
    Given I have a file "binary_literals.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: u8 = 0b11111111;
          let y: u16 = 0b1010101010101010;
          log("Binary literals work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Binary literals work"
    And the exit code should be 0

  @wip
  Scenario: Hexadecimal literals
    Given I have a file "hex_literals.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: u8 = 0xFF;
          let y: u32 = 0xDEADBEEF;
          log("Hex literals work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Hex literals work"
    And the exit code should be 0