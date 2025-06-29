Feature: Boolean operators
  As a developer
  I want to use boolean operators and expressions
  So that I can write logical conditions and boolean arithmetic

  Background:
    Given the Asthra compiler is available

  Scenario: Boolean literals
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let t: bool = true;
          let f: bool = false;
          
          if t && !f {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Logical NOT operator
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let val: bool = true;
          let negated = !val;
          
          if negated {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Logical AND operator
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let a: bool = true;
          let b: bool = true;
          let c: bool = false;
          
          let result1 = a && b;
          let result2 = a && c;
          
          if result1 && !result2 {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Logical OR operator
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let a: bool = true;
          let b: bool = false;
          let c: bool = false;
          
          let result1 = a || b;
          let result2 = b || c;
          
          if result1 && !result2 {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Boolean operator precedence
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let a: bool = true;
          let b: bool = false;
          let c: bool = true;
          
          // NOT has highest precedence, then AND, then OR
          let result1 = a || b && c;    // true || (false && true) = true
          let result2 = !a || b && c;   // (!true) || (false && true) = false
          let result3 = a && b || c;    // (true && false) || true = true
          
          if result1 && !result2 && result3 {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Complex boolean expressions
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 5;
          let y: i32 = 10;
          let z: i32 = 15;
          
          let result = (x < y) && (y < z) || (x == 5);
          
          if result {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Boolean expressions as values
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_bool(val: i32) -> bool {
          return val > 0;
      }

      pub fn main(none) -> i32 {
          let a: bool = get_bool(5);
          let b: bool = get_bool(-5);
          let c: bool = get_bool(0);
          
          let result = a && !b && !c;
          
          if result {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Short-circuit evaluation with AND
    Given I have the following Asthra code:
      """
      package test;

      var counter: i32 = 0;

      pub fn increment_and_return_true(none) -> bool {
          counter = counter + 1;
          return true;
      }

      pub fn increment_and_return_false(none) -> bool {
          counter = counter + 1;
          return false;
      }

      pub fn main(none) -> i32 {
          // Should short-circuit after first false
          let result = increment_and_return_false() && increment_and_return_true();
          
          // Counter should be 1, not 2, due to short-circuit
          if !result && counter == 1 {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Short-circuit evaluation with OR
    Given I have the following Asthra code:
      """
      package test;

      var counter: i32 = 0;

      pub fn increment_and_return_true(none) -> bool {
          counter = counter + 1;
          return true;
      }

      pub fn increment_and_return_false(none) -> bool {
          counter = counter + 1;
          return false;
      }

      pub fn main(none) -> i32 {
          // Should short-circuit after first true
          let result = increment_and_return_true() || increment_and_return_false();
          
          // Counter should be 1, not 2, due to short-circuit
          if result && counter == 1 {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Nested boolean expressions
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let a: bool = true;
          let b: bool = false;
          let c: bool = true;
          let d: bool = false;
          
          let complex = (a && b) || (c && !d);
          let nested = !(!a || !c) && (b || !d);
          
          if complex && nested {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Boolean type inference
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let inferred = true;  // Type should be inferred as bool
          let also_inferred = !false;
          let expression = 5 > 3;
          
          if inferred && also_inferred && expression {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  @wip
  Scenario: Boolean assignment and mutation
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          var flag: bool = false;
          flag = true;
          flag = !flag;
          flag = flag || true;
          flag = flag && false;
          
          if !flag {
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Error - Type mismatch in boolean operation
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let num: i32 = 42;
          let result = !num;  // Error: NOT operator expects bool
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected bool"

  Scenario: Error - Non-boolean in logical AND
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let a: bool = true;
          let b: i32 = 1;
          let result = a && b;  // Error: AND expects both operands to be bool
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected bool"

  Scenario: Error - Non-boolean condition in if
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let num: i32 = 42;
          
          if num {  // Error: if condition must be bool
              return 0;
          } else {
              return 1;
          }
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "condition" or "expected bool"