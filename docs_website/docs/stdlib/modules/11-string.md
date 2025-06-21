# String Module

**Package**: `stdlib::string`  
**Import**: `import "stdlib/string";`  
**Status**: ✅ Complete  

## Overview

The string module provides comprehensive string processing capabilities including UTF-8 validation, text manipulation, searching, formatting, and conversion utilities. All functions are designed to handle Unicode text correctly and provide explicit error handling through `Result&lt;T, E&gt;` types.

## Quick Start

```asthra
import "stdlib/string";

fn main() -&gt; i32 {
    // UTF-8 validation and conversion
    let data: &#91;&#93;u8 = &#91;72, 101, 108, 108, 111&#93;; // "Hello" in UTF-8
    let string_result: Result&lt;string, string.StringError&gt; = string.from_utf8(data);
    
    match string_result {
        Result.Ok(text) =&gt; {
            log("Valid UTF-8: " + text);
        },
        Result.Err(error) =&gt; {
            log("Invalid UTF-8: " + error);
        }
    }
    
    return 0;
}
```

## Core Functions

### UTF-8 Conversion

#### `from_utf8(data: &#91;&#93;u8) -&gt; Result&lt;string, StringError&gt;`
Converts a byte array to a string, validating UTF-8 encoding.

```asthra
let data: &#91;&#93;u8 = &#91;72, 101, 108, 108, 111&#93;; // "Hello"
let text: string = string.from_utf8_lossy(data);
log("Text: " + text); // Output: "Text: Hello"
```

#### `to_utf8(text: string) -&gt; &#91;&#93;u8`
Converts a string to its UTF-8 byte representation.

```asthra
let text: string = "Hello, 世界";
let bytes: &#91;&#93;u8 = string.to_utf8(text);
log("Byte count: " + bytes.len); // Output: "Byte count: 13"
```

### Length and Character Counting

#### `len(text: string) -&gt; usize`
Returns the byte length of a string (UTF-8 bytes, not characters).

#### `char_count(text: string) -&gt; usize`
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

#### `is_empty(text: string) -&gt; bool`
Checks if a string is empty (zero length).

#### `len(text: string) -&gt; usize`
Returns the byte length of the string.

#### `char_count(text: string) -&gt; usize`
Returns the number of Unicode characters.

```asthra
let text: string = "Hello, 世界";
let byte_len: usize = string.len(text);
let char_count: usize = string.char_count(text);

log("'" + text + "' has " + char_count + " characters in " + byte_len + " bytes");
```

### Whitespace and Validation

#### `trim(text: string) -&gt; string`
Removes leading and trailing whitespace.

#### `is_empty(text: string) -&gt; bool`
Checks if string is empty after trimming.

```asthra
fn validate_input(input: string) -&gt; Result&lt;string, string&gt; {
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

#### `is_ascii(text: string) -&gt; bool`
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

#### `contains(text: string, substring: string) -&gt; bool`
Checks if the string contains the given substring.

```asthra
let messages: &#91;&#93;string = &#91;"error: file not found", "warning: deprecated", "info: success"&#93;;

