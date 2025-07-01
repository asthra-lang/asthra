Feature: Pattern Bindings
  As a developer using Asthra
  I want to bind variables through pattern matching
  So that I can extract and use values from complex data structures

  Background:
    Given a new compilation context

  # Simple identifier patterns
  @wip
  Scenario: Bind variable in match arm
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let value: i32 = 42;
        match value {
          x => { return x; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Bind multiple variables in match
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pair: (i32, i32) = (20, 22);
        match pair {
          (a, b) => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Wildcard patterns
  @wip
  Scenario: Use wildcard to ignore values
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let triple: (i32, i32, i32) = (42, 100, 200);
        match triple {
          (x, _, _) => { return x; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Nested pattern bindings
  @wip
  Scenario: Bind variables in nested patterns
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let nested: ((i32, i32), i32) = ((20, 22), 0);
        match nested {
          ((x, y), _) => { return x + y; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Struct pattern bindings
  @wip
  Scenario: Bind struct fields to variables
    Given the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let p: Point = Point { x: 20, y: 22 };
        match p {
          Point { x: a, y: b } => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Mix bindings and wildcards in struct pattern
    Given the following Asthra code:
      """
      package test;
      pub struct Data {
        value: i32,
        flag: bool,
        count: i32
      }
      pub fn main(none) -> i32 {
        let d: Data = Data { value: 42, flag: true, count: 100 };
        match d {
          Data { value: v, flag: _, count: _ } => { return v; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Enum pattern bindings
  @wip
  Scenario: Bind enum variant data
    Given the following Asthra code:
      """
      package test;
      pub enum Result {
        Ok(i32),
        Err(i32)
      }
      pub fn main(none) -> i32 {
        let r: Result = Result.Ok(42);
        match r {
          Result.Ok(value) => { return value; }
          Result.Err(code) => { return code; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Bind multiple values from enum variant
    Given the following Asthra code:
      """
      package test;
      pub enum Message {
        Move(i32, i32),
        Quit(none)
      }
      pub fn main(none) -> i32 {
        let msg: Message = Message.Move(20, 22);
        match msg {
          Message.Move(dx, dy) => { return dx + dy; }
          Message.Quit(none) => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # If-let pattern bindings
  @wip
  Scenario: Bind variable in if-let
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let value: i32 = 42;
        if let x = value {
          return x;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Bind tuple elements in if-let
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pair: (i32, i32) = (40, 2);
        if let (a, b) = pair {
          return a + b;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Shadowing in patterns
  @wip
  Scenario: Shadow existing variables in pattern
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let x: i32 = 10;
        let pair: (i32, i32) = (20, 22);
        match pair {
          (x, y) => { return x + y; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Complex nested bindings
  @wip
  Scenario: Bind in deeply nested structure
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

  # Pattern bindings with literals
  @wip
  Scenario: Mix literal matching and bindings
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let triple: (i32, i32, i32) = (1, 42, 3);
        match triple {
          (1, x, 3) => { return x; }
          (_, x, _) => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Multiple bindings in single pattern
  @wip
  Scenario: Bind all elements of a tuple
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let quad: (i32, i32, i32, i32) = (10, 11, 12, 9);
        match quad {
          (a, b, c, d) => { return a + b + c + d; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern bindings in variable declarations
  @wip
  Scenario: Destructure in let binding
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pair: (i32, i32) = (20, 22);
        let (x, y): (i32, i32) = pair;
        return x + y;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mutable bindings in patterns
  @wip
  Scenario: Bind mutable variables in pattern
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut pair: (i32, i32) = (20, 20);
        match pair {
          (a, b) => {
            pair = (a + 1, b + 1);
          }
        }
        match pair {
          (x, y) => { return x + y; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern bindings with Option type
  @wip
  Scenario: Bind Option value
    Given the following Asthra code:
      """
      package test;
      pub enum Option<T> {
        Some(T),
        None(none)
      }
      pub fn main(none) -> i32 {
        let opt: Option<i32> = Option<i32>.Some(42);
        match opt {
          Option.Some(value) => { return value; }
          Option.None(none) => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern bindings with Result type
  @wip
  Scenario: Bind Result values
    Given the following Asthra code:
      """
      package test;
      pub enum Result<T, E> {
        Ok(T),
        Err(E)
      }
      pub fn main(none) -> i32 {
        let res: Result<i32, i32> = Result<i32, i32>.Ok(42);
        match res {
          Result.Ok(val) => { return val; }
          Result.Err(err) => { return err; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Binding with type annotations
  @wip
  Scenario: Pattern binding preserves types
    Given the following Asthra code:
      """
      package test;
      pub fn process(p: (i32, bool)) -> i32 {
        match p {
          (num, true) => { return num; }
          (num, false) => { return -num; }
        }
      }
      pub fn main(none) -> i32 {
        return process((42, true));
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Complex enum with struct patterns
  @wip
  Scenario: Bind from enum containing structs
    Given the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub enum Shape {
        Circle(i32),
        Rectangle(Point)
      }
      pub fn main(none) -> i32 {
        let p: Point = Point { x: 20, y: 22 };
        let shape: Shape = Shape.Rectangle(p);
        match shape {
          Shape.Circle(radius) => { return radius; }
          Shape.Rectangle(Point { x: a, y: b }) => { return a + b; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42