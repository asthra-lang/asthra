# String Module

**Package**: `stdlib::string`  
**Import**: `import "stdlib/string";`  
**Status**: ✅ Complete  

## Overview

The string module provides comprehensive string processing capabilities including UTF-8 validation, text manipulation, searching, formatting, and conversion utilities. All functions are designed to handle Unicode text correctly and provide explicit error handling through `Result<T, E>` types.

## Quick Start

```asthra
import "stdlib/string";

fn main() -> i32 {
    // UTF-8 validation and conversion
    let data: []u8 = [72, 101, 108, 108, 111]; // "Hello" in UTF-8
    let string_result: Result<string, string.StringError> = string.from_utf8(data);
    
    match string_result {
        Result.Ok(text) => {
            log("Valid UTF-8: " + text);
        },
        Result.Err(error) => {
            log("Invalid UTF-8: " + error);
        }
    }
    
    return 0;
}
```

## Core Functions

### UTF-8 Conversion

#### `from_utf8(data: []u8) -> Result<string, StringError>`
Converts a byte array to a string, validating UTF-8 encoding.

```asthra
let data: []u8 = [72, 101, 108, 108, 111]; // "Hello"
let text: string = string.from_utf8_lossy(data);
log("Text: " + text); // Output: "Text: Hello"
```

#### `to_utf8(text: string) -> []u8`
Converts a string to its UTF-8 byte representation.

```asthra
let text: string = "Hello, 世界";
let bytes: []u8 = string.to_utf8(text);
log("Byte count: " + bytes.len); // Output: "Byte count: 13"
```

### Length and Character Counting

#### `len(text: string) -> usize`
Returns the byte length of a string (UTF-8 bytes, not characters).

#### `char_count(text: string) -> usize`
Returns the number of Unicode characters in a string.

```asthra
let ascii: string = "Hello";
let unicode: string = "Hello, 世界";

let ascii_bytes: usize = string.len(ascii);      // 5 bytes
let unicode_bytes: usize = string.len(unicode);  // 13 bytes
let ascii_chars: usize = string.char_count(ascii);   // 5 characters
let unicode_chars: usize = string.char_count(unicode); // 9 characters

log("ASCII: " + ascii_bytes + " bytes, " + ascii_chars + " chars");
log("Unicode: " + unicode_bytes + " bytes, " + unicode_chars + " chars");
```

### String Properties

#### `is_empty(text: string) -> bool`
Checks if a string is empty (zero length).

#### `len(text: string) -> usize`
Returns the byte length of the string.

#### `char_count(text: string) -> usize`
Returns the number of Unicode characters.

```asthra
let text: string = "Hello, 世界";
let byte_len: usize = string.len(text);
let char_count: usize = string.char_count(text);

log("'" + text + "' has " + char_count + " characters in " + byte_len + " bytes");
```

### Whitespace and Validation

#### `trim(text: string) -> string`
Removes leading and trailing whitespace.

#### `is_empty(text: string) -> bool`
Checks if string is empty after trimming.

```asthra
fn validate_input(input: string) -> Result<string, string> {
    if string.is_empty(input) {
        return Result.Err("Input cannot be empty");
    }
    
    let trimmed: string = string.trim(input);
    if string.is_empty(trimmed) {
        return Result.Err("Input cannot be only whitespace");
    }
    
    return Result.Ok(trimmed);
}
```

### Character Set Validation

#### `is_ascii(text: string) -> bool`
Checks if all characters in the string are ASCII (0-127).

```asthra
let ascii_text: string = "Hello World";
let unicode_text: string = "Hello 世界";

if string.is_ascii(text) {
    log("Text contains only ASCII characters");
} else {
    log("Text contains Unicode characters");
}
```

### Search Operations

#### `contains(text: string, substring: string) -> bool`
Checks if the string contains the given substring.

```asthra
let messages: []string = ["error: file not found", "warning: deprecated", "info: success"];

for i in range(0, messages.len) {
    if string.contains(messages[i], keyword) {
        log("Found message: " + messages[i]);
    }
}
```

#### `starts_with(text: string, prefix: string) -> bool`
Checks if the string starts with the given prefix.

#### `ends_with(text: string, suffix: string) -> bool`
Checks if the string ends with the given suffix.

