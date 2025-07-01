Feature: Struct Patterns
  As a developer using Asthra
  I want to use pattern matching to destructure structs
  So that I can extract values from structs efficiently

  Background:
    Given a new compilation context

  # Basic struct destructuring

  Scenario: Match struct with all fields
    Given the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let p: Point = Point { x: 42, y: 100 };
        match p {
          Point { x: a, y: b } => { return a; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Match struct with field reordering
    Given the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let p: Point = Point { x: 10, y: 32 };
        match p {
          Point { y: b, x: a } => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Match struct with wildcard fields
    Given the following Asthra code:
      """
      package test;
      pub struct Rectangle {
        width: i32,
        height: i32,
        color: i32
      }
      pub fn main(none) -> i32 {
        let r: Rectangle = Rectangle { width: 42, height: 20, color: 255 };
        match r {
          Rectangle { width: w, height: _, color: _ } => { return w; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # If-let struct patterns

  Scenario: If-let with struct pattern
    Given the following Asthra code:
      """
      package test;
      pub struct Config {
        value: i32,
        enabled: bool
      }
      pub fn main(none) -> i32 {
        let c: Config = Config { value: 42, enabled: true };
        if let Config { value: v, enabled: _ } = c {
          return v;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Nested struct patterns
  Scenario: Match nested struct patterns
    Given the following Asthra code:
      """
      package test;
      pub struct Inner {
        value: i32
      }
      pub struct Outer {
        inner: Inner,
        count: i32
      }
      pub fn main(none) -> i32 {
        let i: Inner = Inner { value: 40 };
        let o: Outer = Outer { inner: i, count: 2 };
        match o {
          Outer { inner: Inner { value: v }, count: c } => { return v + c; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Generic struct patterns

  Scenario: Match generic struct pattern
    Given the following Asthra code:
      """
      package test;
      pub struct Container<T> {
        value: T
      }
      pub fn main(none) -> i32 {
        let c: Container<i32> = Container<i32> { value: 42 };
        match c {
          Container<i32> { value: v } => { return v; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Complex field bindings

  Scenario: Match struct with same field and binding names
    Given the following Asthra code:
      """
      package test;
      pub struct Data {
        value: i32,
        flag: bool
      }
      pub fn main(none) -> i32 {
        let d: Data = Data { value: 42, flag: false };
        match d {
          Data { value: value, flag: flag } => { 
            if flag {
              return 0;
            } else {
              return value;
            }
          }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Match struct with renamed bindings
    Given the following Asthra code:
      """
      package test;
      pub struct Coordinate {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let coord: Coordinate = Coordinate { x: 20, y: 22 };
        match coord {
          Coordinate { x: horizontal, y: vertical } => { return horizontal + vertical; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Multiple patterns

  Scenario: Match multiple struct patterns
    Given the following Asthra code:
      """
      package test;
      pub struct Status {
        code: i32,
        active: bool
      }
      pub fn main(none) -> i32 {
        let s: Status = Status { code: 42, active: true };
        match s {
          Status { code: 0, active: _ } => { return 0; }
          Status { code: c, active: true } => { return c; }
          Status { code: _, active: false } => { return -1; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Struct patterns in functions

  Scenario: Struct pattern in function parameter
    Given the following Asthra code:
      """
      package test;
      pub struct Pair {
        first: i32,
        second: i32
      }
      pub fn get_first(p: Pair) -> i32 {
        match p {
          Pair { first: f, second: _ } => { return f; }
        }
      }
      pub fn main(none) -> i32 {
        let pair: Pair = Pair { first: 42, second: 100 };
        return get_first(pair);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Empty struct patterns

  Scenario: Match empty struct
    Given the following Asthra code:
      """
      package test;
      pub struct Empty {
        none
      }
      pub fn main(none) -> i32 {
        let e: Empty = Empty { none };
        match e {
          Empty { none } => { return 42; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Combined patterns

  Scenario: Match struct with tuple fields
    Given the following Asthra code:
      """
      package test;
      pub struct Complex {
        pair: (i32, i32),
        single: i32
      }
      pub fn main(none) -> i32 {
        let c: Complex = Complex { pair: (20, 22), single: 0 };
        match c {
          Complex { pair: (a, b), single: _ } => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern matching expressions

  Scenario: Struct pattern in match expression
    Given the following Asthra code:
      """
      package test;
      pub struct Result {
        value: i32,
        success: bool
      }
      pub fn compute(none) -> Result {
        return Result { value: 42, success: true };
      }
      pub fn main(none) -> i32 {
        match compute(none) {
          Result { value: v, success: true } => { return v; }
          Result { value: _, success: false } => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases

  Scenario: Match struct with single field
    Given the following Asthra code:
      """
      package test;
      pub struct Wrapper {
        data: i32
      }
      pub fn main(none) -> i32 {
        let w: Wrapper = Wrapper { data: 42 };
        match w {
          Wrapper { data: d } => { return d; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Nested if-let struct patterns
    Given the following Asthra code:
      """
      package test;
      pub struct First {
        value: i32
      }
      pub struct Second {
        data: i32
      }
      pub fn main(none) -> i32 {
        let f: First = First { value: 42 };
        if let First { value: v } = f {
          let s: Second = Second { data: v };
          if let Second { data: d } = s {
            return d;
          } else {
            return 0;
          }
        } else {
          return -1;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42