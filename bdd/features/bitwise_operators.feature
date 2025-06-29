Feature: Bitwise Operators
  As a developer
  I want to use bitwise operators in Asthra
  So that I can perform low-level bit manipulation operations

  Background:
    Given the Asthra compiler is available

  Scenario: Bitwise AND operation
    Given I have a file "bitwise_and.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 12;  // Binary: 1100
          let b: i32 = 10;  // Binary: 1010
          let result: i32 = a & b;  // Binary: 1000 = 8
          
          if result == 8 {
              log("Bitwise AND: 12 & 10 = 8");
          }
          
          // Test with different values
          let x: i32 = 255;  // Binary: 11111111
          let y: i32 = 15;   // Binary: 00001111
          let mask_result: i32 = x & y;  // Binary: 00001111 = 15
          
          if mask_result == 15 {
              log("Masking operation: 255 & 15 = 15");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Bitwise AND: 12 & 10 = 8"
    And the output should contain "Masking operation: 255 & 15 = 15"
    And the exit code should be 0

  Scenario: Bitwise OR operation
    Given I have a file "bitwise_or.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 12;  // Binary: 1100
          let b: i32 = 10;  // Binary: 1010
          let result: i32 = a | b;  // Binary: 1110 = 14
          
          if result == 14 {
              log("Bitwise OR: 12 | 10 = 14");
          }
          
          // Setting flags
          let flags: i32 = 0;
          let flag1: i32 = 1;  // Binary: 0001
          let flag2: i32 = 4;  // Binary: 0100
          let flag3: i32 = 8;  // Binary: 1000
          
          let all_flags: i32 = flags | flag1 | flag2 | flag3;  // Binary: 1101 = 13
          
          if all_flags == 13 {
              log("Flag combination: 0 | 1 | 4 | 8 = 13");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Bitwise OR: 12 | 10 = 14"
    And the output should contain "Flag combination: 0 | 1 | 4 | 8 = 13"
    And the exit code should be 0

  Scenario: Bitwise XOR operation
    Given I have a file "bitwise_xor.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 12;  // Binary: 1100
          let b: i32 = 10;  // Binary: 1010
          let result: i32 = a ^ b;  // Binary: 0110 = 6
          
          if result == 6 {
              log("Bitwise XOR: 12 ^ 10 = 6");
          }
          
          // XOR swap trick
          let mut x: i32 = 5;
          let mut y: i32 = 7;
          
          x = x ^ y;  // x now contains 5 ^ 7
          y = x ^ y;  // y = (5 ^ 7) ^ 7 = 5
          x = x ^ y;  // x = (5 ^ 7) ^ 5 = 7
          
          if x == 7 && y == 5 {
              log("XOR swap successful: x=7, y=5");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Bitwise XOR: 12 ^ 10 = 6"
    And the output should contain "XOR swap successful: x=7, y=5"
    And the exit code should be 0

  Scenario: Bitwise NOT operation
    Given I have a file "bitwise_not.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 0;
          let not_a: i32 = ~a;  // All bits flipped
          
          if not_a == -1 {
              log("Bitwise NOT: ~0 = -1");
          }
          
          let b: i32 = 15;  // Binary: 00001111
          let not_b: i32 = ~b;  // Binary: 11110000 (as signed int)
          
          if not_b == -16 {
              log("Bitwise NOT: ~15 = -16");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Bitwise NOT: ~0 = -1"
    And the output should contain "Bitwise NOT: ~15 = -16"
    And the exit code should be 0

  Scenario: Left shift operation
    Given I have a file "left_shift.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 5;   // Binary: 0101
          let result: i32 = a << 2;  // Binary: 10100 = 20
          
          if result == 20 {
              log("Left shift: 5 << 2 = 20");
          }
          
          // Power of 2 multiplication
          let base: i32 = 1;
          let power_of_8: i32 = base << 3;  // 1 * 2^3 = 8
          
          if power_of_8 == 8 {
              log("Power of 2: 1 << 3 = 8");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Left shift: 5 << 2 = 20"
    And the output should contain "Power of 2: 1 << 3 = 8"
    And the exit code should be 0

  Scenario: Right shift operation
    Given I have a file "right_shift.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 20;  // Binary: 10100
          let result: i32 = a >> 2;  // Binary: 00101 = 5
          
          if result == 5 {
              log("Right shift: 20 >> 2 = 5");
          }
          
          // Division by power of 2
          let value: i32 = 64;
          let divided: i32 = value >> 3;  // 64 / 2^3 = 8
          
          if divided == 8 {
              log("Division by power of 2: 64 >> 3 = 8");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Right shift: 20 >> 2 = 5"
    And the output should contain "Division by power of 2: 64 >> 3 = 8"
    And the exit code should be 0

  Scenario: Combined bitwise operations
    Given I have a file "combined_bitwise.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          // Extract bits using mask
          let value: i32 = 0b11010110;  // Binary: 11010110 = 214
          let mask: i32 = 0b00001111;   // Binary: 00001111 = 15
          let lower_nibble: i32 = value & mask;  // Extract lower 4 bits
          
          if lower_nibble == 6 {
              log("Lower nibble extracted: 6");
          }
          
          // Set specific bit
          let mut flags: i32 = 0b00000000;
          let bit_position: i32 = 3;
          flags = flags | (1 << bit_position);  // Set bit 3
          
          if flags == 8 {
              log("Bit 3 set successfully: 8");
          }
          
          // Toggle bit
          flags = flags ^ (1 << bit_position);  // Toggle bit 3
          
          if flags == 0 {
              log("Bit 3 toggled successfully: 0");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Lower nibble extracted: 6"
    And the output should contain "Bit 3 set successfully: 8"
    And the output should contain "Bit 3 toggled successfully: 0"
    And the exit code should be 0

  @wip
  Scenario: Bitwise operations with different integer types
    Given I have a file "bitwise_types.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          // i64 operations
          let a_64: i64 = 0xFF00FF00FF00FF00;
          let b_64: i64 = 0x00FF00FF00FF00FF;
          let result_64: i64 = a_64 & b_64;
          
          if result_64 == 0 {
              log("i64 bitwise AND: alternating pattern = 0");
          }
          
          // Mixed with arithmetic
          let x: i32 = 15;
          let shifted_and_added: i32 = (x << 2) + (x >> 1);  // (15 * 4) + (15 / 2) = 60 + 7 = 67
          
          if shifted_and_added == 67 {
              log("Mixed arithmetic and bitwise: 67");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "i64 bitwise AND: alternating pattern = 0"
    And the output should contain "Mixed arithmetic and bitwise: 67"
    And the exit code should be 0

  Scenario: Bitwise operator precedence
    Given I have a file "bitwise_precedence.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          // Precedence: shift operators have lower precedence than arithmetic
          let a: i32 = 2 + 3 << 1;  // (2 + 3) << 1 = 5 << 1 = 10
          
          if a == 10 {
              log("Shift precedence: 2 + 3 << 1 = 10");
          }
          
          // Bitwise AND has lower precedence than comparison
          let b: i32 = 7;
          let c: i32 = 3;
          let result: bool = (b & c) == 3;  // Parentheses needed
          
          if result {
              log("Bitwise AND with comparison: (7 & 3) == 3 is true");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Shift precedence: 2 + 3 << 1 = 10"
    And the output should contain "Bitwise AND with comparison: (7 & 3) == 3 is true"
    And the exit code should be 0

  Scenario: Error - bitwise operations on non-integer types
    Given I have a file "bitwise_type_error.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: f32 = 3.14;
          let b: f32 = 2.71;
          let result: f32 = a & b;  // Error: bitwise AND on float
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "bitwise"

  @wip
  Scenario: Error - shift by negative amount
    Given I have a file "negative_shift.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 10;
          let shift_amount: i32 = -2;
          let result: i32 = a << shift_amount;  // Error or undefined behavior
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "shift"

  @wip
  Scenario: Binary literals with bitwise operations
    Given I have a file "binary_literals.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let byte_mask: i32 = 0b11111111;  // 255
          let nibble_mask: i32 = 0b1111;    // 15
          
          let value: i32 = 0b10101010;      // 170
          let masked: i32 = value & nibble_mask;
          
          if masked == 0b1010 {  // 10
              log("Binary literal masking: 0b10101010 & 0b1111 = 0b1010");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Binary literal masking: 0b10101010 & 0b1111 = 0b1010"
    And the exit code should be 0