```asthra
fn categorize_file(filename: string) -> string {
    if string.starts_with(filename, "test_") {
        return "test file";
    } else if string.starts_with(filename, "lib_") {
        return "library file";
    } else {
        return "regular file";
    }
}

fn check_file_type(filename: string) -> string {
    if string.ends_with(filename, ".asthra") {
        return "Asthra source file";
    } else if string.ends_with(filename, ".md") {
        return "Markdown documentation";
    } else {
        return "unknown file type";
    }
}
```

#### `find(text: string, substring: string) -> Result<usize, string>`
Finds the first occurrence of a substring and returns its byte position.

```asthra
let email: string = "user@example.com";
let at_pos_result: Result<usize, string> = string.find(email, "@");

match at_pos_result {
    Result.Ok(at_pos) => {
        let username: string = string.substring(email, 0, at_pos);
        let domain: string = string.substring(email, at_pos + 1, string.len(email));
        
        if string.is_empty(domain) {
            log("Invalid email: missing domain");
        } else {
            log("Username: " + username + ", Domain: " + domain);
        }
    },
    Result.Err(error) => {
        log("Invalid email format: " + error);
    }
}
```

### Text Transformation

#### `trim(text: string) -> string`
Removes leading and trailing whitespace characters.

```asthra
let input: string = "  hello world  ";
let cleaned: string = string.trim(input);
log("'" + cleaned + "'"); // Output: "'hello world'"
```

#### `to_lowercase(text: string) -> string`
Converts all characters to lowercase.

#### `to_uppercase(text: string) -> string`
Converts all characters to uppercase.

```asthra
let text: string = "Hello World";
let lower: string = string.to_lowercase(text);
log(lower); // Output: "hello world"

let upper: string = string.to_uppercase(text);
log(upper); // Output: "HELLO WORLD"
```

### String Replacement

#### `replace(text: string, from: string, to: string) -> string`
Replaces the first occurrence of a substring.

```asthra
let text: string = "The quick brown fox jumps over teh lazy dog";
let fixed: string = string.replace(text, "teh", "the");
log(fixed); // Output: "The quick brown fox jumps over the lazy dog"
```

#### `replace_all(text: string, from: string, to: string) -> string`
Replaces all occurrences of a substring.

```asthra
fn sanitize_filename(filename: string) -> string {
    let mut sanitized: string = filename;
    
    // Replace problematic characters
    sanitized = string.replace_all(sanitized, " ", "_");
    sanitized = string.replace_all(sanitized, "/", "-");
    sanitized = string.replace_all(sanitized, "\\", "-");
    
    return sanitized;
}
```

### String Splitting

#### `split(text: string, delimiter: string) -> []string`
Splits a string by a delimiter into an array of substrings.

```asthra
fn parse_csv_line(line: string) -> []string {
    let fields: []string = string.split(line, ",");
    
    // Trim whitespace from each field
    for i in range(0, fields.len) {
        fields[i] = string.trim(fields[i]);
    }
    
    return fields;
}
```

#### `split_whitespace(text: string) -> []string`
Splits a string by any whitespace characters.

```asthra
fn parse_command(input: string) -> []string {
    let args: []string = string.split_whitespace(input);
    return args;
}
```

#### `lines(text: string) -> []string`
Splits text into lines (by newline characters).

```asthra
fn process_text_file(content: string) -> []string {
    let lines: []string = string.lines(content);
    let mut valid_lines: []string = [];
    
    for i in range(0, lines.len) {
        let line: string = string.trim(lines[i]);
        if !string.is_empty(line) {
            valid_lines.push(line);
        }
    }
    
    return valid_lines;
}
```

### String Joining

#### `join(parts: []string, separator: string) -> string`
Joins an array of strings with a separator.

```asthra
let components: []string = ["usr", "local", "bin"];
let path: string = string.join(components, "/");
log(path); // Output: "usr/local/bin"
```

```asthra
fn format_list(items: []string) -> string {
    if items.len == 0 {
        return "empty list";
    } else if items.len == 1 {
        return items[0];
    } else {
        let all_but_last: []string = items[0..items.len-1];
        let joined: string = string.join(all_but_last, ", ");
        return joined + " and " + items[items.len-1];
    }
}
```

### String Formatting

