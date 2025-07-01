Feature: Enum Patterns
  As a developer using Asthra
  I want to use pattern matching with enum variants
  So that I can handle different cases of enum values effectively

  Background:
    Given a new compilation context

  # Basic enum variant matching
  @wip
  Scenario: Match enum variant without data
    Given the following Asthra code:
      """
      package test;
      pub enum Status {
        Active,
        Inactive
      }
      pub fn main(none) -> i32 {
        let s: Status = Status.Active;
        match s {
          Status.Active => { return 1; }
          Status.Inactive => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 1

  @wip
  Scenario: Match enum variant with single value
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

  Scenario: Match enum variant with tuple data
    Given the following Asthra code:
      """
      package test;
      pub enum Message {
        Point(i32, i32),
        Quit
      }
      pub fn main(none) -> i32 {
        let msg: Message = Message.Point(10, 32);
        match msg {
          Message.Point(x, y) => { return x + y; }
          Message.Quit => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Nested pattern matching
  Scenario: Match nested enum patterns
    Given the following Asthra code:
      """
      package test;
      pub enum Inner {
        Value(i32),
        Empty
      }
      pub enum Outer {
        Contains(Inner),
        Nothing
      }
      pub fn main(none) -> i32 {
        let inner: Inner = Inner.Value(42);
        let outer: Outer = Outer.Contains(inner);
        match outer {
          Outer.Contains(Inner.Value(n)) => { return n; }
          Outer.Contains(Inner.Empty) => { return 0; }
          Outer.Nothing => { return -1; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # If-let pattern matching
  @wip
  Scenario: If-let with enum variant
    Given the following Asthra code:
      """
      package test;
      pub enum Option {
        Some(i32),
        None
      }
      pub fn main(none) -> i32 {
        let opt: Option = Option.Some(42);
        if let Option.Some(value) = opt {
          return value;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: If-let with enum variant returning false case
    Given the following Asthra code:
      """
      package test;
      pub enum Option {
        Some(i32),
        None
      }
      pub fn main(none) -> i32 {
        let opt: Option = Option.None;
        if let Option.Some(value) = opt {
          return value;
        } else {
          return 42;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern with wildcards
  @wip
  Scenario: Match with wildcard pattern
    Given the following Asthra code:
      """
      package test;
      pub enum Color {
        Red,
        Green,
        Blue,
        RGB(i32, i32, i32)
      }
      pub fn main(none) -> i32 {
        let c: Color = Color.RGB(42, 0, 0);
        match c {
          Color.RGB(r, _, _) => { return r; }
          Color.Red => { return 255; }
          Color.Green => { return 0; }
          Color.Blue => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Multiple patterns in single match arm
  @wip
  Scenario: Match multiple patterns
    Given the following Asthra code:
      """
      package test;
      pub enum State {
        Starting,
        Running,
        Stopping,
        Stopped
      }
      pub fn main(none) -> i32 {
        let s: State = State.Running;
        match s {
          State.Starting => { return 1; }
          State.Running => { return 42; }
          State.Stopping => { return 2; }
          State.Stopped => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Generic enum patterns
  Scenario: Match generic enum variant
    Given the following Asthra code:
      """
      package test;
      pub enum Result<T, E> {
        Ok(T),
        Err(E)
      }
      pub fn main(none) -> i32 {
        let r: Result<i32, i32> = Result<i32, i32>.Ok(42);
        match r {
          Result.Ok(value) => { return value; }
          Result.Err(error) => { return error; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Enum construction
  @wip
  Scenario: Construct enum variant without data
    Given the following Asthra code:
      """
      package test;
      pub enum Flag {
        True,
        False
      }
      pub fn main(none) -> i32 {
        let f: Flag = Flag.True;
        match f {
          Flag.True => { return 42; }
          Flag.False => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Construct enum variant with data
    Given the following Asthra code:
      """
      package test;
      pub enum Container {
        Empty,
        Value(i32)
      }
      pub fn main(none) -> i32 {
        let c: Container = Container.Value(42);
        match c {
          Container.Empty => { return 0; }
          Container.Value(n) => { return n; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Complex patterns
  @wip
  Scenario: Match with variable binding
    Given the following Asthra code:
      """
      package test;
      pub enum Command {
        Move(i32, i32),
        Stop
      }
      pub fn main(none) -> i32 {
        let cmd: Command = Command.Move(20, 22);
        match cmd {
          Command.Move(dx, dy) => {
            let sum: i32 = dx + dy;
            return sum;
          }
          Command.Stop => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Nested match expressions
    Given the following Asthra code:
      """
      package test;
      pub enum Outer {
        A(i32),
        B(i32)
      }
      pub enum Inner {
        X(Outer),
        Y
      }
      pub fn main(none) -> i32 {
        let outer: Outer = Outer.A(42);
        let inner: Inner = Inner.X(outer);
        match inner {
          Inner.X(o) => {
            match o {
              Outer.A(n) => { return n; }
              Outer.B(n) => { return n; }
            }
          }
          Inner.Y => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases
  @wip
  Scenario: Empty enum with none content
    Given the following Asthra code:
      """
      package test;
      pub enum Empty {
        none
      }
      pub fn main(none) -> i32 {
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Match with all patterns covered
    Given the following Asthra code:
      """
      package test;
      pub enum Binary {
        Zero,
        One
      }
      pub fn main(none) -> i32 {
        let b: Binary = Binary.One;
        match b {
          Binary.Zero => { return 0; }
          Binary.One => { return 42; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern matching in functions
  Scenario: Pattern match as function parameter
    Given the following Asthra code:
      """
      package test;
      pub enum MyOption {
        Some(i32),
        None
      }
      pub fn unwrap(opt: MyOption) -> i32 {
        match opt {
          MyOption.Some(value) => { return value; }
          MyOption.None => { return 0; }
        }
      }
      pub fn main(none) -> i32 {
        let o: MyOption = MyOption.Some(42);
        return unwrap(o);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Pattern matching with expressions
  @wip
  Scenario: Match expression result
    Given the following Asthra code:
      """
      package test;
      pub enum Result {
        Success(i32),
        Failure
      }
      pub fn compute(none) -> Result {
        return Result.Success(42);
      }
      pub fn main(none) -> i32 {
        match compute(none) {
          Result.Success(n) => { return n; }
          Result.Failure => { return 0; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42