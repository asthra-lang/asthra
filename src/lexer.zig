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
