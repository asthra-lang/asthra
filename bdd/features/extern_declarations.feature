Feature: Extern Declarations
  As an Asthra developer
  I want to declare external functions for FFI
  So that I can interoperate with C libraries

  Background:
    Given I have a clean test environment
    And I am testing "extern_declarations"

  # Basic extern function declarations
  Scenario: Simple extern function declaration
    Given the following Asthra code:
      """
      package test;

      extern fn printf(format: *const char) -> i32;

      pub fn main(none) -> void {
          log("Extern declaration works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Extern declaration works"

  Scenario: Extern function with multiple parameters
    Given the following Asthra code:
      """
      package test;

      extern fn strcmp(s1: *const char, s2: *const char) -> i32;

      pub fn main(none) -> void {
          log("Multiple param extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Multiple param extern works"

  Scenario: Extern function with no parameters
    Given the following Asthra code:
      """
      package test;

      extern fn get_errno(none) -> i32;

      pub fn main(none) -> void {
          log("No param extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "No param extern works"

  Scenario: Extern function returning void
    Given the following Asthra code:
      """
      package test;

      extern fn exit(status: i32) -> void;

      pub fn main(none) -> void {
          log("Void return extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Void return extern works"

  # Extern with library specification
  Scenario: Extern function with library name
    Given the following Asthra code:
      """
      package test;

      extern "C" fn malloc(size: usize) -> *mut void;

      pub fn main(none) -> void {
          log("Library spec extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Library spec extern works"

  # FFI annotations
  Scenario: Extern function with transfer_full annotation
    Given the following Asthra code:
      """
      package test;

      extern fn strdup(#[transfer_full] s: *const char) -> *mut char;

      pub fn main(none) -> void {
          log("Transfer full annotation works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Transfer full annotation works"

  Scenario: Extern function with transfer_none annotation
    Given the following Asthra code:
      """
      package test;

      extern fn strlen(#[transfer_none] s: *const char) -> usize;

      pub fn main(none) -> void {
          log("Transfer none annotation works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Transfer none annotation works"

  Scenario: Extern function with borrowed annotation
    Given the following Asthra code:
      """
      package test;

      extern fn fopen(#[borrowed] filename: *const char, #[borrowed] mode: *const char) -> *mut void;

      pub fn main(none) -> void {
          log("Borrowed annotation works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Borrowed annotation works"

  Scenario: Return type with FFI annotation
    Given the following Asthra code:
      """
      package test;

      extern fn create_buffer(size: usize) -> #[transfer_full] *mut u8;

      pub fn main(none) -> void {
          log("Return annotation works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Return annotation works"

  # Various parameter types
  Scenario: Extern function with pointer types
    Given the following Asthra code:
      """
      package test;

      extern fn memcpy(dest: *mut void, src: *const void, n: usize) -> *mut void;

      pub fn main(none) -> void {
          log("Pointer types extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Pointer types extern works"

  Scenario: Extern function with various numeric types
    Given the following Asthra code:
      """
      package test;

      extern fn process_numbers(a: i8, b: u16, c: i32, d: u64, e: f32, f: f64) -> i64;

      pub fn main(none) -> void {
          log("Numeric types extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Numeric types extern works"

  # Visibility modifiers
  Scenario: Private extern function
    Given the following Asthra code:
      """
      package test;

      priv extern fn internal_helper(x: i32) -> i32;

      pub fn main(none) -> void {
          log("Private extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Private extern works"

  Scenario: Public extern function
    Given the following Asthra code:
      """
      package test;

      pub extern fn public_api_call(data: *const u8, len: usize) -> i32;

      pub fn main(none) -> void {
          log("Public extern works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Public extern works"

  # Error scenarios
  @wip
  Scenario: Extern function without return type
    Given the following Asthra code:
      """
      package test;

      extern fn bad_extern(x: i32);

      pub fn main(none) -> void {
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "return type"

  Scenario: Extern function with invalid parameter
    Given the following Asthra code:
      """
      package test;

      extern fn bad_param(x) -> i32;

      pub fn main(none) -> void {
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Multiple FFI annotations on same parameter
    Given the following Asthra code:
      """
      package test;

      extern fn conflicting(#[transfer_full] #[borrowed] ptr: *mut void) -> void;

      pub fn main(none) -> void {
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "annotation"

  # Complex scenarios
  Scenario: Multiple extern declarations
    Given the following Asthra code:
      """
      package test;

      extern fn open(path: *const char, flags: i32) -> i32;
      extern fn read(fd: i32, buf: *mut void, count: usize) -> isize;
      extern fn close(fd: i32) -> i32;

      pub fn main(none) -> void {
          log("Multiple externs work");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Multiple externs work"

  Scenario: Extern function with mixed annotations
    Given the following Asthra code:
      """
      package test;

      extern fn process_data(
          #[borrowed] input: *const u8,
          input_len: usize,
          #[transfer_full] output: *mut u8,
          output_len: usize
      ) -> i32;

      pub fn main(none) -> void {
          log("Mixed annotations work");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Mixed annotations work"