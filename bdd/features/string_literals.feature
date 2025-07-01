Feature: String Literals
  As a developer using Asthra
  I want to use various string literal formats
  So that I can express text data with proper formatting and escape sequences

  Background:
    Given a new compilation context

  # Regular string literals

  Scenario: Simple string literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Hello";
        if s == "Hello" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Empty string literal
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "";
        if s == "" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String with spaces
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Hello World";
        if s == "Hello World" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Escape sequences in regular strings

  Scenario: String with newline escape
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Line1\nLine2";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String with tab escape
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Column1\tColumn2";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String with carriage return escape
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Text\rReturn";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String with backslash escape
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Path\\to\\file";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String with quote escape
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "He said \"Hello\"";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String with single quote escape
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "It\'s working";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: String with null character escape
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Null\0End";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Multi-line processed strings

  Scenario: Simple multi-line processed string
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = """Hello
World""";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Multi-line processed string with escape sequences
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = """Line1\nLine2
Line3\tTabbed""";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Empty multi-line processed string
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = """""";
        if s == "" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Raw multi-line strings

  Scenario: Simple raw multi-line string
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = r"""No escape processing\n""";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Raw string with backslashes
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = r"""C:\path\to\file""";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Raw string with quotes
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = r"""He said "Hello" without escaping""";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # String comparisons

  Scenario: Compare regular strings
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s1: string = "test";
        let s2: string = "test";
        if s1 == s2 {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Compare different string types
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s1: string = "Hello";
        let s2: string = """Hello""";
        if s1 == s2 {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # String concatenation

  Scenario: Concatenate two strings
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s1: string = "Hello";
        let s2: string = " World";
        let s3: string = s1 + s2;
        if s3 == "Hello World" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # String in expressions

  Scenario: String in conditional expression
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let name: string = "Alice";
        if name == "Alice" || name == "Bob" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # String as function parameter

  Scenario: Pass string to function
    Given the following Asthra code:
      """
      package test;
      pub fn check_string(s: string) -> i32 {
        if s == "correct" {
          return 42;
        } else {
          return 0;
        }
      }
      pub fn main(none) -> i32 {
        return check_string("correct");
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Const strings

  Scenario: Const string declaration
    Given the following Asthra code:
      """
      package test;
      pub const MESSAGE: string = "Hello, World!";
      pub fn main(none) -> i32 {
        if MESSAGE == "Hello, World!" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # String length

  Scenario: Get string length
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "Hello";
        let len: i32 = s.len as i32;
        if len == 5 {
          return 42;
        } else {
          return len;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Edge cases

  Scenario: String with only escape sequences
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = "\n\t\r\\\"\'\0";
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  Scenario: Multi-line string with indentation
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let s: string = """
        Indented line 1
          Indented line 2
            Indented line 3
        """;
        return 42;
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # String in match expressions

  Scenario: Match on string value
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let cmd: string = "run";
        match cmd {
          "run" => { return 42; }
          "stop" => { return 0; }
          _ => { return 1; }
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42

  # Mutable strings

  Scenario: Modify mutable string variable
    Given the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let mut s: string = "old";
        s = "new";
        if s == "new" {
          return 42;
        } else {
          return 0;
        }
      }
      """
    When I compile and run the code
    Then the exit code should be 42