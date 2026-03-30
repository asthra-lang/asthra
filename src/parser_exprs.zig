const std = @import("std");
const Ast = @import("ast.zig").Ast;
const Tag = @import("token.zig").Tag;
const parser_mod = @import("parser.zig");
const Parser = parser_mod.Parser;
const processEscapes = parser_mod.processEscapes;
const parseIntLiteral = parser_mod.parseIntLiteral;

// Expression parsing with precedence climbing
pub fn parseExpr(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    return parseLogicOr(self);
}

fn parseLogicOr(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseLogicAnd(self);
    while (self.current.tag == .double_pipe) {
        self.advance();
        const right = try parseLogicAnd(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .logic_or, .rhs = right } });
    }
    return left;
}

fn parseLogicAnd(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseBitwiseOr(self);
    while (self.current.tag == .double_ampersand) {
        self.advance();
        const right = try parseBitwiseOr(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .logic_and, .rhs = right } });
    }
    return left;
}

fn parseBitwiseOr(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseBitwiseXor(self);
    while (self.current.tag == .pipe) {
        self.advance();
        const right = try parseBitwiseXor(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .bit_or, .rhs = right } });
    }
    return left;
}

fn parseBitwiseXor(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseBitwiseAnd(self);
    while (self.current.tag == .caret) {
        self.advance();
        const right = try parseBitwiseAnd(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .bit_xor, .rhs = right } });
    }
    return left;
}

fn parseBitwiseAnd(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseEquality(self);
    while (self.current.tag == .ampersand) {
        self.advance();
        const right = try parseEquality(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .bit_and, .rhs = right } });
    }
    return left;
}

