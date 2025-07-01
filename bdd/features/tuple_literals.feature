Feature: Tuple Literals
  As a developer using Asthra
  I want to use tuple literal syntax
  So that I can create and work with fixed-size heterogeneous collections

  Background:
    Given a new compilation context

  # Basic tuple creation (minimum 2 elements)

  Scenario: Two-element integer tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pair: (i32, i32) = (20, 22);
        return pair.0 + pair.1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Two-element mixed type tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mixed: (i32, bool) = (42, true);
        if mixed.1 {
          return mixed.0;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Three-element tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let triple: (i32, i32, i32) = (10, 15, 17);
        return triple.0 + triple.1 + triple.2;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Four-element tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let quad: (i8, i16, i32, i8) = (5, 10, 20, 7);
        return (quad.0 as i32) + (quad.1 as i32) + quad.2 + (quad.3 as i32);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple element access

  Scenario: Access first element
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let tup: (i32, i32, i32) = (42, 100, 200);
        return tup.0;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Access middle element
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let tup: (i32, i32, i32, i32, i32) = (1, 2, 42, 4, 5);
        return tup.2;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Access last element
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let tup: (i32, i32, i32) = (10, 20, 42);
        return tup.2;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mixed type tuples

  Scenario: Integer and boolean tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let data: (i32, bool) = (84, false);
        if !data.1 {
          return data.0 / 2;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String and integer tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let info: (string, i32) = ("answer", 42);
        if info.0 == "answer" {
          return info.1;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Float and integer tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let nums: (f32, i32) = (21.0, 21);
        return (nums.0 as i32) + nums.1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Nested tuples

  Scenario: Tuple containing tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let nested: ((i32, i32), i32) = ((20, 15), 7);
        let inner: (i32, i32) = nested.0;
        return inner.0 + inner.1 + nested.1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Deeply nested tuples
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let deep: (i32, (i32, (i32, i32))) = (10, (12, (15, 5)));
        return deep.0 + deep.1.0 + deep.1.1.0 + deep.1.1.1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple pattern matching

  Scenario: Basic tuple destructuring
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pair: (i32, i32) = (17, 25);
        let (a, b): (i32, i32) = pair;
        return a + b;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Tuple destructuring with underscore
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let triple: (i32, i32, i32) = (42, 100, 200);
        let (x, _, _): (i32, i32, i32) = triple;
        return x;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Nested tuple destructuring
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let nested: ((i32, i32), i32) = ((15, 20), 7);
        let ((a, b), c): ((i32, i32), i32) = nested;
        return a + b + c;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuples as function parameters

  Scenario: Pass tuple to function
    Given the following Asthra code:
      """
      package test;
      pub fn sum_pair(p: (i32, i32)) -> i32 {
        return p.0 + p.1;
      }
      pub fn main(none) -> i32 {
        let pair: (i32, i32) = (20, 22);
        return sum_pair(pair);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Return tuple from function
    Given the following Asthra code:
      """
      package test;
      pub fn make_pair(none) -> (i32, i32) {
        return (17, 25);
      }
      pub fn main(none) -> i32 {
        let p: (i32, i32) = make_pair(none);
        return p.0 + p.1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mutable tuples

  Scenario: Mutable tuple reassignment
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut tup: (i32, i32) = (10, 20);
        tup = (15, 27);
        return tup.0 + tup.1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple comparison

  Scenario: Compare equal tuples
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t1: (i32, i32) = (21, 21);
        let t2: (i32, i32) = (21, 21);
        if t1 == t2 {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Compare different tuples
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t1: (i32, bool) = (42, true);
        let t2: (i32, bool) = (42, false);
        if t1 != t2 {
          return t1.0;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuples in expressions

  Scenario: Tuple in conditional expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let result: (i32, bool) = (42, true);
        return if result.1 { result.0 } else { 0 };
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Tuple element in arithmetic
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let nums: (i32, i32, i32) = (8, 5, 3);
        return nums.0 * nums.1 + nums.2 - 1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuples with arrays

  Scenario: Array of tuples
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pairs: [](i32, i32) = [(10, 5), (15, 7), (20, 10)];
        let p: (i32, i32) = pairs[1];
        return p.0 + p.1 + 20;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Tuple containing array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let data: ([]i32, i32) = ([10, 20, 30], 12);
        return data.0[0] + data.0[1] + data.1 - 10;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases

  Scenario: Large tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let big: (i32, i32, i32, i32, i32, i32, i32, i32) = (1, 2, 3, 4, 5, 6, 7, 12);
        return big.0 + big.1 + big.2 + big.3 + big.4 + big.5 + big.6 + big.7;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Tuple with unit types
    Given the following Asthra code:
      """
      package test;
      pub fn nothing(none) -> void { return (); }
      pub fn main(none) -> i32 {
        let t: (i32, void) = (42, nothing(none));
        return t.0;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Const tuples

  Scenario: Const tuple declaration
    Given the following Asthra code:
      """
      package test;
      pub const PAIR: (i32, i32) = (20, 22);
      pub fn main(none) -> i32 {
        return PAIR.0 + PAIR.1;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple in match expressions

  Scenario: Match on tuple value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pair: (i32, bool) = (42, true);
        match pair {
          (42, true) => { return 42; }
          (42, false) => { return 0; }
          _ => { return 1; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple with if-let

  Scenario: If-let with tuple pattern
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let data: (i32, i32) = (21, 21);
        if let (x, y) = data {
          return x + y;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42