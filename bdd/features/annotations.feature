Feature: Annotations
  As an Asthra developer
  I want to use annotations to mark code with metadata
  So that I can provide hints for AI generation, security, and FFI safety

  Background:
    Given the Asthra compiler is available

  Scenario: Human review annotation on function
    Given I have a file "human_review_function.asthra" with:
      """
      package main;
      
      #[human_review(high)]
      pub fn critical_operation(none) -> void {
          log("Critical operation needs human review");
          return ();
      }
      
      pub fn main(none) -> void {
          critical_operation();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Critical operation needs human review"
    And the exit code should be 0

  Scenario: Multiple human review levels
    Given I have a file "review_levels.asthra" with:
      """
      package main;
      
      #[human_review(low)]
      pub fn simple_task(none) -> void {
          log("Low priority review");
          return ();
      }
      
      #[human_review(medium)]
      pub fn normal_task(none) -> void {
          log("Medium priority review");
          return ();
      }
      
      #[human_review(high)]
      pub fn critical_task(none) -> void {
          log("High priority review");
          return ();
      }
      
      pub fn main(none) -> void {
          simple_task();
          normal_task();
          critical_task();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Low priority review"
    And the output should contain "Medium priority review"
    And the output should contain "High priority review"
    And the exit code should be 0

  Scenario: Security annotation - constant time
    Given I have a file "constant_time.asthra" with:
      """
      package main;
      
      #[constant_time]
      pub fn secure_compare(a: i32, b: i32) -> bool {
          return a == b;
      }
      
      pub fn main(none) -> void {
          let result: bool = secure_compare(42, 42);
          log("Constant time comparison");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Constant time comparison"
    And the exit code should be 0

  Scenario: Security annotation - volatile memory
    Given I have a file "volatile_memory.asthra" with:
      """
      package main;
      
      #[volatile_memory]
      pub fn secure_clear(none) -> void {
          log("Clearing sensitive memory");
          return ();
      }
      
      pub fn main(none) -> void {
          secure_clear();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Clearing sensitive memory"
    And the exit code should be 0

  Scenario: Ownership annotation on struct
    Given I have a file "ownership_struct.asthra" with:
      """
      package main;
      
      #[ownership(gc)]
      pub struct ManagedData {
          value: i32
      }
      
      #[ownership(c)]
      pub struct UnmanagedData {
          ptr: *mut i32
      }
      
      #[ownership(pinned)]
      pub struct PinnedData {
          buffer: [1024]u8
      }
      
      pub fn main(none) -> void {
          log("Ownership annotations work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Ownership annotations work"
    And the exit code should be 0

  Scenario: FFI transfer annotations
    Given I have a file "ffi_transfer.asthra" with:
      """
      package main;
      
      extern "C" fn external_take_ownership(ptr: #[transfer_full] *mut i32) -> void;
      extern "C" fn external_borrow(ptr: #[transfer_none] *const i32) -> void;
      
      pub fn main(none) -> void {
          log("FFI transfer annotations work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "FFI transfer annotations work"
    And the exit code should be 0

  Scenario: Borrowed reference annotation
    Given I have a file "borrowed_ref.asthra" with:
      """
      package main;
      
      extern "C" fn use_borrowed(data: #[borrowed] *const i32) -> void;
      
      pub fn main(none) -> void {
          log("Borrowed reference annotation works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Borrowed reference annotation works"
    And the exit code should be 0

  Scenario: Non-deterministic annotation
    Given I have a file "non_deterministic.asthra" with:
      """
      package main;
      
      #[non_deterministic]
      pub fn random_value(none) -> i32 {
          return 42;
      }
      
      pub fn main(none) -> void {
          let val: i32 = random_value();
          log("Non-deterministic annotation works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Non-deterministic annotation works"
    And the exit code should be 0

  Scenario: Generic semantic annotation
    Given I have a file "generic_annotation.asthra" with:
      """
      package main;
      
      #[deprecated(reason = "Use new_function instead")]
      pub fn old_function(none) -> void {
          log("Deprecated function");
          return ();
      }
      
      #[performance(critical = "true", cache_aligned = "true")]
      pub fn hot_path(none) -> void {
          log("Performance critical");
          return ();
      }
      
      pub fn main(none) -> void {
          old_function();
          hot_path();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Deprecated function"
    And the output should contain "Performance critical"
    And the exit code should be 0

  Scenario: Multiple annotations on same element
    Given I have a file "multiple_annotations.asthra" with:
      """
      package main;
      
      #[human_review(high)]
      #[constant_time]
      #[performance(critical = "true")]
      pub fn secure_critical_operation(none) -> void {
          log("Multiple annotations");
          return ();
      }
      
      pub fn main(none) -> void {
          secure_critical_operation();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Multiple annotations"
    And the exit code should be 0

  Scenario: Invalid human review level
    Given I have a file "invalid_review_level.asthra" with:
      """
      package main;
      
      #[human_review(extreme)]
      pub fn some_function(none) -> void {
          return ();
      }
      
      pub fn main(none) -> void {
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "invalid review level"

  Scenario: Invalid ownership type
    Given I have a file "invalid_ownership.asthra" with:
      """
      package main;
      
      #[ownership(manual)]
      pub struct Data {
          value: i32
      }
      
      pub fn main(none) -> void {
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "invalid ownership type"

  Scenario: Annotation with none parameters
    Given I have a file "annotation_none_params.asthra" with:
      """
      package main;
      
      #[custom(none)]
      pub fn annotated_function(none) -> void {
          log("Annotation with none params");
          return ();
      }
      
      pub fn main(none) -> void {
          annotated_function();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Annotation with none params"
    And the exit code should be 0
