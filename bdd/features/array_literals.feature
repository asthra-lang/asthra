Feature: Array Literals
  As a developer using Asthra
  I want to use array literal syntax
  So that I can create and initialize arrays with various patterns

  Background:
    Given a new compilation context

  # Empty arrays
  @wip
  Scenario: Empty array with none marker
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [none];
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Empty array for fixed size
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [0]i32 = [none];
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Simple array literals
  @wip
  Scenario: Single element array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [42];
        return arr[0];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Multiple element array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 50];
        return arr[1] + arr[3] - 18;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Array of different numeric types
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr_i8: []i8 = [1, 2, 3];
        let arr_u32: []u32 = [100, 200, 300];
        return (arr_i8[1] as i32) + 40;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Repeated element arrays
  @wip
  Scenario: Repeated element array syntax
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [7; 6];
        return arr[0] * arr[5];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Repeated zero initialization
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [0; 100];
        return arr[0] + arr[50] + arr[99] + 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Repeated element with expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let value: i32 = 14;
        let arr: []i32 = [value; 3];
        return arr[0] + arr[1] + arr[2];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Fixed-size arrays
  @wip
  Scenario: Fixed-size array declaration
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [5]i32 = [8, 8, 8, 9, 9];
        return arr[0] + arr[1] + arr[2] + arr[3] + arr[4];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Fixed-size array with const size
    Given the following Asthra code:
      """
      package test;
      pub const SIZE: i32 = 3;
      pub fn main(none) -> i32 {
        let arr: [SIZE]i32 = [14, 14, 14];
        return arr[0] + arr[1] + arr[2];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Array indexing
  @wip
  Scenario: Basic array indexing
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 50];
        let idx: i32 = 2;
        return arr[idx] + 12;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Array indexing with expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [5, 10, 15, 20, 25];
        return arr[1 + 2] + 22;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mutable arrays
  @wip
  Scenario: Mutable array element modification
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut arr: []i32 = [10, 20, 30];
        arr[1] = 12;
        return arr[0] + arr[1] + arr[2] - 10;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Mutable array reassignment
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut arr: []i32 = [1, 2, 3];
        arr = [14, 14, 14];
        return arr[0] + arr[1] + arr[2];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Array of arrays
  Scenario: Two-dimensional array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let matrix: [][]i32 = [[1, 2], [3, 4], [5, 6]];
        return matrix[0][0] + matrix[1][1] + matrix[2][0] + 32;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Nested array indexing
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [][]i32 = [[10, 20], [30, 40]];
        return arr[0][1] + arr[1][0] - 8;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Array slicing
  @wip
  Scenario: Full array slice
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 50];
        let slice: []i32 = arr[:];
        return slice[1] + slice[3] - 18;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Array slice from start
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [5, 10, 15, 20, 25];
        let slice: []i32 = arr[:3];
        return slice[0] + slice[1] + slice[2] + 12;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Array slice to end
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [5, 10, 15, 20, 25];
        let slice: []i32 = arr[2:];
        return slice[0] + slice[1] + slice[2] - 18;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Array slice range
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [1, 2, 3, 4, 5, 6, 7, 8, 9];
        let slice: []i32 = arr[2:5];
        return slice[0] + slice[1] + slice[2] + 30;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Array length
  @wip
  Scenario: Get array length
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30, 40, 50, 60, 70];
        return arr.len as i32 * 6;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Empty array length
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [none];
        return arr.len as i32 + 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Arrays with other types
  @wip
  Scenario: Boolean array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let flags: []bool = [true, false, true, true];
        if flags[0] && !flags[1] && flags[2] {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: String array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let words: []string = ["hello", "world", "test"];
        if words[0] == "hello" && words[1] == "world" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Arrays as function parameters
  Scenario: Pass array to function
    Given the following Asthra code:
      """
      package test;
      pub fn sum_array(arr: []i32) -> i32 {
        return arr[0] + arr[1] + arr[2];
      }
      pub fn main(none) -> i32 {
        let numbers: []i32 = [10, 15, 17];
        return sum_array(numbers);
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Return array from function
    Given the following Asthra code:
      """
      package test;
      pub fn create_array(none) -> []i32 {
        return [14, 14, 14];
      }
      pub fn main(none) -> i32 {
        let arr: []i32 = create_array(none);
        return arr[0] + arr[1] + arr[2];
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Array operations in loops
  @wip
  Scenario: Iterate over array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [5, 8, 10, 7, 12];
        let sum: i32 = 0;
        for val in arr {
          sum = sum + val;
        }
        return sum;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Iterate with range
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [10, 20, 30];
        let mut sum: i32 = 0;
        for i in range(3) {
          sum = sum + arr[i];
        }
        return sum - 18;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases
  @wip
  Scenario: Large repeated array
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: []i32 = [1; 42];
        return arr[0] + arr[20] + arr[41] + 39;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  @wip
  Scenario: Array of tuples
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let pairs: [](i32, i32) = [(10, 20), (5, 7), (15, 25)];
        let p: (i32, i32) = pairs[1];
        return p.0 + p.1 + 30;
      }
      """
    When I compile and run the code
    Then the exit code should be 42