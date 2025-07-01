Feature: Return statements
  As a developer
  I want to use explicit return statements with expressions
  So that I can control function flow and return values clearly

  Background:
    Given the Asthra compiler is available

  Scenario: Basic return with integer
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_value(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          let result = get_value();
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Return with arithmetic expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn calculate(a: i32, b: i32) -> i32 {
          return a * b + 10;
      }

      pub fn main(none) -> i32 {
          return calculate(3, 4);
      }
      """
    When I compile and run the program
    Then the exit code should be 22

  Scenario: Return with boolean expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn is_positive(value: i32) -> bool {
          return value > 0;
      }

      pub fn main(none) -> i32 {
          let result = is_positive(5);
          if result {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Return void with unit expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn do_nothing(none) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          do_nothing();
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Early return in conditional
    Given I have the following Asthra code:
      """
      package test;

      pub fn check_value(value: i32) -> i32 {
          if value < 0 {
              return -1;
          }
          
          if value == 0 {
              return 0;
          }
          
          return 1;
      }

      pub fn main(none) -> i32 {
          return check_value(-5);
      }
      """
    When I compile and run the program
    Then the exit code should be 255

  Scenario: Return with function call
    Given I have the following Asthra code:
      """
      package test;

      pub fn multiply(a: i32, b: i32) -> i32 {
          return a * b;
      }

      pub fn calculate_area(width: i32, height: i32) -> i32 {
          return multiply(width, height);
      }

      pub fn main(none) -> i32 {
          return calculate_area(6, 7);
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Return with struct construction
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: i32,
          y: i32
      }

      pub fn create_point(x_val: i32, y_val: i32) -> Point {
          return Point { x: x_val, y: y_val };
      }

      pub fn main(none) -> i32 {
          let point = create_point(3, 4);
          return point.x + point.y;
      }
      """
    When I compile and run the program
    Then the exit code should be 7

  Scenario: Return with array access
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_element(array: [3]i32, index: i32) -> i32 {
          return array[index];
      }

      pub fn main(none) -> i32 {
          let data: [3]i32 = [10, 20, 30];
          return get_element(data, 1);
      }
      """
    When I compile and run the program
    Then the exit code should be 20

  Scenario: Return with tuple construction
    Given I have the following Asthra code:
      """
      package test;

      pub fn make_pair(a: i32, b: i32) -> (i32, i32) {
          return (a, b);
      }

      pub fn main(none) -> i32 {
          let pair = make_pair(5, 15);
          return pair.0 + pair.1;
      }
      """
    When I compile and run the program
    Then the exit code should be 20

  Scenario: Return with enum construction
    Given I have the following Asthra code:
      """
      package test;

      enum Status {
          Success(i32),
          Error
      }

      pub fn get_status(success: bool) -> Status {
          if success {
              return Status.Success(42);
          } else {
              return Status.Error;
          }
      }

      pub fn main(none) -> i32 {
          let status = get_status(true);
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Return with Result type
    Given I have the following Asthra code:
      """
      package test;

      pub fn safe_divide(a: i32, b: i32) -> Result<i32, string> {
          if b == 0 {
              return Result::Err("Division by zero");
          }
          return Result::Ok(a / b);
      }

      pub fn main(none) -> i32 {
          let result = safe_divide(10, 2);
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Return with Option type
    Given I have the following Asthra code:
      """
      package test;

      pub fn find_positive(value: i32) -> Option<i32> {
          if value > 0 {
              return Option::Some(value);
          }
          return Option::None;
      }

      pub fn main(none) -> i32 {
          let result = find_positive(25);
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Multiple return paths
    Given I have the following Asthra code:
      """
      package test;

      pub fn classify_number(num: i32) -> i32 {
          if num < 0 {
              return 1;  // Negative
          } else if num == 0 {
              return 2;  // Zero
          } else if num < 10 {
              return 3;  // Single digit positive
          } else {
              return 4;  // Multi-digit positive
          }
      }

      pub fn main(none) -> i32 {
          return classify_number(7);
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Return in loop
    Given I have the following Asthra code:
      """
      package test;

      pub fn find_first_even(array: [5]i32) -> i32 {
          for i in range(len(array)) {
              if array[i] % 2 == 0 {
                  return array[i];
              }
          }
          return -1;  // Not found
      }

      pub fn main(none) -> i32 {
          let numbers: [5]i32 = [1, 3, 8, 5, 7];
          return find_first_even(numbers);
      }
      """
    When I compile and run the program
    Then the exit code should be 8

  Scenario: Return with complex expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn complex_calculation(x: i32, y: i32, z: i32) -> i32 {
          return (x + y) * z - (x - y) / 2;
      }

      pub fn main(none) -> i32 {
          return complex_calculation(10, 6, 3);
      }
      """
    When I compile and run the program
    Then the exit code should be 46

  Scenario: Return with pointer dereference
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_value_through_pointer(ptr: *const i32) -> i32 {
          unsafe {
              return *ptr;
          }
      }

      pub fn main(none) -> i32 {
          let value: i32 = 99;
          return get_value_through_pointer(&value);
      }
      """
    When I compile and run the program
    Then the exit code should be 99

  Scenario: Return with sizeof expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_type_size<T>(none) -> usize {
          return sizeof(T);
      }

      pub fn main(none) -> i32 {
          let size = get_type_size<i64>();
          return size as i32;
      }
      """
    When I compile and run the program
    Then the exit code should be 8

  Scenario: Error - Missing return expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn incomplete_return(none) -> i32 {
          return;  // Error: missing expression
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected expression" or "missing return value"

  Scenario: Error - Return type mismatch
    Given I have the following Asthra code:
      """
      package test;

      pub fn wrong_return(none) -> i32 {
          return "string";  // Error: wrong type
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected i32"

  Scenario: Error - Missing semicolon after return
    Given I have the following Asthra code:
      """
      package test;

      pub fn missing_semicolon(none) -> i32 {
          return 42  // Error: missing semicolon
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected semicolon" or "syntax error"

  Scenario: Error - Return in void function without unit
    Given I have the following Asthra code:
      """
      package test;

      pub fn void_function(none) -> void {
          return 42;  // Error: void function can't return value
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "void function" or "unexpected return value"

  Scenario: Return with method call
    Given I have the following Asthra code:
      """
      package test;

      struct Calculator {
          base: i32
      }

      impl Calculator {
          pub fn add(self, value: i32) -> i32 {
              return self.base + value;
          }
      }

      pub fn get_result(none) -> i32 {
          let calc = Calculator { base: 10 };
          return calc.add(5);
      }

      pub fn main(none) -> i32 {
          return get_result();
      }
      """
    When I compile and run the program
    Then the exit code should be 15

  Scenario: Return with cast expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn convert_to_int(value: f32) -> i32 {
          return value as i32;
      }

      pub fn main(none) -> i32 {
          return convert_to_int(42.7);
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Return with slice access
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_slice_element(data: []i32, index: i32) -> i32 {
          return data[index];
      }

      pub fn main(none) -> i32 {
          let array: [4]i32 = [5, 10, 15, 20];
          let slice: []i32 = array[1:3];
          return get_slice_element(slice, 1);
      }
      """
    When I compile and run the program
    Then the exit code should be 15