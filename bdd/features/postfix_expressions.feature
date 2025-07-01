Feature: Postfix Expressions
  As a developer using Asthra
  I want to use postfix expressions for function calls, field access, and indexing
  So that I can work with functions, structs, arrays, and tuples effectively

  Background:
    Given a new compilation context

  # Function call tests
  Scenario: Basic function call with no arguments
    Given the following Asthra code:
      """
      package test;
      pub fn greet(none) -> i32 {
        return 42;
      }
      pub fn main(none) -> i32 {
        return greet(none);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Function call with single argument
    Given the following Asthra code:
      """
      package test;
      pub fn double(x: i32) -> i32 {
        return x * 2;
      }
      pub fn main(none) -> i32 {
        return double(21);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Function call with multiple arguments
    Given the following Asthra code:
      """
      package test;
      pub fn add(a: i32, b: i32, c: i32) -> i32 {
        return a + b + c;
      }
      pub fn main(none) -> i32 {
        return add(10, 20, 12);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Nested function calls
    Given the following Asthra code:
      """
      package test;
      pub fn add(a: i32, b: i32) -> i32 {
        return a + b;
      }
      pub fn double(x: i32) -> i32 {
        return x * 2;
      }
      pub fn main(none) -> i32 {
        return add(double(10), double(11));
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Function call with expression arguments
    Given the following Asthra code:
      """
      package test;
      pub fn calculate(x: i32, y: i32) -> i32 {
        return x - y;
      }
      pub fn main(none) -> i32 {
        let a: i32 = 50;
        let b: i32 = 8;
        return calculate(a, b);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Struct field access tests

  Scenario: Basic struct field access
    Given the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let p: Point = Point { x: 42, y: 100 };
        return p.x;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Nested struct field access
    Given the following Asthra code:
      """
      package test;
      pub struct Inner {
        value: i32
      }
      pub struct Outer {
        inner: Inner
      }
      pub fn main(none) -> i32 {
        let i: Inner = Inner { value: 42 };
        let o: Outer = Outer { inner: i };
        return o.inner.value;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Struct field access with mutable fields
    Given the following Asthra code:
      """
      package test;
      pub struct Counter {
        count: i32
      }
      pub fn main(none) -> i32 {
        let mut c: Counter = Counter { count: 40 };
        c.count = 42;
        return c.count;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Tuple access tests

  Scenario: Basic tuple element access
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32) = (42, 100);
        return t.0;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Tuple access with larger index
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: (i32, i32, i32) = (10, 20, 42);
        return t.2;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Nested tuple access
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let t: ((i32, i32), i32) = ((42, 50), 100);
        return t.0.0;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Array indexing tests

  Scenario: Basic array indexing
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [5]i32 = [10, 20, 30, 40, 42];
        return arr[4];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Array indexing with expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [5]i32 = [10, 20, 30, 40, 42];
        let idx: i32 = 2 + 2;
        return arr[idx];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Multi-dimensional array indexing
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let matrix: [3][3]i32 = [[1, 2, 3], [4, 5, 6], [7, 8, 42]];
        return matrix[2][2];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Slice operations tests

  Scenario: Basic slice indexing
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 42];
        return arr[4];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Slice range from start
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 50];
        let slice: []i32 = arr[2:];
        return slice[0];  // Should be 30
      }
      """
    When I compile and run the code
    Then the exit code should be 30

  Scenario: Slice range to end
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 50];
        let slice: []i32 = arr[:3];
        return slice[2];  // Should be 30
      }
      """
    When I compile and run the code
    Then the exit code should be 30

  Scenario: Slice with start and end
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 42, 50];
        let slice: []i32 = arr[2:4];
        return slice[1];  // Should be 42
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Full slice copy
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 42];
        let copy: []i32 = arr[:];
        return copy[4];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Length operation tests

  Scenario: Array length operation
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [42]i32 = [0; 42];
        return len(arr);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Slice length operation
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let slice: []i32 = [1, 2, 3, 4, 5];
        let sub: []i32 = slice[1:4];
        return len(sub);  // Should be 3
      }
      """
    When I compile and run the code
    Then the exit code should be 3

  Scenario: String length operation
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Hello, World!";
        return len(s);  // Should be 13
      }
      """
    When I compile and run the code
    Then the exit code should be 13

  # Method calls tests

  Scenario: Basic method call
    Given the following Asthra code:
      """
      package test;
      pub struct Counter {
        value: i32
      }
      impl Counter {
        pub fn get(self) -> i32 {
          return self.value;
        }
      }
      pub fn main(none) -> i32 {
        let c: Counter = Counter { value: 42 };
        return c.get(none);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Method call with arguments
    Given the following Asthra code:
      """
      package test;
      pub struct Calculator {
        base: i32
      }
      impl Calculator {
        pub fn add(self, x: i32) -> i32 {
          return self.base + x;
        }
      }
      pub fn main(none) -> i32 {
        let calc: Calculator = Calculator { base: 30 };
        return calc.add(12);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Associated function calls tests

  Scenario: Associated function call
    Given the following Asthra code:
      """
      package test;
      pub struct Number {
        value: i32
      }
      impl Number {
        pub fn new(v: i32) -> Number {
          return Number { value: v };
        }
      }
      pub fn main(none) -> i32 {
        let n: Number = Number::new(42);
        return n.value;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Generic associated function call
    Given the following Asthra code:
      """
      package test;
      pub struct Container<T> {
        value: T
      }
      impl Container<i32> {
        pub fn create(v: i32) -> Container<i32> {
          return Container<i32> { value: v };
        }
      }
      pub fn main(none) -> i32 {
        let c: Container<i32> = Container<i32>::create(42);
        return c.value;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Complex chaining tests
  Scenario: Chained field and method access
    Given the following Asthra code:
      """
      package test;
      pub struct Inner {
        value: i32
      }
      pub struct Outer {
        inner: Inner
      }
      impl Inner {
        pub fn double(self) -> i32 {
          return self.value * 2;
        }
      }
      pub fn main(none) -> i32 {
        let i: Inner = Inner { value: 21 };
        let o: Outer = Outer { inner: i };
        return o.inner.double();
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Function call on indexed element
    Given the following Asthra code:
      """
      package test;
      pub fn triple(x: i32) -> i32 {
        return x * 3;
      }
      pub fn main(none) -> i32 {
        let arr: [3]i32 = [10, 14, 30];
        return triple(arr[1]);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Array of structs with field access
    Given the following Asthra code:
      """
      package test;
      pub struct Point {
        x: i32,
        y: i32
      }
      pub fn main(none) -> i32 {
        let points: [3]Point = [
          Point { x: 10, y: 20 },
          Point { x: 42, y: 50 },
          Point { x: 30, y: 40 }
        ];
        return points[1].x;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases

  Scenario: Empty array literal with none
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [none];
        return len(arr);  // Should be 0
      }
      """
    When I compile and run the code
    Then the exit code should be 0

  Scenario: Complex expression in array index
    Given the following Asthra code:
      """
      package test;
      pub fn get_index(none) -> i32 {
        return 2;
      }
      pub fn main(none) -> i32 {
        let arr: [5]i32 = [10, 20, 42, 30, 40];
        return arr[get_index(none)];
      }
      """
    When I compile and run the code
    Then the exit code should be 42