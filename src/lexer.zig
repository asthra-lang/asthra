const std = @import("std");
const Token = @import("token.zig").Token;
const Tag = @import("token.zig").Tag;
const getKeyword = @import("token.zig").getKeyword;

pub const Lexer = struct {
    source: []const u8,
    index: u32,

    pub fn init(source: []const u8) Lexer {
        return .{
            .source = source,
            .index = 0,
        };
    }

    pub fn next(self: *Lexer) Token {
        self.skipWhitespaceAndComments();

        const start = self.index;

        if (self.isAtEnd()) {
            return .{ .tag = .eof, .loc = .{ .start = start, .end = start } };
        }

        const c = self.peek();

        // Identifiers and keywords
        if (isAlpha(c)) {
            return self.readIdentifier();
        }

        // Numbers
        if (isDigit(c)) {
            return self.readNumber();
        }

        // Strings
        if (c == '"') {
            return self.readString();
        }

        // Characters
        if (c == '\'') {
            return self.readChar();
        }

        // Operators and delimiters
        return self.readOperator();
    }

    fn readIdentifier(self: *Lexer) Token {
        const start = self.index;
        while (!self.isAtEnd() and (isAlpha(self.peek()) or isDigit(self.peek()))) {
            self.advance();
        }
        const text = self.source[start..self.index];
        const tag = getKeyword(text) orelse .identifier;
        return .{ .tag = tag, .loc = .{ .start = start, .end = self.index } };
    }

    fn readNumber(self: *Lexer) Token {
        const start = self.index;

        // Check for hex, binary, octal prefixes
        if (self.peek() == '0' and !self.isAtEndOffset(1)) {
            const next_c = self.peekOffset(1);
            if (next_c == 'x' or next_c == 'X') {
                self.advance(); // 0
                self.advance(); // x
                while (!self.isAtEnd() and isHexDigit(self.peek())) {
                    self.advance();
                }
                return .{ .tag = .int_literal, .loc = .{ .start = start, .end = self.index } };
            }
            if (next_c == 'b' or next_c == 'B') {
                self.advance(); // 0
                self.advance(); // b
                while (!self.isAtEnd() and (self.peek() == '0' or self.peek() == '1')) {
                    self.advance();
                }
                return .{ .tag = .int_literal, .loc = .{ .start = start, .end = self.index } };
            }
            if (next_c == 'o') {
                self.advance(); // 0
                self.advance(); // o
                while (!self.isAtEnd() and self.peek() >= '0' and self.peek() <= '7') {
                    self.advance();
                }
                return .{ .tag = .int_literal, .loc = .{ .start = start, .end = self.index } };
            }
        }

        // Decimal digits
        while (!self.isAtEnd() and isDigit(self.peek())) {
            self.advance();
        }

        // Check for float
        if (!self.isAtEnd() and self.peek() == '.') {
            // Look ahead to make sure it's not a method call like `123.method`
            if (!self.isAtEndOffset(1) and isDigit(self.peekOffset(1))) {
                self.advance(); // .
                while (!self.isAtEnd() and isDigit(self.peek())) {
                    self.advance();
                }
                return .{ .tag = .float_literal, .loc = .{ .start = start, .end = self.index } };
            }
        }

        return .{ .tag = .int_literal, .loc = .{ .start = start, .end = self.index } };
    }

    fn readString(self: *Lexer) Token {
        const start = self.index;
        self.advance(); // opening "

        // Check for triple-quoted strings
        if (!self.isAtEnd() and self.peek() == '"' and !self.isAtEndOffset(1) and self.peekOffset(1) == '"') {
            self.advance(); // second "
            self.advance(); // third "
            // Read until closing """
            while (!self.isAtEnd()) {
                if (self.peek() == '"' and !self.isAtEndOffset(1) and self.peekOffset(1) == '"' and !self.isAtEndOffset(2) and self.peekOffset(2) == '"') {
                    self.advance(); // "
                    self.advance(); // "
                    self.advance(); // "
                    return .{ .tag = .string_literal, .loc = .{ .start = start, .end = self.index } };
                }
                self.advance();
            }
            // Unterminated triple-quoted string
            return .{ .tag = .invalid, .loc = .{ .start = start, .end = self.index } };
        }

        // Regular string
        while (!self.isAtEnd() and self.peek() != '"' and self.peek() != '\n') {
            if (self.peek() == '\\') {
                self.advance(); // backslash
                if (!self.isAtEnd()) self.advance(); // escaped char
            } else {
                self.advance();
            }
        }

        if (self.isAtEnd() or self.peek() == '\n') {
            return .{ .tag = .invalid, .loc = .{ .start = start, .end = self.index } };
        }

        self.advance(); // closing "
        return .{ .tag = .string_literal, .loc = .{ .start = start, .end = self.index } };
    }

    fn readChar(self: *Lexer) Token {
        const start = self.index;
        self.advance(); // opening '

        if (self.isAtEnd()) {
            return .{ .tag = .invalid, .loc = .{ .start = start, .end = self.index } };
        }

        if (self.peek() == '\\') {
            self.advance(); // backslash
            if (!self.isAtEnd()) self.advance(); // escaped char
        } else {
            self.advance(); // regular char
        }

        if (self.isAtEnd() or self.peek() != '\'') {
            return .{ .tag = .invalid, .loc = .{ .start = start, .end = self.index } };
        }

        self.advance(); // closing '
        return .{ .tag = .char_literal, .loc = .{ .start = start, .end = self.index } };
    }

    fn readOperator(self: *Lexer) Token {
        const start = self.index;
        const c = self.peek();
        self.advance();

        switch (c) {
            '(' => return .{ .tag = .lparen, .loc = .{ .start = start, .end = self.index } },
            ')' => return .{ .tag = .rparen, .loc = .{ .start = start, .end = self.index } },
            '{' => return .{ .tag = .lbrace, .loc = .{ .start = start, .end = self.index } },
            '}' => return .{ .tag = .rbrace, .loc = .{ .start = start, .end = self.index } },
            '[' => return .{ .tag = .lbracket, .loc = .{ .start = start, .end = self.index } },
            ']' => return .{ .tag = .rbracket, .loc = .{ .start = start, .end = self.index } },
            ';' => return .{ .tag = .semicolon, .loc = .{ .start = start, .end = self.index } },
            ':' => {
                if (!self.isAtEnd() and self.peek() == ':') {
                    self.advance();
                    return .{ .tag = .double_colon, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .colon, .loc = .{ .start = start, .end = self.index } };
            },
            ',' => return .{ .tag = .comma, .loc = .{ .start = start, .end = self.index } },
            '.' => return .{ .tag = .dot, .loc = .{ .start = start, .end = self.index } },
            '+' => return .{ .tag = .plus, .loc = .{ .start = start, .end = self.index } },
            '*' => return .{ .tag = .star, .loc = .{ .start = start, .end = self.index } },
            '/' => return .{ .tag = .slash, .loc = .{ .start = start, .end = self.index } },
            '%' => return .{ .tag = .percent, .loc = .{ .start = start, .end = self.index } },
            '^' => return .{ .tag = .caret, .loc = .{ .start = start, .end = self.index } },
            '~' => return .{ .tag = .tilde, .loc = .{ .start = start, .end = self.index } },
            '-' => {
                if (!self.isAtEnd() and self.peek() == '>') {
                    self.advance();
                    return .{ .tag = .arrow, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .minus, .loc = .{ .start = start, .end = self.index } };
            },
            '=' => {
                if (!self.isAtEnd() and self.peek() == '=') {
                    self.advance();
                    return .{ .tag = .double_equal, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .equal, .loc = .{ .start = start, .end = self.index } };
            },
            '!' => {
                if (!self.isAtEnd() and self.peek() == '=') {
                    self.advance();
                    return .{ .tag = .bang_equal, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .bang, .loc = .{ .start = start, .end = self.index } };
            },
            '<' => {
                if (!self.isAtEnd() and self.peek() == '=') {
                    self.advance();
                    return .{ .tag = .less_equal, .loc = .{ .start = start, .end = self.index } };
                }
                if (!self.isAtEnd() and self.peek() == '<') {
                    self.advance();
                    return .{ .tag = .shift_left, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .less, .loc = .{ .start = start, .end = self.index } };
            },
            '>' => {
                if (!self.isAtEnd() and self.peek() == '=') {
                    self.advance();
                    return .{ .tag = .greater_equal, .loc = .{ .start = start, .end = self.index } };
                }
                if (!self.isAtEnd() and self.peek() == '>') {
                    self.advance();
                    return .{ .tag = .shift_right, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .greater, .loc = .{ .start = start, .end = self.index } };
            },
            '&' => {
                if (!self.isAtEnd() and self.peek() == '&') {
                    self.advance();
                    return .{ .tag = .double_ampersand, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .ampersand, .loc = .{ .start = start, .end = self.index } };
            },
            '|' => {
                if (!self.isAtEnd() and self.peek() == '|') {
                    self.advance();
                    return .{ .tag = .double_pipe, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .pipe, .loc = .{ .start = start, .end = self.index } };
            },
            '#' => {
                if (!self.isAtEnd() and self.peek() == '[') {
                    self.advance();
                    return .{ .tag = .hash_lbracket, .loc = .{ .start = start, .end = self.index } };
                }
                return .{ .tag = .invalid, .loc = .{ .start = start, .end = self.index } };
            },
            else => return .{ .tag = .invalid, .loc = .{ .start = start, .end = self.index } },
        }
    }

    fn skipWhitespaceAndComments(self: *Lexer) void {
        while (!self.isAtEnd()) {
            const c = self.peek();
            if (c == ' ' or c == '\t' or c == '\n' or c == '\r') {
                self.advance();
                continue;
            }

            // Single-line comment
            if (c == '/' and !self.isAtEndOffset(1) and self.peekOffset(1) == '/') {
                while (!self.isAtEnd() and self.peek() != '\n') {
                    self.advance();
                }
                continue;
            }

            // Multi-line comment
            if (c == '/' and !self.isAtEndOffset(1) and self.peekOffset(1) == '*') {
                self.advance(); // /
                self.advance(); // *
                var depth: u32 = 1;
                while (!self.isAtEnd() and depth > 0) {
                    if (self.peek() == '/' and !self.isAtEndOffset(1) and self.peekOffset(1) == '*') {
                        self.advance();
                        self.advance();
                        depth += 1;
                    } else if (self.peek() == '*' and !self.isAtEndOffset(1) and self.peekOffset(1) == '/') {
                        self.advance();
                        self.advance();
                        depth -= 1;
                    } else {
                        self.advance();
                    }
                }
                continue;
            }

            break;
        }
    }

    fn peek(self: *const Lexer) u8 {
        return self.source[self.index];
    }

    fn peekOffset(self: *const Lexer, offset: u32) u8 {
        return self.source[self.index + offset];
    }

    fn advance(self: *Lexer) void {
        self.index += 1;
    }

    fn isAtEnd(self: *const Lexer) bool {
        return self.index >= self.source.len;
    }

    fn isAtEndOffset(self: *const Lexer, offset: u32) bool {
        return self.index + offset >= self.source.len;
    }
};

fn isAlpha(c: u8) bool {
    return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_';
}

fn isDigit(c: u8) bool {
    return c >= '0' and c <= '9';
}

fn isHexDigit(c: u8) bool {
    return isDigit(c) or (c >= 'a' and c <= 'f') or (c >= 'A' and c <= 'F');
}

// --- Tests ---

const testing = std.testing;

fn collectTags(source: []const u8) ![]Tag {
    var lexer = Lexer.init(source);
    var tags = std.ArrayList(Tag).init(testing.allocator);
    while (true) {
        const tok = lexer.next();
        try tags.append(testing.allocator, tok.tag);
        if (tok.tag == .eof) break;
    }
    return tags.items;
}

test "lex empty input" {
    var lexer = Lexer.init("");
    const tok = lexer.next();
    try testing.expectEqual(Tag.eof, tok.tag);
}

test "lex keywords" {
    var lexer = Lexer.init("let mut fn return");
    try testing.expectEqual(Tag.keyword_let, lexer.next().tag);
    try testing.expectEqual(Tag.keyword_mut, lexer.next().tag);
    try testing.expectEqual(Tag.keyword_fn, lexer.next().tag);
    try testing.expectEqual(Tag.keyword_return, lexer.next().tag);
    try testing.expectEqual(Tag.eof, lexer.next().tag);
}

test "lex identifiers" {
    var lexer = Lexer.init("foo bar_baz _x x123");
    const source = "foo bar_baz _x x123";
    var tok = lexer.next();
    try testing.expectEqual(Tag.identifier, tok.tag);
    try testing.expectEqualStrings("foo", tok.slice(source));
    tok = lexer.next();
    try testing.expectEqualStrings("bar_baz", tok.slice(source));
    tok = lexer.next();
    try testing.expectEqualStrings("_x", tok.slice(source));
    tok = lexer.next();
    try testing.expectEqualStrings("x123", tok.slice(source));
}

test "lex integer literals" {
    const source = "42 0xFF 0b1010 0o77";
    var lexer = Lexer.init(source);
    var tok = lexer.next();
    try testing.expectEqual(Tag.int_literal, tok.tag);
    try testing.expectEqualStrings("42", tok.slice(source));
    tok = lexer.next();
    try testing.expectEqual(Tag.int_literal, tok.tag);
    try testing.expectEqualStrings("0xFF", tok.slice(source));
    tok = lexer.next();
    try testing.expectEqual(Tag.int_literal, tok.tag);
    try testing.expectEqualStrings("0b1010", tok.slice(source));
    tok = lexer.next();
    try testing.expectEqual(Tag.int_literal, tok.tag);
    try testing.expectEqualStrings("0o77", tok.slice(source));
}

test "lex float literals" {
    const source = "3.14 0.5";
    var lexer = Lexer.init(source);
    var tok = lexer.next();
    try testing.expectEqual(Tag.float_literal, tok.tag);
    try testing.expectEqualStrings("3.14", tok.slice(source));
    tok = lexer.next();
    try testing.expectEqual(Tag.float_literal, tok.tag);
    try testing.expectEqualStrings("0.5", tok.slice(source));
}

test "lex string literals" {
    const source =
        \\"hello" "world"
    ;
    var lexer = Lexer.init(source);
    try testing.expectEqual(Tag.string_literal, lexer.next().tag);
    try testing.expectEqual(Tag.string_literal, lexer.next().tag);
    try testing.expectEqual(Tag.eof, lexer.next().tag);
}

test "lex string with escape sequences" {
    const source =
        \\"hello\nworld"
    ;
    var lexer = Lexer.init(source);
    const tok = lexer.next();
    try testing.expectEqual(Tag.string_literal, tok.tag);
}

test "lex char literals" {
    const source = "'a' '\\n'";
    var lexer = Lexer.init(source);
    try testing.expectEqual(Tag.char_literal, lexer.next().tag);
    try testing.expectEqual(Tag.char_literal, lexer.next().tag);
}

test "lex operators" {
    var lexer = Lexer.init("+ - * / % -> == != <= >= << >> && || :: #[");
    try testing.expectEqual(Tag.plus, lexer.next().tag);
    try testing.expectEqual(Tag.minus, lexer.next().tag);
    try testing.expectEqual(Tag.star, lexer.next().tag);
    try testing.expectEqual(Tag.slash, lexer.next().tag);
    try testing.expectEqual(Tag.percent, lexer.next().tag);
    try testing.expectEqual(Tag.arrow, lexer.next().tag);
    try testing.expectEqual(Tag.double_equal, lexer.next().tag);
    try testing.expectEqual(Tag.bang_equal, lexer.next().tag);
    try testing.expectEqual(Tag.less_equal, lexer.next().tag);
    try testing.expectEqual(Tag.greater_equal, lexer.next().tag);
    try testing.expectEqual(Tag.shift_left, lexer.next().tag);
    try testing.expectEqual(Tag.shift_right, lexer.next().tag);
    try testing.expectEqual(Tag.double_ampersand, lexer.next().tag);
    try testing.expectEqual(Tag.double_pipe, lexer.next().tag);
    try testing.expectEqual(Tag.double_colon, lexer.next().tag);
    try testing.expectEqual(Tag.hash_lbracket, lexer.next().tag);
}

test "lex delimiters" {
    var lexer = Lexer.init("( ) { } [ ] ; : , .");
    try testing.expectEqual(Tag.lparen, lexer.next().tag);
    try testing.expectEqual(Tag.rparen, lexer.next().tag);
    try testing.expectEqual(Tag.lbrace, lexer.next().tag);
    try testing.expectEqual(Tag.rbrace, lexer.next().tag);
    try testing.expectEqual(Tag.lbracket, lexer.next().tag);
    try testing.expectEqual(Tag.rbracket, lexer.next().tag);
    try testing.expectEqual(Tag.semicolon, lexer.next().tag);
    try testing.expectEqual(Tag.colon, lexer.next().tag);
    try testing.expectEqual(Tag.comma, lexer.next().tag);
    try testing.expectEqual(Tag.dot, lexer.next().tag);
}

test "skip single-line comments" {
    var lexer = Lexer.init("foo // this is a comment\nbar");
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.eof, lexer.next().tag);
}

test "skip multi-line comments" {
    var lexer = Lexer.init("foo /* comment */ bar");
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.eof, lexer.next().tag);
}

test "skip nested multi-line comments" {
    var lexer = Lexer.init("foo /* outer /* inner */ outer */ bar");
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.eof, lexer.next().tag);
}

test "lex complete function declaration" {
    var lexer = Lexer.init("pub fn main() -> void { return; }");
    try testing.expectEqual(Tag.keyword_pub, lexer.next().tag);
    try testing.expectEqual(Tag.keyword_fn, lexer.next().tag);
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.lparen, lexer.next().tag);
    try testing.expectEqual(Tag.rparen, lexer.next().tag);
    try testing.expectEqual(Tag.arrow, lexer.next().tag);
    try testing.expectEqual(Tag.keyword_void, lexer.next().tag);
    try testing.expectEqual(Tag.lbrace, lexer.next().tag);
    try testing.expectEqual(Tag.keyword_return, lexer.next().tag);
    try testing.expectEqual(Tag.semicolon, lexer.next().tag);
    try testing.expectEqual(Tag.rbrace, lexer.next().tag);
    try testing.expectEqual(Tag.eof, lexer.next().tag);
}

test "lex variable declaration with arithmetic" {
    var lexer = Lexer.init("let x: i32 = 10 + 20 * 2;");
    try testing.expectEqual(Tag.keyword_let, lexer.next().tag);
    try testing.expectEqual(Tag.identifier, lexer.next().tag);
    try testing.expectEqual(Tag.colon, lexer.next().tag);
    try testing.expectEqual(Tag.keyword_i32, lexer.next().tag);
    try testing.expectEqual(Tag.equal, lexer.next().tag);
    try testing.expectEqual(Tag.int_literal, lexer.next().tag);
    try testing.expectEqual(Tag.plus, lexer.next().tag);
    try testing.expectEqual(Tag.int_literal, lexer.next().tag);
    try testing.expectEqual(Tag.star, lexer.next().tag);
    try testing.expectEqual(Tag.int_literal, lexer.next().tag);
    try testing.expectEqual(Tag.semicolon, lexer.next().tag);
}

test "lex invalid character produces invalid token" {
    var lexer = Lexer.init("@");
    try testing.expectEqual(Tag.invalid, lexer.next().tag);
}
