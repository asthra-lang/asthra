Feature: Bitwise Operators
  As a developer using Asthra
  I want to use bitwise operators (&, |, ^, ~, <<, >>)
  So that I can perform low-level bit manipulation operations

  Background:
    Given a new compilation context

  # Bitwise AND operator tests
  Scenario: Basic bitwise AND operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 12;  // Binary: 1100
        let b: i32 = 10;  // Binary: 1010
        let result: i32 = a & b;  // Binary: 1000 = 8
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 8

  Scenario: Bitwise AND with zero
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 255;
        let b: i32 = 0;
        let result: i32 = a & b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Bitwise AND masking operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let value: i32 = 0xFF;      // 255
        let mask: i32 = 0x0F;       // 15 (lower nibble)
        let result: i32 = value & mask;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 15

  Scenario: Multiple bitwise AND operations
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0b11111111;  // 255
        let b: i32 = 0b11110000;  // 240
        let c: i32 = 0b11001100;  // 204
        let result: i32 = a & b & c;  // 0b11000000 = 192
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 192

  # Bitwise OR operator tests
  Scenario: Basic bitwise OR operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 12;  // Binary: 1100
        let b: i32 = 10;  // Binary: 1010
        let result: i32 = a | b;  // Binary: 1110 = 14
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 14

  Scenario: Bitwise OR with zero
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 42;
        let b: i32 = 0;
        let result: i32 = a | b;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Setting bits with OR
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let flags: i32 = 0;
        let flag1: i32 = 1;   // Bit 0
        let flag2: i32 = 4;   // Bit 2
        let flag3: i32 = 16;  // Bit 4
        let result: i32 = flags | flag1 | flag2 | flag3;  // 21
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 21

  # Bitwise XOR operator tests
  Scenario: Basic bitwise XOR operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 12;  // Binary: 1100
        let b: i32 = 10;  // Binary: 1010
        let result: i32 = a ^ b;  // Binary: 0110 = 6
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 6

  Scenario: XOR with same value
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 255;
        let result: i32 = a ^ a;  // Always 0
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: XOR toggle operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let value: i32 = 0b10101010;  // 170
        let toggle: i32 = 0b11111111; // 255
        let result: i32 = value ^ toggle;  // 0b01010101 = 85
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 85

  Scenario: XOR swap algorithm
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut x: i32 = 5;
        let mut y: i32 = 7;
        
        x = x ^ y;  // x = 5 ^ 7
        y = x ^ y;  // y = (5 ^ 7) ^ 7 = 5
        x = x ^ y;  // x = (5 ^ 7) ^ 5 = 7
        
        return x * 10 + y;  // 75
      }
      """
    When I compile and run the code
    Then the exit code should be 75

  # Bitwise NOT (complement) operator tests
  Scenario: Basic bitwise NOT operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0;
        let result: i32 = ~a;  // -1 (all bits set)
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -1

  Scenario: Bitwise NOT on positive value
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 15;  // Binary: 00001111
        let result: i32 = ~a;  // Binary: 11110000 = -16
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -16

  Scenario: Double bitwise NOT
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 42;
        let result: i32 = ~(~a);  // Should be 42
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Left shift operator tests
  Scenario: Basic left shift operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 5;   // Binary: 0101
        let result: i32 = a << 2;  // Binary: 10100 = 20
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 20

  Scenario: Left shift by zero
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 42;
        let result: i32 = a << 0;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Left shift for multiplication by power of 2
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 3;
        let result: i32 = a << 4;  // 3 * 16 = 48
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 48

  Scenario: Multiple left shifts
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 1;
        let result: i32 = a << 1 << 2 << 1;  // 1 * 2 * 4 * 2 = 16
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 16

  # Right shift operator tests
  Scenario: Basic right shift operation
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 20;  // Binary: 10100
        let result: i32 = a >> 2;  // Binary: 00101 = 5
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 5

  Scenario: Right shift by zero
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 42;
        let result: i32 = a >> 0;
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Right shift for division by power of 2
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 64;
        let result: i32 = a >> 3;  // 64 / 8 = 8
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 8

  Scenario: Right shift with negative number
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = -16;
        let result: i32 = a >> 2;  // Arithmetic shift: -4
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be -4

  # Combined operations tests
  Scenario: Bit extraction using mask
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let value: i32 = 0b11010110;  // 214
        let mask: i32 = 0b00001111;   // 15
        let lower_nibble: i32 = value & mask;  // Extract lower 4 bits = 6
        return lower_nibble;
      }
      """
    When I compile and run the code
    Then the exit code should be 6

  Scenario: Setting specific bit
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut flags: i32 = 0b00000100;  // 4
        let bit_position: i32 = 3;
        flags = flags | (1 << bit_position);  // Set bit 3
        return flags;  // 12
      }
      """
    When I compile and run the code
    Then the exit code should be 12

  Scenario: Clearing specific bit
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut flags: i32 = 0b00001111;  // 15
        let bit_position: i32 = 2;
        flags = flags & ~(1 << bit_position);  // Clear bit 2
        return flags;  // 11
      }
      """
    When I compile and run the code
    Then the exit code should be 11

  Scenario: Toggling specific bit
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut flags: i32 = 0b00001010;  // 10
        let bit_position: i32 = 0;
        flags = flags ^ (1 << bit_position);  // Toggle bit 0
        return flags;  // 11
      }
      """
    When I compile and run the code
    Then the exit code should be 11

  # Precedence tests
  Scenario: Shift has lower precedence than arithmetic
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let result: i32 = 2 + 3 << 1;  // (2 + 3) << 1 = 5 << 1 = 10
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 10

  Scenario: Bitwise AND precedence with comparison
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 7;
        let b: i32 = 3;
        if (a & b) == 3 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Mixed bitwise operations precedence
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 12;
        let b: i32 = 10;
        let c: i32 = 6;
        let result: i32 = a | b & c;  // a | (b & c) = 12 | 2 = 14
        return result;
      }
      """
    When I compile and run the code
    Then the exit code should be 14

  # Different integer types
  Scenario: Bitwise operations with u8
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: u8 = 255;
        let b: u8 = 170;  // 0b10101010
        let result: u8 = a ^ b;  // 0b01010101 = 85
        return result as i32;
      }
      """
    When I compile and run the code
    Then the exit code should be 85

  @wip
  Scenario: Bitwise operations with i64
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i64 = 0xFFFFFFFF00000000;
        let b: i64 = 0x00000000FFFFFFFF;
        let result: i64 = a | b;  // All bits set
        if result == -1 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Edge cases
  Scenario: Shift by large amount
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 1;
        let result: i32 = a << 31;  // Largest valid shift for i32
        if result < 0 {  // Should be negative (sign bit set)
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Complex expressions
  Scenario: Complex bitwise expression
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0b11001100;  // 204
        let b: i32 = 0b10101010;  // 170
        let c: i32 = 0b11110000;  // 240
        let result: i32 = (a & b) | (b ^ c) | ~(a | c);
        return result & 0xFF;  // Mask to get lower byte
      }
      """
    When I compile and run the code
    Then the exit code should be 95

  # Bitwise operations in conditions
  Scenario: Bitwise operations in if conditions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let flags: i32 = 0b00001010;  // 10
        let mask: i32 = 0b00000010;   // Check bit 1
        
        if (flags & mask) != 0 {
          return 1;  // Bit is set
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Binary literals
  Scenario: Binary literals with all operations
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: i32 = 0b1010;  // 10
        let b: i32 = 0b1100;  // 12
        
        let and_result: i32 = a & b;  // 0b1000 = 8
        let or_result: i32 = a | b;   // 0b1110 = 14
        let xor_result: i32 = a ^ b;  // 0b0110 = 6
        
        return and_result + or_result + xor_result;  // 28
      }
      """
    When I compile and run the code
    Then the exit code should be 28

  # Hexadecimal literals
  Scenario: Hexadecimal literals with bitwise operations
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let high_byte: i32 = 0xFF00;
        let low_byte: i32 = 0x00FF;
        let value: i32 = 0xABCD;
        
        let high: i32 = (value & high_byte) >> 8;  // 0xAB = 171
        let low: i32 = value & low_byte;           // 0xCD = 205
        
        return high + low;  // 376
      }
      """
    When I compile and run the code
    Then the exit code should be 376

  # Practical bit manipulation patterns
  Scenario: Check if number is power of 2
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let n: i32 = 16;
        // Power of 2 has only one bit set, so n & (n-1) == 0
        if n > 0 && (n & (n - 1)) == 0 {
          return 1;  // Is power of 2
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Count trailing zeros using bitwise operations
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let n: i32 = 40;  // Binary: 101000
        let mut count: i32 = 0;
        let mut value: i32 = n;
        
        // Count trailing zeros
        if value != 0 {
          if (value & 1) == 0 {
            count = count + 1;
            value = value >> 1;
            if (value & 1) == 0 {
              count = count + 1;
              value = value >> 1;
              if (value & 1) == 0 {
                count = count + 1;
              }
            }
          }
        }
        
        return count;  // 3 trailing zeros
      }
      """
    When I compile and run the code
    Then the exit code should be 3