for i in range(0, messages.len) {
    if string.contains(messages&#91;i&#93;, keyword) {
        log("Found message: " + messages&#91;i&#93;);
    }
}
```

#### `starts_with(text: string, prefix: string) -&gt; bool`
Checks if the string starts with the given prefix.

#### `ends_with(text: string, suffix: string) -&gt; bool`
Checks if the string ends with the given suffix.

```asthra
fn categorize_file(filename: string) -&gt; string {
    if string.starts_with(filename, "test_") {
        return "test file";
    } else if string.starts_with(filename, "lib_") {
        return "library file";
    } else {
        return "regular file";
    }
}

fn check_file_type(filename: string) -&gt; string {
    if string.ends_with(filename, ".asthra") {
        return "Asthra source file";
    } else if string.ends_with(filename, ".md") {
        return "Markdown documentation";
    } else {
        return "unknown file type";
    }
}
```

#### `find(text: string, substring: string) -&gt; Result&lt;usize, string&gt;`
Finds the first occurrence of a substring and returns its byte position.

```asthra
let email: string = "user@example.com";
let at_pos_result: Result&lt;usize, string&gt; = string.find(email, "@");

match at_pos_result {
    Result.Ok(at_pos) =&gt; {
        let username: string = string.substring(email, 0, at_pos);
        let domain: string = string.substring(email, at_pos + 1, string.len(email));
        
        if string.is_empty(domain) {
            log("Invalid email: missing domain");
        } else {
            log("Username: " + username + ", Domain: " + domain);
        }
    },
    Result.Err(error) =&gt; {
        log("Invalid email format: " + error);
    }
}
```

### Text Transformation

#### `trim(text: string) -&gt; string`
Removes leading and trailing whitespace characters.

```asthra
let input: string = "  hello world  ";
let cleaned: string = string.trim(input);
log("'" + cleaned + "'"); // Output: "'hello world'"
```

#### `to_lowercase(text: string) -&gt; string`
Converts all characters to lowercase.

#### `to_uppercase(text: string) -&gt; string`
Converts all characters to uppercase.

```asthra
let text: string = "Hello World";
let lower: string = string.to_lowercase(text);
log(lower); // Output: "hello world"

let upper: string = string.to_uppercase(text);
log(upper); // Output: "HELLO WORLD"
```

### String Replacement

#### `replace(text: string, from: string, to: string) -&gt; string`
Replaces the first occurrence of a substring.

```asthra
let text: string = "The quick brown fox jumps over teh lazy dog";
let fixed: string = string.replace(text, "teh", "the");
log(fixed); // Output: "The quick brown fox jumps over the lazy dog"
```

#### `replace_all(text: string, from: string, to: string) -&gt; string`
Replaces all occurrences of a substring.

```asthra
fn sanitize_filename(filename: string) -&gt; string {
    let mut sanitized: string = filename;
    
    // Replace problematic characters
    sanitized = string.replace_all(sanitized, " ", "_");
    sanitized = string.replace_all(sanitized, "/", "-");
    sanitized = string.replace_all(sanitized, "\\", "-");
    
    return sanitized;
}
```

### String Splitting

#### `split(text: string, delimiter: string) -&gt; &#91;&#93;string`
Splits a string by a delimiter into an array of substrings.

```asthra
fn parse_csv_line(line: string) -&gt; &#91;&#93;string {
    let fields: &#91;&#93;string = string.split(line, ",");
    
    // Trim whitespace from each field
    for i in range(0, fields.len) {
        fields&#91;i&#93; = string.trim(fields&#91;i&#93;);
    }
    
    return fields;
}
```

#### `split_whitespace(text: string) -&gt; &#91;&#93;string`
Splits a string by any whitespace characters.

```asthra
fn parse_command(input: string) -&gt; &#91;&#93;string {
    let args: &#91;&#93;string = string.split_whitespace(input);
    return args;
}
```

#### `lines(text: string) -&gt; &#91;&#93;string`
Splits text into lines (by newline characters).

```asthra
fn process_text_file(content: string) -&gt; &#91;&#93;string {
    let lines: &#91;&#93;string = string.lines(content);
    let mut valid_lines: &#91;&#93;string = &#91;&#93;;
    
    for i in range(0, lines.len) {
        let line: string = string.trim(lines&#91;i&#93;);
        if !string.is_empty(line) {
            valid_lines.push(line);
        }
    }
    
    return valid_lines;
}
```

### String Joining

#### `join(parts: &#91;&#93;string, separator: string) -&gt; string`
Joins an array of strings with a separator.

```asthra
let components: &#91;&#93;string = &#91;"usr", "local", "bin"&#93;;
let path: string = string.join(components, "/");
log(path); // Output: "usr/local/bin"
```

```asthra
fn format_list(items: &#91;&#93;string) -&gt; string {
    if items.len == 0 {
        return "empty list";
    } else if items.len == 1 {
        return items&#91;0&#93;;
    } else {
        let all_but_last: &#91;&#93;string = items&#91;0..items.len-1&#93;;
        let joined: string = string.join(all_but_last, ", ");
        return joined + " and " + items&#91;items.len-1&#93;;
    }
}
```

### String Formatting

#### `format(template: string, args: &#91;&#93;string) -&gt; string`
Simple string formatting with positional arguments.

```asthra
fn create_user_message(username: string, action: string, timestamp: string) -&gt; string {
    let template: string = "User {0} performed {1} at {2}";
    let args: &#91;&#93;string = &#91;username, action, timestamp&#93;;
    
    let message: string = string.format(
        template,
        args
    );
    
    return message;
}
```

### String Manipulation

#### `substring(text: string, start: usize, end: usize) -&gt; string`
Extracts a substring from start to end position (byte positions).

```asthra
fn truncate_with_ellipsis(text: string, max_length: usize) -&gt; string {
    let text_len: usize = string.len(text);
    
    if text_len &lt;= max_length {
        return text;
    } else {
        let truncated: string = string.substring(text, 0, max_length - 3);
        return truncated + "...";
    }
}
```

#### `repeat(text: string, count: usize) -&gt; string`
Repeats a string a specified number of times.

```asthra
fn create_separator(width: usize) -&gt; string {
    let separator: string = string.repeat("-", width);
    return separator;
}

fn indent_text(text: string, level: usize) -&gt; string {
    let indent: string = string.repeat("  ", level);
    return indent + text;
}
```

#### `pad_left(text: string, width: usize, fill_char: char) -&gt; string`
Pads a string on the left to reach the specified width.

#### `pad_right(text: string, width: usize, fill_char: char) -&gt; string`
Pads a string on the right to reach the specified width.

```asthra
fn format_number(number: i32, width: usize) -&gt; string {
    let num_str: string = number.to_string();
    let padded: string = string.pad_left(num_str, width, '0');
    return padded;
}

fn align_text(text: string, width: usize) -&gt; string {
    return string.pad_left(text, width, ' ');
}

fn format_table_row(columns: &#91;&#93;string, widths: &#91;&#93;usize) -&gt; string {
    let mut formatted: &#91;&#93;string = &#91;&#93;;
    
    for i in range(0, columns.len) {
        let padded: string = string.pad_right(columns&#91;i&#93;, widths&#91;i&#93;, ' ');
        formatted.push(padded);
    }
    
    return string.join(formatted, " | ");
}
```

## Error Handling Examples

### Strict UTF-8 Validation

```asthra
fn process_external_data(external_data: &#91;&#93;u8) -&gt; Result&lt;string, string&gt; {
    let strict_result: Result&lt;string, string.StringError&gt; = string.from_utf8(external_data);
    
    match strict_result {
        Result.Ok(valid_text) =&gt; {
            return Result.Ok(valid_text);
        },
        Result.Err(utf8_error) =&gt; {
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
fn analyze_text(text: string) -&gt; string {
    let mut analysis: &#91;&#93;string = &#91;&#93;;
    
    let chars: usize = string.char_count(text);
    analysis.push("Characters: " + chars);
    
    let bytes: usize = string.len(text);
    analysis.push("Bytes: " + bytes);
    
    let has_comma: bool = string.contains(text, ",");
    analysis.push("Contains comma: " + has_comma);
    
    let comma_pos: Result&lt;usize, string&gt; = string.find(text, ",");
    match comma_pos {
        Result.Ok(pos) =&gt; {
            analysis.push("First comma at byte: " + pos);
        },
        Result.Err(_) =&gt; {
            analysis.push("No comma found");
        }
    }
    
    return string.join(analysis, "\n");
}
```

### Safe Text Processing

```asthra
fn clean_text_lines(input: string) -&gt; string {
    let lines: &#91;&#93;string = string.lines(input);
    let mut result_parts: &#91;&#93;string = &#91;&#93;;
    
    for i in range(0, lines.len) {
        let line: string = lines&#91;i&#93;;
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
- **Error Handling**: All potentially failing operations return `Result&lt;T, E&gt;`
- **Performance**: Efficient implementations optimized for common use cases

All string operations are designed to handle Unicode text correctly while providing clear error handling for edge cases and invalid input. 
