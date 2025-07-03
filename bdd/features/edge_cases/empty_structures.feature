@edge_cases
Feature: Empty Structures Using 'none' Keyword
  As an Asthra language user
  I want to properly handle empty structures using the 'none' keyword
  So that I can define and use empty structs, enums, and functions correctly

  Background:
    Given the Asthra compiler is ready

  Scenario: Empty struct declaration using 'none'
    Given a source file with content:
      """
      package test;
      
      pub struct EmptyStruct { none }
      
      pub fn main() -> void {
          let empty: EmptyStruct = EmptyStruct {};
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should execute successfully

  Scenario: Empty enum declaration using 'none'
    Given a source file with content:
      """
      package test;
      
      pub enum EmptyEnum { none }
      
      pub fn main() -> void {
          // Empty enum cannot be instantiated
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should execute successfully

  Scenario: Function with no parameters using 'none'
    Given a source file with content:
      """
      package test;
      
      pub fn greet(none) -> string {
          return "Hello, World!";
      }
      
      pub fn main() -> void {
          let message: string = greet(none);
          log(message);
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output "Hello, World!"

  Scenario: Empty array literal using 'none'
    Given a source file with content:
      """
      package test;
      
      pub fn main() -> void {
          let empty_array: []i32 = [none];
          let length: usize = len(empty_array);
          log("Array length: " + string(length));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output "Array length: 0"

  Scenario: Mixing empty and non-empty structs
    Given a source file with content:
      """
      package test;
      
      pub struct Empty { none }
      pub struct Person {
          pub name: string,
          pub age: i32
      }
      
      pub fn process_empty(e: Empty) -> string {
          return "Processing empty struct";
      }
      
      pub fn process_person(p: Person) -> string {
          return "Processing " + p.name;
      }
      
      pub fn main() -> void {
          let e: Empty = Empty {};
          let p: Person = Person { name: "Alice", age: 30 };
          
          log(process_empty(e));
          log(process_person(p));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output:
      """
      Processing empty struct
      Processing Alice
      """

  Scenario: Enum with both empty and non-empty variants
    Given a source file with content:
      """
      package test;
      
      pub enum Status {
          Empty,
          Loading,
          Loaded(string)
      }
      
      pub fn check_status(s: Status) -> string {
          match s {
              Status.Empty => { return "Empty status"; }
              Status.Loading => { return "Loading..."; }
              Status.Loaded(data) => { return "Loaded: " + data; }
          }
      }
      
      pub fn main() -> void {
          let s1: Status = Status.Empty(none);
          let s2: Status = Status.Loading(none);
          let s3: Status = Status.Loaded("Data ready");
          
          log(check_status(s1));
          log(check_status(s2));
          log(check_status(s3));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output:
      """
      Empty status
      Loading...
      Loaded: Data ready
      """

  Scenario: Pattern matching with 'none' in enum patterns
    Given a source file with content:
      """
      package test;
      
      pub enum Message {
          Text(string),
          Number(i32),
          Empty
      }
      
      pub fn process_message(msg: Message) -> string {
          match msg {
              Message.Text(s) => { return "Text: " + s; }
              Message.Number(n) => { return "Number: " + string(n); }
              Message.Empty(none) => { return "Empty message"; }
          }
      }
      
      pub fn main() -> void {
          let m1: Message = Message.Text("Hello");
          let m2: Message = Message.Number(42);
          let m3: Message = Message.Empty(none);
          
          log(process_message(m1));
          log(process_message(m2));
          log(process_message(m3));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output:
      """
      Text: Hello
      Number: 42
      Empty message
      """

  Scenario: External function with no parameters using 'none'
    Given a source file with content:
      """
      package test;
      
      extern "C" fn get_timestamp(none) -> i64;
      
      pub fn main() -> void {
          let ts: i64 = get_timestamp(none);
          log("Timestamp received (non-zero indicates success): " + string(ts != 0));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output contains "Timestamp received"

  Scenario: Method with empty parameter list using 'none'
    Given a source file with content:
      """
      package test;
      
      pub struct Counter {
          priv value: i32
      }
      
      impl Counter {
          pub fn new(none) -> Counter {
              return Counter { value: 0 };
          }
          
          pub fn increment(self) -> void {
              // Note: Cannot mutate self as it's immutable
              return ();
          }
          
          pub fn get_value(self) -> i32 {
              return self.value;
          }
      }
      
      pub fn main() -> void {
          let c: Counter = Counter::new(none);
          log("Counter value: " + string(c.get_value()));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output "Counter value: 0"

  Scenario: Empty struct pattern matching
    Given a source file with content:
      """
      package test;
      
      pub struct Empty { none }
      pub struct Container<T> {
          pub value: T
      }
      
      pub fn process_container(c: Container<Empty>) -> string {
          let Container { value: _ } = c;
          return "Processed empty container";
      }
      
      pub fn main() -> void {
          let empty: Empty = Empty {};
          let container: Container<Empty> = Container { value: empty };
          log(process_container(container));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output "Processed empty container"

  Scenario: Function returning void with explicit unit return
    Given a source file with content:
      """
      package test;
      
      pub fn do_nothing(none) -> void {
          // Explicit unit return as required by grammar
          return ();
      }
      
      pub fn main() -> void {
          do_nothing(none);
          log("Function with 'none' parameters executed");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output "Function with 'none' parameters executed"

  Scenario: Comparison of 'none' usage vs actual empty content
    Given a source file with content:
      """
      package test;
      
      // Using 'none' marker
      pub struct EmptyWithNone { none }
      pub fn func_with_none(none) -> i32 { return 42; }
      
      // Functions always require explicit parameter list
      // pub fn func_no_params() -> i32 { return 42; }  // This would be invalid
      
      pub fn main() -> void {
          let e1: EmptyWithNone = EmptyWithNone {};
          let result: i32 = func_with_none(none);
          
          log("Empty struct created: true");
          log("Function result: " + string(result));
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the program should output:
      """
      Empty struct created: true
      Function result: 42
      """