#### `format(template: string, args: []string) -> string`
Simple string formatting with positional arguments.

```asthra
fn create_user_message(username: string, action: string, timestamp: string) -> string {
    let template: string = "User {0} performed {1} at {2}";
    let args: []string = [username, action, timestamp];
    
    let message: string = string.format(
        template,
        args
    );
    
    return message;
}
```

### String Manipulation

#### `substring(text: string, start: usize, end: usize) -> string`
Extracts a substring from start to end position (byte positions).

```asthra
fn truncate_with_ellipsis(text: string, max_length: usize) -> string {
    let text_len: usize = string.len(text);
    
    if text_len <= max_length {
        return text;
    } else {
        let truncated: string = string.substring(text, 0, max_length - 3);
        return truncated + "...";
    }
}
```

#### `repeat(text: string, count: usize) -> string`
Repeats a string a specified number of times.

```asthra
fn create_separator(width: usize) -> string {
    let separator: string = string.repeat("-", width);
    return separator;
}

fn indent_text(text: string, level: usize) -> string {
    let indent: string = string.repeat("  ", level);
    return indent + text;
}
```

#### `pad_left(text: string, width: usize, fill_char: char) -> string`
Pads a string on the left to reach the specified width.

#### `pad_right(text: string, width: usize, fill_char: char) -> string`
Pads a string on the right to reach the specified width.

```asthra
fn format_number(number: i32, width: usize) -> string {
    let num_str: string = number.to_string();
    let padded: string = string.pad_left(num_str, width, '0');
    return padded;
}

fn align_text(text: string, width: usize) -> string {
    return string.pad_left(text, width, ' ');
}

fn format_table_row(columns: []string, widths: []usize) -> string {
    let mut formatted: []string = [];
    
    for i in range(0, columns.len) {
        let padded: string = string.pad_right(columns[i], widths[i], ' ');
        formatted.push(padded);
    }
    
    return string.join(formatted, " | ");
}
```

## Error Handling Examples

### Strict UTF-8 Validation

```asthra
fn process_external_data(external_data: []u8) -> Result<string, string> {
    let strict_result: Result<string, string.StringError> = string.from_utf8(external_data);
    
    match strict_result {
        Result.Ok(valid_text) => {
            return Result.Ok(valid_text);
        },
        Result.Err(utf8_error) => {
            // Fall back to lossy conversion
            let lossy_text: string = string.from_utf8_lossy(external_data);
            log("Warning: Invalid UTF-8 detected, using lossy conversion");
            return Result.Ok(lossy_text);
        }
    }
}
```

### Comprehensive Text Analysis

```asthra
fn analyze_text(text: string) -> string {
    let mut analysis: []string = [];
    
    let chars: usize = string.char_count(text);
    analysis.push("Characters: " + chars);
    
    let bytes: usize = string.len(text);
    analysis.push("Bytes: " + bytes);
    
    let has_comma: bool = string.contains(text, ",");
    analysis.push("Contains comma: " + has_comma);
    
    let comma_pos: Result<usize, string> = string.find(text, ",");
    match comma_pos {
        Result.Ok(pos) => {
            analysis.push("First comma at byte: " + pos);
        },
        Result.Err(_) => {
            analysis.push("No comma found");
        }
    }
    
    return string.join(analysis, "\n");
}
```

### Safe Text Processing

```asthra
fn clean_text_lines(input: string) -> string {
    let lines: []string = string.lines(input);
    let mut result_parts: []string = [];
    
    for i in range(0, lines.len) {
        let line: string = lines[i];
        if !string.is_empty(string.trim(line)) {
            result_parts.push(string.trim(line));
        }
    }
    
    return string.join(result_parts, "\n");
}
```

## Summary

The `stdlib::string` module provides:

- **UTF-8 Safety**: Explicit validation and conversion functions
- **Unicode Support**: Proper character counting and text processing
- **Comprehensive Search**: Find, contains, starts/ends with operations
- **Text Transformation**: Case conversion, trimming, replacement
- **Flexible Splitting/Joining**: Multiple ways to break apart and combine strings
- **Error Handling**: All potentially failing operations return `Result<T, E>`
- **Performance**: Efficient implementations optimized for common use cases

All string operations are designed to handle Unicode text correctly while providing clear error handling for edge cases and invalid input. 
