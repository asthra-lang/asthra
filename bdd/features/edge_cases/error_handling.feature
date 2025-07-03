@edge_cases
Feature: Result type error handling patterns
  As a language designer
  I want to ensure Result types provide comprehensive error handling
  So that programs can handle errors safely and ergonomically

  Background:
    Given the Asthra compiler is available

  # Test 1: Basic Result type construction and usage
  Scenario: Creating and using Result types with Ok and Err variants
    Given I have the following Asthra code:
      """
      package test;

      enum MyError {
          DivideByZero,
          InvalidInput
      }

      pub fn divide(a: i32, b: i32) -> Result<i32, MyError> {
          if b == 0 {
              return Result.Err(MyError.DivideByZero);
          }
          return Result.Ok(a / b);
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, MyError> = divide(10, 2);
          match result {
              Result.Ok(value) => {
                  return value;
              },
              Result.Err(_) => {
                  return -1;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 5

  # Test 2: Pattern matching on Result types with error extraction
  Scenario: Extracting and handling specific errors from Result
    Given I have the following Asthra code:
      """
      package test;

      enum ParseError {
          InvalidFormat,
          OutOfRange(i32)
      }

      pub fn parse_number(s: string) -> Result<i32, ParseError> {
          if s == "invalid" {
              return Result.Err(ParseError.InvalidFormat);
          }
          if s == "999999" {
              return Result.Err(ParseError.OutOfRange(999999));
          }
          return Result.Ok(42);
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, ParseError> = parse_number("999999");
          match result {
              Result.Ok(n) => {
                  return n;
              },
              Result.Err(ParseError.InvalidFormat) => {
                  return -1;
              },
              Result.Err(ParseError.OutOfRange(value)) => {
                  return value / 10000;  // Returns 99
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 99

  # Test 3: Propagating errors through function returns
  Scenario: Error propagation through multiple function calls
    Given I have the following Asthra code:
      """
      package test;

      enum FileError {
          NotFound,
          PermissionDenied
      }

      pub fn read_file(path: string) -> Result<string, FileError> {
          if path == "missing.txt" {
              return Result.Err(FileError.NotFound);
          }
          return Result.Ok("file contents");
      }

      pub fn process_file(path: string) -> Result<i32, FileError> {
          let contents: Result<string, FileError> = read_file(path);
          match contents {
              Result.Ok(data) => {
                  return Result.Ok(100);
              },
              Result.Err(e) => {
                  return Result.Err(e);  // Propagate error
              }
          }
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, FileError> = process_file("missing.txt");
          match result {
              Result.Ok(val) => {
                  return val;
              },
              Result.Err(FileError.NotFound) => {
                  return 404;
              },
              Result.Err(FileError.PermissionDenied) => {
                  return 403;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 404

  # Test 4: Chaining Result operations
  Scenario: Combining multiple Result-returning operations
    Given I have the following Asthra code:
      """
      package test;

      enum MathError {
          DivideByZero,
          Overflow
      }

      pub fn safe_divide(a: i32, b: i32) -> Result<i32, MathError> {
          if b == 0 {
              return Result.Err(MathError.DivideByZero);
          }
          return Result.Ok(a / b);
      }

      pub fn safe_multiply(a: i32, b: i32) -> Result<i32, MathError> {
          if a > 1000 || b > 1000 {
              return Result.Err(MathError.Overflow);
          }
          return Result.Ok(a * b);
      }

      pub fn calculate(x: i32, y: i32, z: i32) -> Result<i32, MathError> {
          let div_result: Result<i32, MathError> = safe_divide(x, y);
          match div_result {
              Result.Ok(quotient) => {
                  let mul_result: Result<i32, MathError> = safe_multiply(quotient, z);
                  match mul_result {
                      Result.Ok(product) => {
                          return Result.Ok(product);
                      },
                      Result.Err(e) => {
                          return Result.Err(e);
                      }
                  }
              },
              Result.Err(e) => {
                  return Result.Err(e);
              }
          }
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, MathError> = calculate(100, 5, 10);
          match result {
              Result.Ok(val) => {
                  return val;
              },
              Result.Err(_) => {
                  return -1;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 200

  # Test 5: Converting between Result and Option types
  Scenario: Converting Result to Option and vice versa
    Given I have the following Asthra code:
      """
      package test;

      enum SearchError {
          NotFound,
          InvalidQuery
      }

      pub fn result_to_option(res: Result<i32, SearchError>) -> Option<i32> {
          match res {
              Result.Ok(val) => {
                  return Option.Some(val);
              },
              Result.Err(_) => {
                  return Option.None;
              }
          }
      }

      pub fn option_to_result(opt: Option<i32>, err: SearchError) -> Result<i32, SearchError> {
          match opt {
              Option.Some(val) => {
                  return Result.Ok(val);
              },
              Option.None => {
                  return Result.Err(err);
              }
          }
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, SearchError> = Result.Ok(42);
          let option: Option<i32> = result_to_option(result);
          
          let back_to_result: Result<i32, SearchError> = option_to_result(option, SearchError.NotFound);
          match back_to_result {
              Result.Ok(val) => {
                  return val;
              },
              Result.Err(_) => {
                  return -1;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  # Test 6: Error handling in spawned tasks
  Scenario: Handling Results from spawned tasks with TaskHandle
    Given I have the following Asthra code:
      """
      package test;

      enum TaskError {
          Failed,
          Timeout
      }

      pub fn risky_task(value: i32) -> Result<i32, TaskError> {
          if value < 0 {
              return Result.Err(TaskError.Failed);
          }
          return Result.Ok(value * 2);
      }

      pub fn main(none) -> i32 {
          let handle: TaskHandle<Result<i32, TaskError>> = spawn risky_task(50);
          let task_result: Result<i32, TaskError> = await handle;
          
          match task_result {
              Result.Ok(val) => {
                  return val;
              },
              Result.Err(TaskError.Failed) => {
                  return -1;
              },
              Result.Err(TaskError.Timeout) => {
                  return -2;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 100

  # Test 7: Result types with complex generic parameters
  Scenario: Using Result with nested generic types
    Given I have the following Asthra code:
      """
      package test;

      struct Data<T> {
          value: T
      }

      enum ComplexError<E> {
          DataError(E),
          SystemError
      }

      pub fn process_data(input: i32) -> Result<Data<i32>, ComplexError<string>> {
          if input < 0 {
              return Result.Err(ComplexError.DataError("negative input"));
          }
          if input > 1000 {
              return Result.Err(ComplexError.SystemError);
          }
          let data: Data<i32> = Data { value: input * 3 };
          return Result.Ok(data);
      }

      pub fn main(none) -> i32 {
          let result: Result<Data<i32>, ComplexError<string>> = process_data(10);
          match result {
              Result.Ok(data) => {
                  return data.value;
              },
              Result.Err(ComplexError.DataError(_)) => {
                  return -1;
              },
              Result.Err(ComplexError.SystemError) => {
                  return -2;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 30

  # Test 8: Never type in error cases
  Scenario: Using Never type for functions that always return errors
    Given I have the following Asthra code:
      """
      package test;

      enum FatalError {
          Unrecoverable,
          Panic
      }

      pub fn always_fails(none) -> Result<i32, FatalError> {
          return Result.Err(FatalError.Unrecoverable);
      }

      pub fn panic_function(msg: string) -> Never {
          // This function never returns normally
          // In a real implementation, this would panic/abort
          loop {
              // Infinite loop represents non-returning behavior
          }
      }

      pub fn handle_fatal_error(err: FatalError) -> i32 {
          match err {
              FatalError.Unrecoverable => {
                  return 255;
              },
              FatalError.Panic => {
                  // In theory would call panic_function, but we return for testing
                  return 254;
              }
          }
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, FatalError> = always_fails(none);
          match result {
              Result.Ok(_) => {
                  return 0;  // Should never reach here
              },
              Result.Err(e) => {
                  return handle_fatal_error(e);
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 255

  # Test 9: Result type with tuple success values
  Scenario: Using Result with tuple types for multiple return values
    Given I have the following Asthra code:
      """
      package test;

      enum ValidationError {
          TooShort,
          TooLong,
          InvalidChar
      }

      pub fn validate_and_parse(input: string) -> Result<(i32, bool), ValidationError> {
          if input == "short" {
              return Result.Err(ValidationError.TooShort);
          }
          if input == "toolongstring" {
              return Result.Err(ValidationError.TooLong);
          }
          if input == "valid" {
              return Result.Ok((42, true));
          }
          return Result.Ok((100, false));
      }

      pub fn main(none) -> i32 {
          let result: Result<(i32, bool), ValidationError> = validate_and_parse("valid");
          match result {
              Result.Ok((num, flag)) => {
                  if flag {
                      return num;
                  }
                  return num + 1;
              },
              Result.Err(ValidationError.TooShort) => {
                  return -1;
              },
              Result.Err(ValidationError.TooLong) => {
                  return -2;
              },
              Result.Err(ValidationError.InvalidChar) => {
                  return -3;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  # Test 10: Nested Result types
  Scenario: Working with Result<Result<T, E1>, E2> patterns
    Given I have the following Asthra code:
      """
      package test;

      enum OuterError {
          NetworkError,
          Timeout
      }

      enum InnerError {
          ParseError,
          ValidationError
      }

      pub fn fetch_data(none) -> Result<string, OuterError> {
          return Result.Ok("42");
      }

      pub fn parse_data(data: string) -> Result<i32, InnerError> {
          if data == "42" {
              return Result.Ok(42);
          }
          return Result.Err(InnerError.ParseError);
      }

      pub fn fetch_and_parse(none) -> Result<Result<i32, InnerError>, OuterError> {
          let fetch_result: Result<string, OuterError> = fetch_data(none);
          match fetch_result {
              Result.Ok(data) => {
                  let parse_result: Result<i32, InnerError> = parse_data(data);
                  return Result.Ok(parse_result);
              },
              Result.Err(e) => {
                  return Result.Err(e);
              }
          }
      }

      pub fn main(none) -> i32 {
          let nested_result: Result<Result<i32, InnerError>, OuterError> = fetch_and_parse(none);
          match nested_result {
              Result.Ok(inner_result) => {
                  match inner_result {
                      Result.Ok(value) => {
                          return value;
                      },
                      Result.Err(InnerError.ParseError) => {
                          return -10;
                      },
                      Result.Err(InnerError.ValidationError) => {
                          return -11;
                      }
                  }
              },
              Result.Err(OuterError.NetworkError) => {
                  return -20;
              },
              Result.Err(OuterError.Timeout) => {
                  return -21;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42