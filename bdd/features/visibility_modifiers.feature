Feature: Visibility Modifiers
  As an Asthra developer
  I want to control the visibility of my code elements
  So that I can properly encapsulate implementation details

  Background:
    Given the Asthra compiler is available

  Scenario: Public function declaration
    Given I have a file "public_function.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          log("Public function");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Public function"
    And the exit code should be 0

  Scenario: Private function declaration
    Given I have a file "private_function.asthra" with:
      """
      package main;
      
      priv fn helper(none) -> void {
          log("Private helper");
          return ();
      }
      
      pub fn main(none) -> void {
          helper();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Private helper"
    And the exit code should be 0

  @wip
  Scenario: Public struct declaration
    Given I have a file "public_struct.asthra" with:
      """
      package main;
      
      pub struct Point {
          x: i32,
          y: i32
      }
      
      pub fn main(none) -> void {
          let p: Point = Point { x: 10, y: 20 };
          log("Public struct works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Public struct works"
    And the exit code should be 0

  @wip
  Scenario: Private struct declaration
    Given I have a file "private_struct.asthra" with:
      """
      package main;
      
      priv struct InternalData {
          value: i32
      }
      
      pub fn main(none) -> void {
          let data: InternalData = InternalData { value: 42 };
          log("Private struct works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Private struct works"
    And the exit code should be 0

  @wip
  Scenario: Public enum declaration
    Given I have a file "public_enum.asthra" with:
      """
      package main;
      
      pub enum Status {
          Ok,
          Error(i32)
      }
      
      pub fn main(none) -> void {
          let s: Status = Status::Ok;
          log("Public enum works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Public enum works"
    And the exit code should be 0

  @wip
  Scenario: Private enum declaration
    Given I have a file "private_enum.asthra" with:
      """
      package main;
      
      priv enum InternalState {
          Ready,
          Busy,
          Done
      }
      
      pub fn main(none) -> void {
          let state: InternalState = InternalState::Ready;
          log("Private enum works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Private enum works"
    And the exit code should be 0

  @wip
  Scenario: Struct with mixed field visibility
    Given I have a file "mixed_field_visibility.asthra" with:
      """
      package main;
      
      pub struct User {
          pub name: string,
          priv id: i32
      }
      
      pub fn main(none) -> void {
          let u: User = User { name: "Alice", id: 123 };
          log("Mixed visibility works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Mixed visibility works"
    And the exit code should be 0

  @wip
  Scenario: Missing visibility modifier on function
    Given I have a file "no_visibility_function.asthra" with:
      """
      package main;
      
      fn helper(none) -> void {
          return ();
      }
      
      pub fn main(none) -> void {
          helper();
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected visibility modifier"

  @wip
  Scenario: Missing visibility modifier on struct
    Given I have a file "no_visibility_struct.asthra" with:
      """
      package main;
      
      struct Point {
          x: i32,
          y: i32
      }
      
      pub fn main(none) -> void {
          log("No visibility");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected visibility modifier"

  @wip
  Scenario: Missing visibility modifier on enum
    Given I have a file "no_visibility_enum.asthra" with:
      """
      package main;
      
      enum Status {
          Ok,
          Error
      }
      
      pub fn main(none) -> void {
          log("No visibility");
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected visibility modifier"

  Scenario: Public constant declaration
    Given I have a file "public_const.asthra" with:
      """
      package main;
      
      pub const MAX_SIZE: i32 = 100;
      
      pub fn main(none) -> void {
          log("Public const works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Public const works"
    And the exit code should be 0

  Scenario: Private constant declaration
    Given I have a file "private_const.asthra" with:
      """
      package main;
      
      priv const INTERNAL_BUFFER_SIZE: i32 = 1024;
      
      pub fn main(none) -> void {
          log("Private const works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Private const works"
    And the exit code should be 0