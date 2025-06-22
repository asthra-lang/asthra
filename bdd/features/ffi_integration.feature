Feature: Foreign Function Interface (FFI)
  As an Asthra developer
  I want to seamlessly interact with C libraries
  So that I can leverage existing code and libraries

  Background:
    Given the Asthra runtime is initialized
    And FFI support is enabled

  Scenario: Call a simple C function
    Given I have a C library "mathlib" with function:
      """
      int add(int a, int b) {
          return a + b;
      }
      """
    And I have Asthra code:
      """
      package math_test;
      
      @ffi("mathlib")
      extern fn add(a: i32, b: i32) -> i32;
      
      pub fn test_add(none) -> i32 {
          return add(5, 3);
      }
      """
    When I run the test_add function
    Then the result should be 8

  Scenario: Handle C string conversions
    Given I have a C function that accepts strings
    When I pass an Asthra string to the C function
    Then the string should be correctly marshaled
    And memory should be properly managed

  Scenario: Error handling across FFI boundary
    Given I have a C function that can fail
    When the C function returns an error code
    Then Asthra should handle the error gracefully
    And provide meaningful error information