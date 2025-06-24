Feature: Generic Types
  As an Asthra developer
  I want to use generic types
  So that I can write reusable code that works with different types

  Background:
    Given the Asthra compiler is available

  Scenario: Generic struct with single type parameter
    Given I have a file "generic_struct_single.asthra" with:
      """
      package main;
      
      pub struct Box<T> {
          value: T
      }
      
      pub fn main(none) -> void {
          let int_box: Box<i32> = Box { value: 42 };
          let str_box: Box<string> = Box { value: "hello" };
          log("Generic struct works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Generic struct works"
    And the exit code should be 0

  Scenario: Generic struct with multiple type parameters
    Given I have a file "generic_struct_multiple.asthra" with:
      """
      package main;
      
      pub struct Pair<T, U> {
          first: T,
          second: U
      }
      
      pub fn main(none) -> void {
          let p1: Pair<i32, string> = Pair { first: 42, second: "answer" };
          let p2: Pair<bool, f64> = Pair { first: true, second: 3.14 };
          log("Multiple type parameters work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Multiple type parameters work"
    And the exit code should be 0

  Scenario: Generic enum
    Given I have a file "generic_enum.asthra" with:
      """
      package main;
      
      pub enum Option<T> {
          Some(T),
          None
      }
      
      pub fn main(none) -> void {
          let some_int: Option<i32> = Option::Some(42);
          let some_str: Option<string> = Option::Some("hello");
          let none_int: Option<i32> = Option::None;
          log("Generic enum works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Generic enum works"
    And the exit code should be 0

  Scenario: Generic function
    Given I have a file "generic_function.asthra" with:
      """
      package main;
      
      pub fn identity<T>(value: T) -> T {
          return value;
      }
      
      pub fn main(none) -> void {
          let x: i32 = identity(42);
          let s: string = identity("hello");
          log("Generic function works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Generic function works"
    And the exit code should be 0

  Scenario: Generic methods
    Given I have a file "generic_methods.asthra" with:
      """
      package main;
      
      pub struct Container<T> {
          value: T
      }
      
      impl<T> Container<T> {
          pub fn new(val: T) -> Container<T> {
              return Container { value: val };
          }
          
          pub fn get(self) -> T {
              return self.value;
          }
      }
      
      pub fn main(none) -> void {
          let c1: Container<i32> = Container::new(42);
          let c2: Container<string> = Container::new("hello");
          log("Generic methods work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Generic methods work"
    And the exit code should be 0

  Scenario: Nested generic types
    Given I have a file "nested_generics.asthra" with:
      """
      package main;
      
      pub struct Box<T> {
          value: T
      }
      
      pub enum Option<T> {
          Some(T),
          None
      }
      
      pub fn main(none) -> void {
          let nested: Box<Option<i32>> = Box { 
              value: Option::Some(42) 
          };
          log("Nested generics work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Nested generics work"
    And the exit code should be 0

  Scenario: Generic type with arrays
    Given I have a file "generic_arrays.asthra" with:
      """
      package main;
      
      pub struct Vector<T> {
          data: []T
      }
      
      pub fn main(none) -> void {
          let v1: Vector<i32> = Vector { data: [1, 2, 3] };
          let v2: Vector<string> = Vector { data: ["a", "b", "c"] };
          log("Generic arrays work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Generic arrays work"
    And the exit code should be 0

  Scenario: Result type usage
    Given I have a file "result_type.asthra" with:
      """
      package main;
      
      pub fn divide(a: i32, b: i32) -> Result<i32, string> {
          if b == 0 {
              return Result::Err("Division by zero");
          }
          return Result::Ok(a / b);
      }
      
      pub fn main(none) -> void {
          let r1: Result<i32, string> = divide(10, 2);
          let r2: Result<i32, string> = divide(10, 0);
          log("Result type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Result type works"
    And the exit code should be 0

  Scenario: Option type usage
    Given I have a file "option_type.asthra" with:
      """
      package main;
      
      pub fn find_value(key: string) -> Option<i32> {
          if key == "answer" {
              return Option::Some(42);
          }
          return Option::None;
      }
      
      pub fn main(none) -> void {
          let found: Option<i32> = find_value("answer");
          let not_found: Option<i32> = find_value("question");
          log("Option type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Option type works"
    And the exit code should be 0

  Scenario: Missing type parameter error
    Given I have a file "missing_type_param.asthra" with:
      """
      package main;
      
      pub struct Box<T> {
          value: T
      }
      
      pub fn main(none) -> void {
          let b: Box = Box { value: 42 };
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "missing type parameter"

  Scenario: Wrong number of type parameters
    Given I have a file "wrong_type_params.asthra" with:
      """
      package main;
      
      pub struct Pair<T, U> {
          first: T,
          second: U
      }
      
      pub fn main(none) -> void {
          let p: Pair<i32> = Pair { first: 42, second: "hello" };
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "wrong number of type parameters"

  Scenario: Type parameter name conflict
    Given I have a file "type_param_conflict.asthra" with:
      """
      package main;
      
      pub struct Container<T, T> {
          value1: T,
          value2: T
      }
      
      pub fn main(none) -> void {
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "duplicate type parameter"