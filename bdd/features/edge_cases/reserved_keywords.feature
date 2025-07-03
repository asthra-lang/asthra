@edge_cases
Feature: Reserved keyword enforcement
  As a language designer
  I want to ensure reserved keywords cannot be used as identifiers
  So that the language maintains clear semantic boundaries and prevents parsing ambiguities

  Background:
    Given the Asthra compiler is available

  # Test reserved keywords as variable names (should fail)
  Scenario Outline: Reserved keywords cannot be used as variable names
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let <keyword>: i32 = 42;
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "reserved keyword"

    Examples:
      | keyword            |
      | as                 |
      | fn                 |
      | let                |
      | const              |
      | mut                |
      | if                 |
      | else               |
      | enum               |
      | match              |
      | return             |
      | true               |
      | false              |
      | struct             |
      | extern             |
      | spawn              |
      | spawn_with_handle  |
      | unsafe             |
      | package            |
      | import             |
      | for                |
      | in                 |
      | await              |
      | break              |
      | continue           |
      | sizeof             |
      | pub                |
      | priv               |
      | impl               |
      | self               |
      | void               |
      | none               |
      | Result             |
      | Option             |
      | TaskHandle         |

  # Test reserved keywords as function names (should fail)
  Scenario Outline: Reserved keywords cannot be used as function names
    Given I have the following Asthra code:
      """
      package test;

      fn <keyword>(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "reserved keyword"

    Examples:
      | keyword            |
      | as                 |
      | fn                 |
      | let                |
      | const              |
      | mut                |
      | if                 |
      | else               |
      | enum               |
      | match              |
      | return             |
      | struct             |
      | extern             |
      | spawn              |
      | unsafe             |
      | package            |
      | import             |
      | for                |
      | in                 |
      | await              |
      | break              |
      | continue           |
      | sizeof             |
      | pub                |
      | priv               |
      | impl               |
      | self               |
      | void               |
      | none               |

  # Test reserved keywords as struct names (should fail)
  Scenario Outline: Reserved keywords cannot be used as struct names
    Given I have the following Asthra code:
      """
      package test;

      struct <keyword> {
          value: i32
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "reserved keyword"

    Examples:
      | keyword   |
      | as        |
      | fn        |
      | let       |
      | const     |
      | mut       |
      | if        |
      | else      |
      | enum      |
      | match     |
      | return    |
      | struct    |
      | extern    |
      | spawn     |
      | unsafe    |
      | for       |
      | in        |
      | break     |
      | continue  |
      | sizeof    |
      | impl      |
      | self      |
      | void      |
      | none      |

  # Test reserved keywords as enum names (should fail)
  Scenario Outline: Reserved keywords cannot be used as enum names
    Given I have the following Asthra code:
      """
      package test;

      enum <keyword> {
          VariantA,
          VariantB
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "reserved keyword"

    Examples:
      | keyword   |
      | as        |
      | fn        |
      | let       |
      | const     |
      | if        |
      | else      |
      | enum      |
      | match     |
      | return    |
      | struct    |
      | extern    |
      | spawn     |
      | unsafe    |
      | for       |
      | in        |
      | break     |
      | continue  |
      | sizeof    |
      | impl      |
      | self      |
      | void      |
      | none      |

  # Test reserved keywords as field names (should fail)
  Scenario Outline: Reserved keywords cannot be used as struct field names
    Given I have the following Asthra code:
      """
      package test;

      struct MyStruct {
          <keyword>: i32
      }

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "reserved keyword"

    Examples:
      | keyword            |
      | as                 |
      | fn                 |
      | let                |
      | const              |
      | mut                |
      | if                 |
      | else               |
      | enum               |
      | match              |
      | return             |
      | true               |
      | false              |
      | struct             |
      | extern             |
      | spawn              |
      | spawn_with_handle  |
      | unsafe             |
      | package            |
      | import             |
      | for                |
      | in                 |
      | await              |
      | break              |
      | continue           |
      | sizeof             |
      | pub                |
      | priv               |
      | impl               |
      | self               |
      | void               |
      | none               |
      | Result             |
      | Option             |
      | TaskHandle         |

  # Test that predeclared identifiers CAN be used as identifiers (shadowing allowed)
  Scenario: Predeclared identifier 'log' can be used as variable name
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let log: i32 = 42;
          return log;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Predeclared identifier 'range' can be used as variable name
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let range: i32 = 10;
          return range;
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  Scenario: Predeclared identifier 'panic' can be used as function name
    Given I have the following Asthra code:
      """
      package test;

      fn panic(value: i32) -> i32 {
          return value * 2;
      }

      pub fn main(none) -> i32 {
          let result: i32 = panic(21);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Predeclared identifier 'exit' can be used as struct name
    Given I have the following Asthra code:
      """
      package test;

      struct exit {
          code: i32
      }

      pub fn main(none) -> i32 {
          let e: exit = exit { code: 5 };
          return e.code;
      }
      """
    When I compile and run the program
    Then the exit code should be 5

  Scenario: Predeclared identifier 'args' can be used as enum name
    Given I have the following Asthra code:
      """
      package test;

      enum args {
          Empty,
          WithValue(i32)
      }

      pub fn main(none) -> i32 {
          let a: args = args::WithValue(7);
          match a {
              args::Empty => return 0;,
              args::WithValue(v) => return v;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 7

  Scenario: Predeclared identifier 'len' can be used as field name
    Given I have the following Asthra code:
      """
      package test;

      struct Vector {
          len: i32,
          capacity: i32
      }

      pub fn main(none) -> i32 {
          let v: Vector = Vector { len: 15, capacity: 30 };
          return v.len;
      }
      """
    When I compile and run the program
    Then the exit code should be 15

  Scenario: Multiple predeclared identifiers can be shadowed in same scope
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let log: i32 = 1;
          let range: i32 = 2;
          let panic: i32 = 3;
          let exit: i32 = 4;
          let args: i32 = 5;
          let len: i32 = 6;
          let infinite: i32 = 7;
          
          return log + range + panic + exit + args + len + infinite;
      }
      """
    When I compile and run the program
    Then the exit code should be 28