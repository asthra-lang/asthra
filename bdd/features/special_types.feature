Feature: Special Types
  As an Asthra developer
  I want to use special types like Never, unit type, and size types
  So that I can express advanced type semantics in my programs

  Background:
    Given the Asthra compiler is available

  Scenario: Unit type as void return
    Given I have a file "unit_type.asthra" with:
      """
      package main;
      
      pub fn do_nothing(none) -> void {
          return ();
      }
      
      pub fn main(none) -> void {
          do_nothing();
          log("Unit type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Unit type works"
    And the exit code should be 0

  @wip
  Scenario: Unit type in expressions
    Given I have a file "unit_expressions.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let unit: void = ();
          let result: void = if true { () } else { () };
          log("Unit in expressions works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Unit in expressions works"
    And the exit code should be 0

  @wip
  Scenario: Never type for non-returning functions
    Given I have a file "never_type.asthra" with:
      """
      package main;
      
      pub fn panic(msg: string) -> Never {
          log(msg);
          exit(1);
      }
      
      pub fn main(none) -> void {
          let x: i32 = if false {
              panic("This won't happen");
          } else {
              42
          };
          log("Never type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Never type works"
    And the exit code should be 0

  Scenario: Size types - usize
    Given I have a file "usize_type.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let array_size: usize = 100;
          let index: usize = 0;
          let count: usize = array_size;
          log("usize type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "usize type works"
    And the exit code should be 0

  Scenario: Size types - isize
    Given I have a file "isize_type.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let offset: isize = -10;
          let position: isize = 50;
          let delta: isize = offset + position;
          log("isize type works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "isize type works"
    And the exit code should be 0

  Scenario: Size types in array operations
    Given I have a file "size_array_ops.asthra" with:
      """
      package main;
      
      pub fn get_array_length<T>(arr: []T) -> usize {
          return arr.len();
      }
      
      pub fn main(none) -> void {
          let numbers: []i32 = [1, 2, 3, 4, 5];
          let len: usize = get_array_length(numbers);
          log("Size types in arrays work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Size types in arrays work"
    And the exit code should be 0

  Scenario: SizeOf compile-time expression
    Given I have a file "sizeof_expr.asthra" with:
      """
      package main;
      
      pub const I32_SIZE: usize = sizeof(i32);
      pub const BOOL_SIZE: usize = sizeof(bool);
      
      pub struct Point {
          x: i32,
          y: i32
      }
      
      pub const POINT_SIZE: usize = sizeof(Point);
      
      pub fn main(none) -> void {
          log("sizeof expressions work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "sizeof expressions work"
    And the exit code should be 0

  @wip
  Scenario: Never type in match expressions
    Given I have a file "never_match.asthra" with:
      """
      package main;
      
      pub enum Status {
          Ok,
          Error
      }
      
      pub fn handle_error(none) -> Never {
          log("Fatal error");
          exit(1);
      }
      
      pub fn process(s: Status) -> i32 {
          return match s {
              Status::Ok => 42,
              Status::Error => handle_error()
          };
      }
      
      pub fn main(none) -> void {
          let result: i32 = process(Status::Ok);
          log("Never in match works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Never in match works"
    And the exit code should be 0

  Scenario: Unit type in struct fields
    Given I have a file "unit_struct_field.asthra" with:
      """
      package main;
      
      pub struct Marker {
          tag: string,
          placeholder: void
      }
      
      pub fn main(none) -> void {
          let m: Marker = Marker { tag: "test", placeholder: () };
          log("Unit in struct works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Unit in struct works"
    And the exit code should be 0

  Scenario: Never type cannot be instantiated
    Given I have a file "never_instantiate.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: Never = panic("error");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "Never type cannot be instantiated"

  Scenario: Invalid sizeof usage
    Given I have a file "invalid_sizeof.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = 42;
          let size: usize = sizeof(x);
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "sizeof expects a type"

  @wip
  Scenario: Unit type comparison
    Given I have a file "unit_comparison.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let u1: void = ();
          let u2: void = ();
          if u1 == u2 {
              log("Unit values are equal");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Unit values are equal"
    And the exit code should be 0

  @wip
  Scenario: Platform-specific size types
    Given I have a file "platform_sizes.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let ptr_size: usize = sizeof(*const i32);
          let size_size: usize = sizeof(usize);
          log("Platform size types work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Platform size types work"
    And the exit code should be 0