fn parseEquality(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseRelational(self);
    while (self.current.tag == .double_equal or self.current.tag == .bang_equal) {
        const op: Ast.BinaryOp = if (self.current.tag == .double_equal) .eq else .neq;
        self.advance();
        const right = try parseRelational(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
    }
    return left;
}

fn parseRelational(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseShift(self);
    while (self.current.tag == .less or self.current.tag == .less_equal or
        self.current.tag == .greater or self.current.tag == .greater_equal)
    {
        const op: Ast.BinaryOp = switch (self.current.tag) {
            .less => .lt,
            .less_equal => .le,
            .greater => .gt,
            .greater_equal => .ge,
            else => unreachable,
        };
        self.advance();
        const right = try parseShift(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
    }
    return left;
}

fn parseShift(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseAdd(self);
    while (self.current.tag == .shift_left or self.current.tag == .shift_right) {
        const op: Ast.BinaryOp = if (self.current.tag == .shift_left) .shift_left else .shift_right;
        self.advance();
        const right = try parseAdd(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
    }
    return left;
}

fn parseAdd(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseMult(self);
    while (self.current.tag == .plus or self.current.tag == .minus) {
        const op: Ast.BinaryOp = if (self.current.tag == .plus) .add else .sub;
        self.advance();
        const right = try parseMult(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
    }
    return left;
}

fn parseMult(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var left = try parseUnary(self);
    while (self.current.tag == .star or self.current.tag == .slash or self.current.tag == .percent) {
        const op: Ast.BinaryOp = switch (self.current.tag) {
            .star => .mul,
            .slash => .div,
            .percent => .mod,
            else => unreachable,
        };
        self.advance();
        const right = try parseUnary(self);
        left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
    }
    return left;
}

pub fn parseUnary(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    if (self.current.tag == .minus) {
        self.advance();
        const operand = try parsePostfix(self);
        return self.ast.addExpr(.{ .unary = .{ .op = .negate, .operand = operand } });
    }
    if (self.current.tag == .bang) {
        self.advance();
        const operand = try parsePostfix(self);
        return self.ast.addExpr(.{ .unary = .{ .op = .logic_not, .operand = operand } });
    }
    if (self.current.tag == .tilde) {
        self.advance();
        const operand = try parsePostfix(self);
        return self.ast.addExpr(.{ .unary = .{ .op = .bit_not, .operand = operand } });
    }
    if (self.current.tag == .keyword_await) {
        self.advance();
        const operand = try parsePostfix(self);
        return self.ast.addExpr(.{ .await_expr = operand });
    }
    if (self.current.tag == .ampersand) {
        self.advance(); // consume '&'
        const operand = try parsePostfix(self);
        return self.ast.addExpr(.{ .address_of = operand });
    }
    if (self.current.tag == .star) {
        self.advance(); // consume '*'
        const operand = try parseUnary(self);
        return self.ast.addExpr(.{ .deref = operand });
    }
    return parsePostfix(self);
}

fn parsePostfix(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    var expr = try parsePrimary(self);

    while (true) {
        if (self.current.tag == .lparen) {
            // Function call
            self.advance();
            var args = std.ArrayList(Ast.ExprIndex){};

            if (self.current.tag == .keyword_none) {
                self.advance(); // consume 'none'
            } else if (self.current.tag != .rparen) {
                while (true) {
                    const arg = try self.parseExpr();
                    try args.append(self.ast.allocator, arg);
                    if (self.current.tag != .comma) break;
                    self.advance();
                }
            }
            try self.expect(.rparen);
            expr = try self.ast.addExpr(.{ .call = .{ .callee = expr, .args = args } });
        } else if (self.current.tag == .dot) {
            // Field access or method call: expr.field or expr.method(...)
            // Also handles tuple index: expr.0, expr.1, etc.
            self.advance(); // consume '.'

            // Handle tuple index access: .0, .1, .2, etc.
            if (self.current.tag == .int_literal) {
                const idx_token = self.current;
                const field_name = idx_token.slice(self.source);
                self.advance();
                expr = try self.ast.addExpr(.{ .field_access = .{ .object = expr, .field = field_name } });
                continue;
            }

            const field_token = self.current;
            try self.expect(.identifier);
            const field_name = field_token.slice(self.source);

            // Check if this is a method call: expr.method(args)
            if (self.current.tag == .lparen) {
                self.advance(); // consume '('
                var args = std.ArrayList(Ast.ExprIndex){};
                if (self.current.tag == .keyword_none) {
                    self.advance();
                } else if (self.current.tag != .rparen) {
                    while (true) {
                        const arg = try self.parseExpr();
                        try args.append(self.ast.allocator, arg);
                        if (self.current.tag != .comma) break;
                        self.advance();
                    }
                }
                try self.expect(.rparen);
                expr = try self.ast.addExpr(.{ .method_call = .{ .object = expr, .method = field_name, .args = args } });
            } else {
                expr = try self.ast.addExpr(.{ .field_access = .{ .object = expr, .field = field_name } });
            }
        } else if (self.current.tag == .lbracket) {
            // Index access or slice: expr[index] or expr[start:end]
            self.advance(); // consume '['

            // Check for arr[:] or arr[:end]
            if (self.current.tag == .colon) {
                self.advance(); // consume ':'
                var end_expr: ?Ast.ExprIndex = null;
                if (self.current.tag != .rbracket) {
                    end_expr = try self.parseExpr();
                }
                try self.expect(.rbracket);
                expr = try self.ast.addExpr(.{ .slice_expr = .{ .object = expr, .start = null, .end = end_expr } });
            } else {
                const first = try self.parseExpr();
                if (self.current.tag == .colon) {
                    // Slice: arr[start:end] or arr[start:]
                    self.advance(); // consume ':'
                    var end_expr: ?Ast.ExprIndex = null;
                    if (self.current.tag != .rbracket) {
                        end_expr = try self.parseExpr();
                    }
                    try self.expect(.rbracket);
                    expr = try self.ast.addExpr(.{ .slice_expr = .{ .object = expr, .start = first, .end = end_expr } });
                } else {
                    // Index access: arr[index]
                    try self.expect(.rbracket);
                    expr = try self.ast.addExpr(.{ .index_access = .{ .object = expr, .index = first } });
                }
            }
        } else {
            break;
        }
    }

    return expr;
}

fn parsePrimary(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    switch (self.current.tag) {
        .int_literal => {
            const text = self.current.slice(self.source);
            const value = parseIntLiteral(text);
            self.advance();
            return self.ast.addExpr(.{ .int_literal = value });
        },
        .float_literal => {
            const text = self.current.slice(self.source);
            const value = std.fmt.parseFloat(f64, text) catch 0.0;
            self.advance();
            return self.ast.addExpr(.{ .float_literal = value });
        },
        .keyword_true => {
            self.advance();
            return self.ast.addExpr(.{ .bool_literal = true });
        },
        .keyword_false => {
            self.advance();
            return self.ast.addExpr(.{ .bool_literal = false });
        },
        .char_literal => {
            const raw = self.current.slice(self.source);
            // Extract the character value from 'x' or '\n' style literals
            const char_val: u8 = if (raw.len >= 3 and raw[1] == '\\') blk: {
                // Escape sequence
                break :blk switch (raw[2]) {
                    'n' => '\n',
                    't' => '\t',
                    'r' => '\r',
                    '\\' => '\\',
                    '\'' => '\'',
                    '0' => 0,
                    else => raw[2],
                };
            } else if (raw.len >= 3) blk: {
                break :blk raw[1];
            } else blk: {
                break :blk 0;
            };
            self.advance();
            return self.ast.addExpr(.{ .char_literal = char_val });
        },
        .string_literal => {
            const raw = self.current.slice(self.source);
            // Strip quotes: triple-quoted """...""" or single-quoted "..."
            const value = if (raw.len >= 6 and std.mem.startsWith(u8, raw, "\"\"\""))
                raw[3 .. raw.len - 3]
            else if (raw.len >= 2)
                raw[1 .. raw.len - 1]
            else
                raw;
            // Process escape sequences (\n, \t, \r, \\, \", \', \0)
            const processed = processEscapes(self.ast.allocator, value) catch return error.ParseError;
            self.advance();
            return self.ast.addExpr(.{ .string_literal = processed });
        },
        .raw_string_literal => {
            const raw = self.current.slice(self.source);
            // Strip r""" prefix (4 chars) and """ suffix (3 chars)
            const value = if (raw.len >= 7) raw[4 .. raw.len - 3] else raw;
            self.advance();
            return self.ast.addExpr(.{ .string_literal = value });
        },
        .identifier => {
            const name = self.current.slice(self.source);
            self.advance();
            // Check for associated function call: Type::func(args)
            if (self.current.tag == .double_colon) {
                self.advance(); // consume '::'
                const func_token = self.current;
                try self.expect(.identifier);
                const func_name = func_token.slice(self.source);
                try self.expect(.lparen);
                var args = std.ArrayList(Ast.ExprIndex){};
                if (self.current.tag == .keyword_none) {
                    self.advance();
                } else if (self.current.tag != .rparen) {
                    while (true) {
                        const arg = try self.parseExpr();
                        try args.append(self.ast.allocator, arg);
                        if (self.current.tag != .comma) break;
                        self.advance();
                    }
                }
                try self.expect(.rparen);
                return self.ast.addExpr(.{ .associated_call = .{ .type_name = name, .func_name = func_name, .args = args } });
            }
            // Check for generic struct literal: Name<Type> { field: value, ... }
            if (self.current.tag == .less) {
                if (try self.tryParseGenericStructLiteral(name)) |expr_idx| {
                    return expr_idx;
                }
            }
            // Check for struct literal: Name { field: value, ... }
            if (self.current.tag == .lbrace) {
                // Peek ahead to distinguish struct literal from block
                // Struct literal: identifier '{' identifier ':' ...
                if (self.peekIsStructLiteral()) {
                    return parseStructLiteral(self, name, .{});
                }
            }
            return self.ast.addExpr(.{ .identifier = name });
        },
        .keyword_self => {
            self.advance();
            return self.ast.addExpr(.{ .identifier = "self" });
        },
        .lparen => {
            self.advance();
            const first = try self.parseExpr();
            if (self.current.tag == .comma) {
                // Tuple literal: (expr, expr, ...)
                var elements = std.ArrayList(Ast.ExprIndex){};
                try elements.append(self.ast.allocator, first);
                while (self.current.tag == .comma) {
                    self.advance(); // consume ','
                    const elem = try self.parseExpr();
                    try elements.append(self.ast.allocator, elem);
                }
                try self.expect(.rparen);
                return self.ast.addExpr(.{ .tuple_literal = .{ .elements = elements } });
            }
            try self.expect(.rparen);
            return self.ast.addExpr(.{ .grouped = first });
        },
        .lbracket => {
            // Array literal: [expr, expr, ...] or repeated array: [expr; count]
            self.advance(); // consume '['
            var elements = std.ArrayList(Ast.ExprIndex){};
            if (self.current.tag != .rbracket) {
                const first = try self.parseExpr();
                // Check for repeated array syntax: [value; count]
                if (self.current.tag == .semicolon) {
                    self.advance(); // consume ';'
                    const count_expr = try self.parseExpr();
                    try self.expect(.rbracket);
                    return self.ast.addExpr(.{ .repeated_array = .{ .value = first, .count = count_expr } });
                }
                try elements.append(self.ast.allocator, first);
                while (self.current.tag == .comma) {
                    self.advance(); // consume comma
                    const elem = try self.parseExpr();
                    try elements.append(self.ast.allocator, elem);
                }
            }
            try self.expect(.rbracket);
            return self.ast.addExpr(.{ .array_literal = .{ .elements = elements } });
        },
        // Option used as expression (e.g., Option.Some(42), Option.None())
        .keyword_Option => {
            self.advance();
            return self.ast.addExpr(.{ .identifier = "Option" });
        },
        // Result used as expression (e.g., Result.Ok(42), Result.Err("error"))
        .keyword_Result => {
            self.advance();
            return self.ast.addExpr(.{ .identifier = "Result" });
        },
        .keyword_sizeof => {
            self.advance(); // consume 'sizeof'
            try self.expect(.lparen);
            const type_expr = try self.parseType();
            try self.expect(.rparen);
            return self.ast.addExpr(.{ .sizeof_expr = type_expr });
        },
        // Type conversion calls: i32(expr), f64(expr), etc.
        .keyword_i8, .keyword_i16, .keyword_i32, .keyword_i64, .keyword_i128, .keyword_u8, .keyword_u16, .keyword_u32, .keyword_u64, .keyword_u128, .keyword_f32, .keyword_f64, .keyword_usize, .keyword_isize, .keyword_bool, .keyword_char => {
            // Treat as an identifier for now (type conversion is a call)
            const name = self.current.slice(self.source);
            self.advance();
            return self.ast.addExpr(.{ .identifier = name });
        },
        .keyword_fn => {
            return parseClosure(self);
        },
        .keyword_try => {
            self.advance(); // consume 'try'
            const inner = try self.parseExpr();
            return self.ast.addExpr(.{ .try_expr = inner });
        },
        else => {
            self.reportError("expected expression");
            return error.ParseError;
        },
    }
}

fn parseClosure(self: *Parser) Parser.ParseError!Ast.ExprIndex {
    self.advance(); // consume 'fn'
    try self.expect(.lparen);

    var params = std.ArrayList(Ast.Param){};
    if (self.current.tag == .keyword_none) {
        self.advance();
    } else if (self.current.tag != .rparen) {
        while (true) {
            const param = try self.parseParam();
            try params.append(self.ast.allocator, param);
            if (self.current.tag != .comma) break;
            self.advance();
        }
    }
    try self.expect(.rparen);

    try self.expect(.arrow);
    const return_type = try self.parseType();
    const body = try self.parseBlock();

    return self.ast.addExpr(.{ .closure = .{
        .params = params,
        .return_type = return_type,
        .body = body,
    } });
}

pub fn peekIsStructLiteral(self: *Parser) bool {
    // Save state and check if we see: { identifier :
    const saved_lexer = self.lexer;
    const saved_current = self.current;
    self.advance(); // consume '{'
    const is_struct = self.current.tag == .identifier and blk: {
        self.advance(); // consume identifier
        break :blk self.current.tag == .colon;
    };
    // Restore state
    self.lexer = saved_lexer;
    self.current = saved_current;
    return is_struct;
}

pub fn parseStructLiteral(self: *Parser, name: []const u8, type_args: std.ArrayList(Ast.TypeExpr)) Parser.ParseError!Ast.ExprIndex {
    try self.expect(.lbrace);
    var field_inits = std.ArrayList(Ast.FieldInitExpr){};

    while (self.current.tag != .rbrace and self.current.tag != .eof) {
        const field_name_token = self.current;
        try self.expect(.identifier);
        const field_name = field_name_token.slice(self.source);
        try self.expect(.colon);
        const value = try self.parseExpr();
        try field_inits.append(self.ast.allocator, .{ .name = field_name, .value = value });
        if (self.current.tag == .comma) {
            self.advance();
        }
    }

    try self.expect(.rbrace);
    return self.ast.addExpr(.{ .struct_literal = .{ .name = name, .type_args = type_args, .field_inits = field_inits } });
}

pub fn tryParseGenericStructLiteral(self: *Parser, name: []const u8) Parser.ParseError!?Ast.ExprIndex {
    // Save state: we're at '<' after identifier
    const saved_lexer = self.lexer;
    const saved_current = self.current;
    const saved_had_error = self.had_error;
    const saved_diag_count = self.diagnostics.errors.items.len;

    self.advance(); // consume '<'
    var type_args = std.ArrayList(Ast.TypeExpr){};
    const first_arg = self.parseType() catch {
        // Not a generic type, restore state
        self.lexer = saved_lexer;
        self.current = saved_current;
        self.had_error = saved_had_error;
        self.restoreDiagnostics(saved_diag_count);
        return null;
    };
    try type_args.append(self.ast.allocator, first_arg);
    while (self.current.tag == .comma) {
        self.advance(); // consume ','
        const arg = self.parseType() catch {
            self.lexer = saved_lexer;
            self.current = saved_current;
            self.had_error = saved_had_error;
            self.restoreDiagnostics(saved_diag_count);
            return null;
        };
        try type_args.append(self.ast.allocator, arg);
    }
    if (self.current.tag != .greater) {
        // Not a generic type, restore state
        self.lexer = saved_lexer;
        self.current = saved_current;
        self.had_error = saved_had_error;
        self.restoreDiagnostics(saved_diag_count);
        return null;
    }
    self.advance(); // consume '>'

    // Must be followed by '{' for a struct literal
    if (self.current.tag == .lbrace and self.peekIsStructLiteral()) {
        const result = try parseStructLiteral(self, name, type_args);
        return result;
    }

    // Not a struct literal, restore state
    self.lexer = saved_lexer;
    self.current = saved_current;
    self.had_error = saved_had_error;
    self.restoreDiagnostics(saved_diag_count);
    return null;
}
