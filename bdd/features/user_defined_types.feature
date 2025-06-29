Feature: User-Defined Types
  As an Asthra developer
  I want to define my own structs and enums
  So that I can create custom data types for my domain

  Background:
    Given the Asthra compiler is available

  Scenario: Simple struct definition
    Given I have a file "simple_struct.asthra" with:
      """
      package main;
      
      pub struct Point {
          x: i32,
          y: i32
      }
      
      pub fn main(none) -> void {
          let p: Point = Point { x: 10, y: 20 };
          log("Simple struct works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Simple struct works"
    And the exit code should be 0

  Scenario: Struct with mixed types
    Given I have a file "mixed_struct.asthra" with:
      """
      package main;
      
      pub struct Person {
          name: string,
          age: u32,
          height: f64,
          is_active: bool
      }
      
      pub fn main(none) -> void {
          let p: Person = Person { 
              name: "Alice", 
              age: 30, 
              height: 1.75, 
              is_active: true 
          };
          log("Mixed type struct works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Mixed type struct works"
    And the exit code should be 0

  @wip
  Scenario: Empty struct
    Given I have a file "empty_struct.asthra" with:
      """
      package main;
      
      pub struct Empty {
          none
      }
      
      pub fn main(none) -> void {
          let e: Empty = Empty {};
          log("Empty struct works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Empty struct works"
    And the exit code should be 0

  Scenario: Nested structs
    Given I have a file "nested_struct.asthra" with:
      """
      package main;
      
      pub struct Point {
          x: i32,
          y: i32
      }
      
      pub struct Rectangle {
          top_left: Point,
          bottom_right: Point
      }
      
      pub fn main(none) -> void {
          let rect: Rectangle = Rectangle {
              top_left: Point { x: 0, y: 0 },
              bottom_right: Point { x: 100, y: 100 }
          };
          log("Nested structs work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Nested structs work"
    And the exit code should be 0

  Scenario: Simple enum definition
    Given I have a file "simple_enum.asthra" with:
      """
      package main;
      
      pub enum Status {
          Active,
          Inactive,
          Pending
      }
      
      pub fn main(none) -> void {
          let s: Status = Status::Active;
          log("Simple enum works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Simple enum works"
    And the exit code should be 0

  @wip
  Scenario: Enum with single type data
    Given I have a file "enum_single_data.asthra" with:
      """
      package main;
      
      pub enum Option {
          Some(i32),
          None
      }
      
      pub fn main(none) -> void {
          let val: Option = Option::Some(42);
          let empty: Option = Option::None;
          log("Enum with data works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Enum with data works"
    And the exit code should be 0

  @wip
  Scenario: Enum with tuple data
    Given I have a file "enum_tuple_data.asthra" with:
      """
      package main;
      
      pub enum Message {
          Move(i32, i32),
          Write(string),
          Color(u8, u8, u8),
          Quit
      }
      
      pub fn main(none) -> void {
          let msg1: Message = Message::Move(10, 20);
          let msg2: Message = Message::Write("Hello");
          let msg3: Message = Message::Color(255, 0, 0);
          let msg4: Message = Message::Quit;
          log("Enum with tuple data works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Enum with tuple data works"
    And the exit code should be 0

  Scenario: Empty enum
    Given I have a file "empty_enum.asthra" with:
      """
      package main;
      
      pub enum Never {
          none
      }
      
      pub fn main(none) -> void {
          log("Empty enum compiles");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Empty enum compiles"
    And the exit code should be 0

  Scenario: Struct with methods
    Given I have a file "struct_methods.asthra" with:
      """
      package main;
      
      pub struct Counter {
          value: i32
      }
      
      impl Counter {
          pub fn new(none) -> Counter {
              return Counter { value: 0 };
          }
          
          pub fn increment(self) -> void {
              self.value = self.value + 1;
              return ();
          }
          
          pub fn get_value(self) -> i32 {
              return self.value;
          }
      }
      
      pub fn main(none) -> void {
          let mut c: Counter = Counter::new();
          c.increment();
          log("Struct methods work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Struct methods work"
    And the exit code should be 0

  Scenario: Struct field visibility
    Given I have a file "field_visibility.asthra" with:
      """
      package main;
      
      pub struct Account {
          pub username: string,
          priv password: string,
          balance: i64
      }
      
      pub fn main(none) -> void {
          let acc: Account = Account {
              username: "alice",
              password: "secret",
              balance: 1000
          };
          log("Field visibility works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Field visibility works"
    And the exit code should be 0

  Scenario: Enum variant visibility
    Given I have a file "variant_visibility.asthra" with:
      """
      package main;
      
      pub enum Response {
          pub Success(string),
          priv InternalError(i32),
          Pending
      }
      
      pub fn main(none) -> void {
          let r: Response = Response::Success("OK");
          log("Variant visibility works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Variant visibility works"
    And the exit code should be 0

  Scenario: Duplicate struct field error
    Given I have a file "duplicate_field.asthra" with:
      """
      package main;
      
      pub struct Point {
          x: i32,
          x: i32
      }
      
      pub fn main(none) -> void {
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "duplicate field"

  Scenario: Duplicate enum variant error
    Given I have a file "duplicate_variant.asthra" with:
      """
      package main;
      
      pub enum Status {
          Active,
          Active
      }
      
      pub fn main(none) -> void {
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "duplicate variant"

  Scenario: Missing struct fields in initialization
    Given I have a file "missing_fields.asthra" with:
      """
      package main;
      
      pub struct Point {
          x: i32,
          y: i32
      }
      
      pub fn main(none) -> void {
          let p: Point = Point { x: 10 };
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "missing field 'y'"