Feature: Logical Operators
  As a developer using Asthra
  I want to use logical operators (&&, ||, !)
  So that I can write boolean expressions and control flow logic

  Background:
    Given a new compilation context

  # Logical AND operator tests
  Scenario: Basic logical AND with true values
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = true;
        let result: bool = a && b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Logical AND with false values
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let result: bool = a && b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Logical AND with all false
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = false;
        let b: bool = false;
        let result: bool = a && b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Multiple logical AND operations
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = true;
        let c: bool = true;
        let d: bool = false;
        let result: bool = a && b && c && d;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  # Logical OR operator tests
  Scenario: Basic logical OR with true values
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let result: bool = a || b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Logical OR with all false
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = false;
        let b: bool = false;
        let result: bool = a || b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Logical OR with all true
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = true;
        let result: bool = a || b;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Multiple logical OR operations
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = false;
        let b: bool = false;
        let c: bool = false;
        let d: bool = true;
        let result: bool = a || b || c || d;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Logical NOT operator tests
  Scenario: Basic logical NOT on true
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let result: bool = !a;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Logical NOT on false
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = false;
        let result: bool = !a;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Double logical NOT
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let result: bool = !(!a);
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Mixed logical operators
  Scenario: AND has higher precedence than OR
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let c: bool = true;
        let result: bool = a || b && c;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Complex logical expression with parentheses
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let c: bool = true;
        let result: bool = (a || b) && c;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: NOT with AND and OR
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let c: bool = true;
        let result: bool = !a || b && c;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  # Logical operators with comparisons
  Scenario: Logical operators with comparison expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let x: i32 = 5;
        let y: i32 = 10;
        let z: i32 = 15;
        let result: bool = x < y && y < z;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Complex comparison with logical operators
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let x: i32 = 5;
        let y: i32 = 10;
        let result: bool = x > 0 && x < 10 || y == 10;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Short-circuit evaluation
  Scenario: Logical AND short-circuit evaluation
    Given the following Asthra code:
      """
      package test;
      pub fn always_false() -> bool {
        log("This should not be called");
        return false;
      }
      
      pub fn main() -> i32 {
        let result: bool = false && always_false();
        if result {
          return 0;
        } else {
          return 1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the output should not contain "This should not be called"

  Scenario: Logical OR short-circuit evaluation
    Given the following Asthra code:
      """
      package test;
      pub fn always_true() -> bool {
        log("This should not be called");
        return true;
      }
      
      pub fn main() -> i32 {
        let result: bool = true || always_true();
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1
    And the output should not contain "This should not be called"

  # Logical operators in control flow
  Scenario: Logical operators in if conditions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let x: i32 = 5;
        let y: i32 = 10;
        
        if x > 0 && y > 0 {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Nested if with logical operators
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let c: bool = true;
        
        if a || b {
          if !b && c {
            return 1;
          }
        }
        return 0;
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Logical operators with mutable variables
  Scenario: Logical operators with mutable boolean
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut flag: bool = false;
        flag = true;
        
        if flag && true {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Complex mutable logic
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut a: bool = true;
        let mut b: bool = false;
        let mut c: bool = true;
        
        b = a && c;
        a = !b || c;
        c = a && !b;
        
        if a && !b && !c {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  # Edge cases
  Scenario: All logical operators in one expression
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let c: bool = true;
        let d: bool = false;
        
        let result: bool = !a || b && c || !d;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: Logical operators with function calls
    Given the following Asthra code:
      """
      package test;
      pub fn is_positive(x: i32) -> bool {
        return x > 0;
      }
      
      pub fn is_even(x: i32) -> bool {
        return x % 2 == 0;
      }
      
      pub fn main() -> i32 {
        let num: i32 = 10;
        
        if is_positive(num) && is_even(num) {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Boolean literals in expressions
  Scenario: Direct boolean literals in logical expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let result: bool = true && false || true;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  Scenario: NOT operator with boolean literals
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let result: bool = !false && !false;
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Complex nested expressions
  Scenario: Deeply nested logical expressions
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let a: bool = true;
        let b: bool = false;
        let c: bool = true;
        let d: bool = true;
        
        let result: bool = ((a && b) || (c && d)) && (!(a && !b) || (c || !d));
        if result {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  # Logical operators in loops
  Scenario: Logical operators in for loop condition
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let mut count: i32 = 0;
        let mut i: i32 = 0;
        
        for j in range(10) {
          if j > 2 && j < 7 {
            count = count + 1;
          }
        }
        
        return count;
      }
      """
    When I compile and run the code
    Then the exit code should be 4

  # Logical assignment patterns
  Scenario: Assigning logical expression results
    Given the following Asthra code:
      """
      package test;
      pub fn main() -> i32 {
        let x: i32 = 5;
        let y: i32 = 10;
        
        let is_valid: bool = x > 0 && x < 10 && y >= 10;
        let is_invalid: bool = !is_valid;
        
        if is_valid && !is_invalid {
          return 1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1