Feature: Associated functions
  As a developer
  I want to call static methods using Type::function() syntax
  So that I can access type-specific functionality without instances

  Background:
    Given the Asthra compiler is available

  Scenario: Basic associated function call
    Given I have the following Asthra code:
      """
      package test;

      struct Math {
          none
      }

      impl Math {
          pub fn add(a: i32, b: i32) -> i32 {
              return a + b;
          }
      }

      pub fn main(none) -> i32 {
          let result = Math::add(5, 3);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 8

  Scenario: Associated function with multiple parameters
    Given I have the following Asthra code:
      """
      package test;

      struct Calculator {
          none
      }

      impl Calculator {
          pub fn multiply_and_add(a: i32, b: i32, c: i32) -> i32 {
              return (a * b) + c;
          }
      }

      pub fn main(none) -> i32 {
          let result = Calculator::multiply_and_add(3, 4, 2);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 14

  Scenario: Associated function returning custom type
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: i32,
          y: i32
      }

      impl Point {
          pub fn new(x_val: i32, y_val: i32) -> Point {
              return Point { x: x_val, y: y_val };
          }
          
          pub fn origin(none) -> Point {
              return Point { x: 0, y: 0 };
          }
      }

      pub fn main(none) -> i32 {
          let p1: Point = Point::new(3, 4);
          let p2: Point = Point::origin();
          return p1.x + p1.y + p2.x + p2.y;
      }
      """
    When I compile and run the program
    Then the exit code should be 7

  Scenario: Associated function with generic type
    Given I have the following Asthra code:
      """
      package test;

      struct Container<T> {
          value: T
      }

      impl Container<T> {
          pub fn create(val: T) -> Container<T> {
              return Container { value: val };
          }
      }

      pub fn main(none) -> i32 {
          let int_container: Container<i32> = Container::create(42);
          let bool_container: Container<bool> = Container::create(true);
          
          if bool_container.value {
              return int_container.value;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Associated function with explicit type arguments
    Given I have the following Asthra code:
      """
      package test;

      struct Wrapper<T> {
          data: T
      }

      impl Wrapper<T> {
          pub fn default_value(none) -> T {
              // Implementation would return default for T
              return 0 as T;  // Simplified for test
          }
      }

      pub fn main(none) -> i32 {
          let default_int = Wrapper<i32>::default_value();
          return default_int;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Multiple associated functions on same type
    Given I have the following Asthra code:
      """
      package test;

      struct Utils {
          none
      }

      impl Utils {
          pub fn add(a: i32, b: i32) -> i32 {
              return a + b;
          }
          
          pub fn subtract(a: i32, b: i32) -> i32 {
              return a - b;
          }
          
          pub fn multiply(a: i32, b: i32) -> i32 {
              return a * b;
          }
      }

      pub fn main(none) -> i32 {
          let sum = Utils::add(10, 5);
          let diff = Utils::subtract(20, 8);
          let product = Utils::multiply(3, 4);
          return sum + diff + product;
      }
      """
    When I compile and run the program
    Then the exit code should be 39

  Scenario: Associated function with no parameters
    Given I have the following Asthra code:
      """
      package test;

      struct Constants {
          none
      }

      impl Constants {
          pub fn pi(none) -> f32 {
              return 3.14159;
          }
          
          pub fn magic_number(none) -> i32 {
              return 42;
          }
      }

      pub fn main(none) -> i32 {
          let magic = Constants::magic_number();
          return magic;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Associated function in expressions
    Given I have the following Asthra code:
      """
      package test;

      struct Math {
          none
      }

      impl Math {
          pub fn square(x: i32) -> i32 {
              return x * x;
          }
          
          pub fn double(x: i32) -> i32 {
              return x * 2;
          }
      }

      pub fn main(none) -> i32 {
          let result = Math::square(3) + Math::double(4);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 17

  Scenario: Associated function with Result return type
    Given I have the following Asthra code:
      """
      package test;

      struct SafeMath {
          none
      }

      impl SafeMath {
          pub fn divide(a: i32, b: i32) -> Result<i32, string> {
              if b == 0 {
                  return Result::Err("Division by zero");
              } else {
                  return Result::Ok(a / b);
              }
          }
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, string> = SafeMath::divide(20, 4);
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Associated function with Option return type
    Given I have the following Asthra code:
      """
      package test;

      struct Finder {
          none
      }

      impl Finder {
          pub fn find_positive(value: i32) -> Option<i32> {
              if value > 0 {
                  return Option::Some(value);
              } else {
                  return Option::None;
              }
          }
      }

      pub fn main(none) -> i32 {
          let result: Option<i32> = Finder::find_positive(42);
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Chained associated function calls
    Given I have the following Asthra code:
      """
      package test;

      struct Builder {
          value: i32
      }

      impl Builder {
          pub fn new(initial: i32) -> Builder {
              return Builder { value: initial };
          }
          
          pub fn from_builder(builder: Builder) -> i32 {
              return builder.value * 2;
          }
      }

      pub fn main(none) -> i32 {
          let result = Builder::from_builder(Builder::new(5));
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  Scenario: Associated function with complex parameter types
    Given I have the following Asthra code:
      """
      package test;

      struct ArrayProcessor {
          none
      }

      impl ArrayProcessor {
          pub fn sum_array(arr: [3]i32) -> i32 {
              return arr[0] + arr[1] + arr[2];
          }
          
          pub fn process_slice(slice: []i32) -> i32 {
              return len(slice) as i32;
          }
      }

      pub fn main(none) -> i32 {
          let array: [3]i32 = [1, 2, 3];
          let sum = ArrayProcessor::sum_array(array);
          return sum;
      }
      """
    When I compile and run the program
    Then the exit code should be 6

  Scenario: Error - Call non-existent associated function
    Given I have the following Asthra code:
      """
      package test;

      struct Empty {
          none
      }

      impl Empty {
          pub fn existing_function(none) -> i32 {
              return 42;
          }
      }

      pub fn main(none) -> i32 {
          let result = Empty::non_existent();  // Error: function not defined
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "function not found" or "undefined function"

  Scenario: Error - Associated function call on undefined type
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let result = UndefinedType::some_function();  // Error: type not defined
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "undefined type" or "type not found"

  Scenario: Error - Wrong parameter count in associated function
    Given I have the following Asthra code:
      """
      package test;

      struct Math {
          none
      }

      impl Math {
          pub fn add(a: i32, b: i32) -> i32 {
              return a + b;
          }
      }

      pub fn main(none) -> i32 {
          let result = Math::add(5);  // Error: missing parameter
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "parameter count" or "expected 2 arguments"

  Scenario: Error - Wrong parameter types in associated function
    Given I have the following Asthra code:
      """
      package test;

      struct Processor {
          none
      }

      impl Processor {
          pub fn process(value: i32) -> i32 {
              return value * 2;
          }
      }

      pub fn main(none) -> i32 {
          let result = Processor::process("invalid");  // Error: wrong type
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected i32"

  Scenario: Error - Missing :: operator
    Given I have the following Asthra code:
      """
      package test;

      struct Helper {
          none
      }

      impl Helper {
          pub fn help(none) -> i32 {
              return 42;
          }
      }

      pub fn main(none) -> i32 {
          let result = Helper.help();  // Error: should use ::
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected '::'" or "syntax error"

  Scenario: Associated function visibility
    Given I have the following Asthra code:
      """
      package test;

      struct AccessTest {
          none
      }

      impl AccessTest {
          pub fn public_function(none) -> i32 {
              return 1;
          }
          
          priv fn private_function(none) -> i32 {
              return 2;
          }
      }

      pub fn main(none) -> i32 {
          let result = AccessTest::public_function();
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Associated function with tuple parameters
    Given I have the following Asthra code:
      """
      package test;

      struct TupleProcessor {
          none
      }

      impl TupleProcessor {
          pub fn process_pair(pair: (i32, i32)) -> i32 {
              return pair.0 + pair.1;
          }
      }

      pub fn main(none) -> i32 {
          let tuple: (i32, i32) = (5, 7);
          let result = TupleProcessor::process_pair(tuple);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 12