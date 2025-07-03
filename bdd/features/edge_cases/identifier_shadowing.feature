@edge_cases
Feature: Predeclared identifier shadowing
  As a language designer
  I want to ensure predeclared identifiers can be properly shadowed
  So that users have flexibility while maintaining semantic clarity

  Background:
    Given the Asthra compiler is available

  # Test 1: Shadowing predeclared functions with user-defined functions
  Scenario: Shadow log function with custom implementation
    Given I have the following Asthra code:
      """
      package test;

      // Shadow the predeclared log function
      pub fn log(prefix: string, message: string) -> void {
          // Custom implementation with different signature
          return ();
      }

      pub fn main(none) -> i32 {
          // Uses our custom log function
          log("INFO", "Using custom log");
          return 42;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Shadow range function with different return type
    Given I have the following Asthra code:
      """
      package test;

      // Shadow predeclared range with a function returning string
      pub fn range(count: i32) -> string {
          return "custom range " + string(count);
      }

      pub fn main(none) -> i32 {
          let result: string = range(5);
          if result == "custom range 5" {
              return 1;
          }
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Shadow panic function to make it non-terminating
    Given I have the following Asthra code:
      """
      package test;

      // Shadow panic with a safe version
      pub fn panic(msg: string) -> i32 {
          // Instead of terminating, just return an error code
          return -1;
      }

      pub fn main(none) -> i32 {
          let error_code: i32 = panic("This won't terminate");
          if error_code == -1 {
              return 0;  // Success - our panic didn't terminate
          }
          return 1;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  # Test 2: Shadowing predeclared identifiers with variables
  Scenario: Shadow all predeclared identifiers as variables
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let log: i32 = 10;
          let range: i32 = 20;
          let panic: i32 = 30;
          let exit: i32 = 40;
          let args: i32 = 50;
          let len: i32 = 60;
          let infinite: i32 = 70;
          
          // All predeclared names are now variables
          return log + range + panic + exit + args + len + infinite;
      }
      """
    When I compile and run the program
    Then the exit code should be 280

  Scenario: Shadow predeclared identifiers with different types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let log: bool = true;
          let range: string = "range string";
          let panic: f64 = 3.14;
          let exit: []i32 = [1, 2, 3];
          
          if log && range == "range string" {
              return len(exit);  // Uses predeclared len since we didn't shadow it
          }
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  # Test 3: Nested scope shadowing
  Scenario: Inner scope shadows outer scope predeclared usage
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          // Outer scope uses predeclared log
          log("Outer scope message");
          
          {
              // Inner scope shadows log
              let log: i32 = 42;
              
              // Can't use predeclared log here - it's shadowed
              if log == 42 {
                  return log;
              }
          }
          
          // Back in outer scope, predeclared log is available again
          log("Back in outer scope");
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Multiple nested scope shadowing
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut result: i32 = 0;
          
          // Level 1: Use predeclared range
          for i in range(3) {
              result = result + i;
              
              // Level 2: Shadow range as variable
              let range: i32 = 10;
              result = result + range;
              
              {
                  // Level 3: Shadow range as string
                  let range: string = "nested";
                  if range == "nested" {
                      result = result + 1;
                  }
              }
          }
          
          return result;  // (0+10+1) + (1+10+1) + (2+10+1) = 36
      }
      """
    When I compile and run the program
    Then the exit code should be 36

  # Test 4: Shadowing with different types
  Scenario: Shadow log as a struct type
    Given I have the following Asthra code:
      """
      package test;

      struct log {
          level: i32,
          message: string
      }

      pub fn main(none) -> i32 {
          let entry: log = log { level: 5, message: "test" };
          return entry.level;
      }
      """
    When I compile and run the program
    Then the exit code should be 5

  Scenario: Shadow range as an enum type
    Given I have the following Asthra code:
      """
      package test;

      enum range {
          Empty,
          WithBounds(i32, i32)
      }

      pub fn main(none) -> i32 {
          let r: range = range::WithBounds(10, 20);
          match r {
              range::Empty => return 0;,
              range::WithBounds(start, end) => return end - start;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  # Test 5: Using both shadowed and original versions in different scopes
  Scenario: Function scope shadows, then uses predeclared in nested function
    Given I have the following Asthra code:
      """
      package test;

      fn use_predeclared(none) -> void {
          // This function uses the predeclared log
          log("Using predeclared log");
          return ();
      }

      fn use_shadowed(none) -> i32 {
          // Shadow log as a variable
          let log: i32 = 99;
          
          // Call function that uses predeclared version
          use_predeclared();
          
          // Return our shadowed variable
          return log;
      }

      pub fn main(none) -> i32 {
          return use_shadowed();
      }
      """
    When I compile and run the program
    Then the exit code should be 99

  Scenario: Complex scope interaction with multiple shadows
    Given I have the following Asthra code:
      """
      package test;

      // Global function that shadows panic
      fn panic(code: i32) -> i32 {
          return code * 2;
      }

      pub fn main(none) -> i32 {
          // Use our shadowed panic function
          let result1: i32 = panic(5);  // 10
          
          {
              // Shadow panic as a variable in this block
              let panic: i32 = 3;
              
              // Shadow exit as well
              let exit: i32 = 7;
              
              result1 = result1 + panic + exit;  // 10 + 3 + 7 = 20
          }
          
          // Back to using our function shadow
          let result2: i32 = panic(result1);  // 20 * 2 = 40
          
          return result2;
      }
      """
    When I compile and run the program
    Then the exit code should be 40

  # Test 6: Shadowing multiple predeclared identifiers in the same scope
  Scenario: Shadow all predeclared identifiers as functions
    Given I have the following Asthra code:
      """
      package test;

      fn log(x: i32) -> i32 { return x + 1; }
      fn range(x: i32) -> i32 { return x + 2; }
      fn panic(x: i32) -> i32 { return x + 3; }
      fn exit(x: i32) -> i32 { return x + 4; }
      fn args(x: i32) -> i32 { return x + 5; }
      fn len(x: i32) -> i32 { return x + 6; }
      fn infinite(x: i32) -> i32 { return x + 7; }

      pub fn main(none) -> i32 {
          let mut sum: i32 = 0;
          sum = log(sum);      // 0 + 1 = 1
          sum = range(sum);    // 1 + 2 = 3
          sum = panic(sum);    // 3 + 3 = 6
          sum = exit(sum);     // 6 + 4 = 10
          sum = args(sum);     // 10 + 5 = 15
          sum = len(sum);      // 15 + 6 = 21
          sum = infinite(sum); // 21 + 7 = 28
          return sum;
      }
      """
    When I compile and run the program
    Then the exit code should be 28

  Scenario: Mix of function and variable shadows in same scope
    Given I have the following Asthra code:
      """
      package test;

      // Shadow some as functions
      fn log(n: i32) -> i32 { return n * 10; }
      fn range(n: i32) -> i32 { return n * 20; }

      pub fn main(none) -> i32 {
          // Shadow others as variables
          let panic: i32 = 3;
          let exit: i32 = 4;
          
          // Use the function shadows
          let result1: i32 = log(2);    // 20
          let result2: i32 = range(1);  // 20
          
          // Use the variable shadows
          let result3: i32 = panic;     // 3
          let result4: i32 = exit;      // 4
          
          return result1 + result2 + result3 + result4;  // 47
      }
      """
    When I compile and run the program
    Then the exit code should be 47

  # Edge case: Attempting to use predeclared after shadowing in same scope
  Scenario: Cannot access predeclared identifier after shadowing in same scope
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          // Shadow log as a variable
          let log: i32 = 42;
          
          // Try to use it as the predeclared function (should fail)
          log("This should not compile");
          
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type" or "cannot call"

  # Test shadowing with struct field access
  Scenario: Shadow predeclared identifier then use as struct field
    Given I have the following Asthra code:
      """
      package test;

      struct Logger {
          log: i32,
          range: string
      }

      pub fn main(none) -> i32 {
          // Create struct with fields named after predeclared identifiers
          let logger: Logger = Logger { log: 15, range: "test" };
          
          // Access fields (not calling predeclared functions)
          let value: i32 = logger.log;
          
          // Shadow range as a function that returns the field value
          fn range(l: Logger) -> i32 {
              return l.log * 2;
          }
          
          return range(logger);  // 15 * 2 = 30
      }
      """
    When I compile and run the program
    Then the exit code should be 30

  # Test method names that shadow predeclared identifiers
  Scenario: Associated functions with predeclared names
    Given I have the following Asthra code:
      """
      package test;

      struct Counter {
          value: i32
      }

      impl Counter {
          fn log(self: &Counter) -> i32 {
              return self.value;
          }
          
          fn range(self: &Counter, multiplier: i32) -> i32 {
              return self.value * multiplier;
          }
      }

      pub fn main(none) -> i32 {
          let c: Counter = Counter { value: 7 };
          
          // Use associated functions that shadow predeclared names
          let logged: i32 = c.log();
          let ranged: i32 = c.range(3);
          
          return logged + ranged;  // 7 + 21 = 28
      }
      """
    When I compile and run the program
    Then the exit code should be 28