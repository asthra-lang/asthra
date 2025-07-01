Feature: Tuple Patterns
  As a developer using Asthra
  I want to use pattern matching to destructure tuples
  So that I can extract values from tuples efficiently

  Background:
    Given a new compilation context

  # Basic tuple destructuring
  @wip
  Scenario: Match tuple with two elements
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32) = (10, 32);
        match t {
          (a, b) => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Match tuple with three elements
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32, i32) = (20, 20, 2);
        match t {
          (a, b, c) => { return a + b + c; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Match tuple with wildcard patterns
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32, i32) = (42, 100, 200);
        match t {
          (x, _, _) => { return x; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # If-let tuple patterns
  @wip
  Scenario: If-let with tuple pattern
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32) = (40, 2);
        if let (a, b) = t {
          return a + b;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Nested tuple patterns
  @wip
  Scenario: Match nested tuples
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: ((i32, i32), i32) = ((20, 22), 0);
        match t {
          ((a, b), _) => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Match deeply nested tuples
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, (i32, (i32, i32))) = (1, (10, (11, 20)));
        match t {
          (a, (b, (c, d))) => { return a + b + c + d; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mixed type tuples
  @wip
  Scenario: Match tuple with different types
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, bool, i32) = (20, true, 22);
        match t {
          (a, true, b) => { return a + b; }
          (a, false, b) => { return a - b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple patterns in functions
  @wip
  Scenario: Tuple pattern in function parameter
    Given the following Asthra code:
      """
      package test;
      pub fn add_pair(p: (i32, i32)) -> i32 {
        match p {
          (a, b) => { return a + b; }
        }
      }
      pub fn main(none) -> i32 {
        let pair: (i32, i32) = (20, 22);
        return add_pair(pair);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple element access
  @wip
  Scenario: Access tuple elements with index syntax
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32, i32) = (42, 100, 200);
        return t.0;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Access nested tuple elements
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: ((i32, i32), i32) = ((42, 100), 200);
        return t.0.0;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Complex patterns
  @wip
  Scenario: Match tuple in struct
    Given the following Asthra code:
      """
      package test;
      pub struct Container {
        data: (i32, i32)
      }
      pub fn main(none) -> i32 {
        let c: Container = Container { data: (20, 22) };
        match c.data {
          (a, b) => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Match tuple in enum variant
    Given the following Asthra code:
      """
      package test;
      pub enum Data {
        Pair(i32, i32),
        Single(i32)
      }
      pub fn main(none) -> i32 {
        let d: Data = Data.Pair(20, 22);
        match d {
          Data.Pair(a, b) => { return a + b; }
          Data.Single(x) => { return x; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Multiple patterns
  @wip
  Scenario: Match multiple tuple patterns
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32) = (42, 0);
        match t {
          (0, y) => { return y; }
          (x, 0) => { return x; }
          (x, y) => { return x + y; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern matching expressions
  @wip
  Scenario: Tuple pattern in match expression
    Given the following Asthra code:
      """
      package test;
      pub fn compute(none) -> (i32, i32) {
        return (40, 2);
      }
      pub fn main(none) -> i32 {
        match compute(none) {
          (a, b) => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Variable binding in tuple patterns
  @wip
  Scenario: Bind variables in nested tuple pattern
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, (i32, i32)) = (10, (12, 20));
        match t {
          (x, (y, z)) => {
            let sum: i32 = x + y + z;
            return sum;
          }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases
  @wip
  Scenario: Match tuple with all wildcards except one
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32, i32, i32) = (1, 42, 3, 4);
        match t {
          (_, x, _, _) => { return x; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Tuple pattern with literal matching
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32) = (1, 41);
        match t {
          (1, x) => { return x + 1; }
          (2, x) => { return x + 2; }
          (_, x) => { return x; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple assignment through pattern matching
  @wip
  Scenario: Destructure tuple in let binding
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32) = (20, 22);
        let (a, b): (i32, i32) = t;
        return a + b;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mutable tuple elements
  @wip
  Scenario: Match and modify tuple elements
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut t: (i32, i32) = (20, 20);
        match t {
          (a, b) => {
            t = (a + 1, b + 1);
          }
        }
        match t {
          (x, y) => { return x + y; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple patterns with string types
  @wip
  Scenario: Match tuple containing strings
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (string, i32) = ("answer", 42);
        match t {
          ("answer", x) => { return x; }
          (_, x) => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42