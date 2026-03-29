const std = @import("std");
const Token = @import("token.zig").Token;
const Tag = @import("token.zig").Tag;
const Lexer = @import("lexer.zig").Lexer;
const Ast = @import("ast.zig").Ast;
const Diagnostics = @import("diagnostics.zig").Diagnostics;

pub const Parser = struct {
    lexer: Lexer,
    current: Token,
    previous: Token,
    ast: *Ast,
    diagnostics: *Diagnostics,
    source: []const u8,
    had_error: bool,

    pub fn init(source: []const u8, ast: *Ast, diagnostics: *Diagnostics) Parser {
        var lexer = Lexer.init(source);
        const first = lexer.next();
        return .{
            .lexer = lexer,
            .current = first,
            .previous = first,
            .ast = ast,
            .diagnostics = diagnostics,
            .source = source,
            .had_error = false,
        };
    }

    pub fn parse(self: *Parser) ParseError!void {
        // Parse package declaration
        self.ast.program.package_name = try self.parsePackageDecl();

        // Parse top-level declarations
        while (self.current.tag != .eof) {
            const decl = self.parseTopLevelDecl() catch {
                self.synchronize();
                continue;
            };
            try self.ast.program.decls.append(self.ast.allocator, decl);
        }
    }

    fn parsePackageDecl(self: *Parser) ParseError![]const u8 {
        try self.expect(.keyword_package);
        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);
        try self.expect(.semicolon);
        return name;
    }

    fn parseTopLevelDecl(self: *Parser) ParseError!Ast.TopLevelDecl {
        const start = self.current.loc.start;
        const visibility = try self.parseVisibility();

        if (self.current.tag == .keyword_extern) {
            const extern_decl = try self.parseExternDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .extern_decl = extern_decl },
                .start = start,
            };
        }

        if (self.current.tag == .keyword_fn) {
            const fn_decl = try self.parseFnDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .function = fn_decl },
                .start = start,
            };
        }

        if (self.current.tag == .keyword_struct) {
            const struct_decl = try self.parseStructDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .struct_decl = struct_decl },
                .start = start,
            };
        }

        if (self.current.tag == .keyword_impl) {
            const impl_decl = try self.parseImplDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .impl_decl = impl_decl },
                .start = start,
            };
        }

        if (self.current.tag == .keyword_enum) {
            const enum_decl = try self.parseEnumDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .enum_decl = enum_decl },
                .start = start,
            };
        }

        self.reportError("expected declaration");
        return error.ParseError;
    }

    fn parseVisibility(self: *Parser) ParseError!Ast.Visibility {
        if (self.current.tag == .keyword_pub) {
            self.advance();
            return .public;
        }
        if (self.current.tag == .keyword_priv) {
            self.advance();
            return .private;
        }
        self.reportError("expected 'pub' or 'priv' visibility modifier");
        return error.ParseError;
    }

    fn parseFnDecl(self: *Parser) ParseError!Ast.FnDecl {
        try self.expect(.keyword_fn);

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        try self.expect(.lparen);
        var params = std.ArrayList(Ast.Param){};

        if (self.current.tag == .keyword_none) {
            self.advance(); // consume 'none'
        } else if (self.current.tag != .rparen) {
            while (true) {
                const param = try self.parseParam();
                try params.append(self.ast.allocator, param);
                if (self.current.tag != .comma) break;
                self.advance(); // consume comma
            }
        }
        try self.expect(.rparen);

        try self.expect(.arrow);
        const return_type = try self.parseType();

        const body = try self.parseBlock();

        return .{
            .name = name,
            .params = params,
            .return_type = return_type,
            .body = body,
        };
    }

    fn parseExternDecl(self: *Parser) ParseError!Ast.ExternDecl {
        self.advance(); // consume 'extern'

        // Optional link name string (e.g., extern "c" fn ...)
        // Grammar: ExternDecl <- 'extern' STRING? 'fn' ...
        if (self.current.tag == .string_literal) {
            self.advance(); // consume link name string (ignored for now)
        }

        try self.expect(.keyword_fn);

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

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

        try self.expect(.semicolon);

        return .{
            .name = name,
            .params = params,
            .return_type = return_type,
        };
    }

    fn parseStructDecl(self: *Parser) ParseError!Ast.StructDecl {
        self.advance(); // consume 'struct'

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        try self.expect(.lbrace);
        var fields = std.ArrayList(Ast.StructField){};

        if (self.current.tag == .keyword_none) {
            self.advance(); // consume 'none'
        } else {
            while (self.current.tag != .rbrace and self.current.tag != .eof) {
                var visibility: Ast.Visibility = .private;
                if (self.current.tag == .keyword_pub) {
                    visibility = .public;
                    self.advance();
                } else if (self.current.tag == .keyword_priv) {
                    visibility = .private;
                    self.advance();
                }

                const field_name_token = self.current;
                try self.expect(.identifier);
                const field_name = field_name_token.slice(self.source);

                try self.expect(.colon);
                const type_expr = try self.parseType();

                try fields.append(self.ast.allocator, .{
                    .name = field_name,
                    .type_expr = type_expr,
                    .visibility = visibility,
                });

                if (self.current.tag == .comma) {
                    self.advance();
                }
            }
        }

        try self.expect(.rbrace);
        return .{ .name = name, .fields = fields };
    }

    fn parseEnumDecl(self: *Parser) ParseError!Ast.EnumDecl {
        self.advance(); // consume 'enum'

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        try self.expect(.lbrace);
        var variants = std.ArrayList(Ast.EnumVariant){};

        if (self.current.tag == .keyword_none) {
            self.advance();
        } else {
            while (self.current.tag != .rbrace and self.current.tag != .eof) {
                var visibility: Ast.Visibility = .private;
                if (self.current.tag == .keyword_pub) {
                    visibility = .public;
                    self.advance();
                } else if (self.current.tag == .keyword_priv) {
                    visibility = .private;
                    self.advance();
                }

                const variant_name_token = self.current;
                try self.expect(.identifier);
                const variant_name = variant_name_token.slice(self.source);

                var data_types = std.ArrayList(Ast.TypeExpr){};
                if (self.current.tag == .lparen) {
                    self.advance(); // consume '('
                    while (self.current.tag != .rparen) {
                        const t = try self.parseType();
                        try data_types.append(self.ast.allocator, t);
                        if (self.current.tag == .comma) self.advance();
                    }
                    try self.expect(.rparen);
                }

                try variants.append(self.ast.allocator, .{
                    .name = variant_name,
                    .visibility = visibility,
                    .data_types = data_types,
                });

                if (self.current.tag == .comma) self.advance();
            }
        }

        try self.expect(.rbrace);
        return .{ .name = name, .variants = variants };
    }

    fn parseImplDecl(self: *Parser) ParseError!Ast.ImplDecl {
        self.advance(); // consume 'impl'

        const name_token = self.current;
        try self.expect(.identifier);
        const type_name = name_token.slice(self.source);

        try self.expect(.lbrace);
        var methods = std.ArrayList(Ast.MethodDecl){};

        while (self.current.tag != .rbrace and self.current.tag != .eof) {
            const visibility = try self.parseVisibility();
            try self.expect(.keyword_fn);

            const method_name_token = self.current;
            try self.expect(.identifier);
            const method_name = method_name_token.slice(self.source);

            try self.expect(.lparen);
            var has_self = false;
            var params = std.ArrayList(Ast.Param){};

            if (self.current.tag == .keyword_self) {
                has_self = true;
                self.advance();
                if (self.current.tag == .comma) {
                    self.advance();
                    // Parse remaining params
                    while (self.current.tag != .rparen) {
                        const param = try self.parseParam();
                        try params.append(self.ast.allocator, param);
                        if (self.current.tag != .comma) break;
                        self.advance();
                    }
                }
            } else if (self.current.tag == .keyword_none) {
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

            try methods.append(self.ast.allocator, .{
                .visibility = visibility,
                .name = method_name,
                .has_self = has_self,
                .params = params,
                .return_type = return_type,
                .body = body,
            });
        }

        try self.expect(.rbrace);
        return .{ .type_name = type_name, .methods = methods };
    }

    fn parseParam(self: *Parser) ParseError!Ast.Param {
        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);
        try self.expect(.colon);
        const type_expr = try self.parseType();
        return .{ .name = name, .type_expr = type_expr };
    }

    fn parseType(self: *Parser) ParseError!Ast.TypeExpr {
        const tag = self.current.tag;

        // Handle Option<T> type
        if (tag == .keyword_Option) {
            self.advance(); // consume 'Option'
            try self.expect(.less); // consume '<'
            const inner = try self.parseType();
            try self.expect(.greater); // consume '>'
            const inner_ptr = self.ast.allocator.create(Ast.TypeExpr) catch return error.ParseError;
            inner_ptr.* = inner;
            return .{ .option_type = .{ .inner_type = inner_ptr } };
        }

        // Handle Result<T, E> type
        if (tag == .keyword_Result) {
            self.advance(); // consume 'Result'
            try self.expect(.less); // consume '<'
            const ok_type = try self.parseType();
            try self.expect(.comma); // consume ','
            const err_type = try self.parseType();
            try self.expect(.greater); // consume '>'
            const ok_ptr = self.ast.allocator.create(Ast.TypeExpr) catch return error.ParseError;
            ok_ptr.* = ok_type;
            const err_ptr = self.ast.allocator.create(Ast.TypeExpr) catch return error.ParseError;
            err_ptr.* = err_type;
            return .{ .result_type = .{ .ok_type = ok_ptr, .err_type = err_ptr } };
        }

        if (tag.isTypeKeyword()) {
            const builtin = switch (tag) {
                .keyword_void => Ast.BuiltinType.void,
                .keyword_bool => .bool_type,
                .keyword_int => .int_type,
                .keyword_float => .float_type,
                .keyword_string => .string_type,
                .keyword_i8 => .i8_type,
                .keyword_i16 => .i16_type,
                .keyword_i32 => .i32_type,
                .keyword_i64 => .i64_type,
                .keyword_i128 => .i128_type,
                .keyword_u8 => .u8_type,
                .keyword_u16 => .u16_type,
                .keyword_u32 => .u32_type,
                .keyword_u64 => .u64_type,
                .keyword_u128 => .u128_type,
                .keyword_f32 => .f32_type,
                .keyword_f64 => .f64_type,
                .keyword_usize => .usize_type,
                .keyword_isize => .isize_type,
                .keyword_Never => .never_type,
                else => unreachable,
            };
            self.advance();
            return .{ .builtin = builtin };
        }

        if (tag == .lbracket) {
            // Array type: [N]T
            self.advance(); // consume '['
            if (self.current.tag != .int_literal) {
                self.reportError("expected array size");
                return error.ParseError;
            }
            const size_text = self.current.slice(self.source);
            const size = parseIntLiteral(size_text);
            self.advance(); // consume size
            try self.expect(.rbracket);
            const elem_type = try self.parseType();
            const elem_ptr = self.ast.allocator.create(Ast.TypeExpr) catch return error.ParseError;
            elem_ptr.* = elem_type;
            return .{ .array_type = .{
                .size = @intCast(size),
                .element_type = elem_ptr,
            } };
        }

        if (tag == .lparen) {
            // Tuple type: (T1, T2, ...)
            self.advance(); // consume '('
            var element_types = std.ArrayList(Ast.TypeExpr){};
            const first_type = try self.parseType();
            try element_types.append(self.ast.allocator, first_type);
            if (self.current.tag != .comma) {
                self.reportError("tuple type requires at least 2 elements");
                return error.ParseError;
            }
            while (self.current.tag == .comma) {
                self.advance(); // consume ','
                const elem_type = try self.parseType();
                try element_types.append(self.ast.allocator, elem_type);
            }
            try self.expect(.rparen);
            return .{ .tuple_type = .{ .element_types = element_types } };
        }

        if (tag == .identifier) {
            const name = self.current.slice(self.source);
            self.advance();
            return .{ .named = name };
        }

        self.reportError("expected type");
        return error.ParseError;
    }

    fn parseBlock(self: *Parser) ParseError!Ast.Block {
        try self.expect(.lbrace);
        var stmts = std.ArrayList(Ast.Stmt){};
        while (self.current.tag != .rbrace and self.current.tag != .eof) {
            const stmt = self.parseStatement() catch {
                self.synchronize();
                continue;
            };
            try stmts.append(self.ast.allocator, stmt);
        }
        try self.expect(.rbrace);
        return .{ .stmts = stmts };
    }

    fn parseStatement(self: *Parser) ParseError!Ast.Stmt {
        switch (self.current.tag) {
            .keyword_return => return self.parseReturnStmt(),
            .keyword_let => return self.parseVarDecl(),
            .keyword_if => return self.parseIfStmt(),
            .keyword_for => return self.parseForStmt(),
            .keyword_break => {
                self.advance();
                try self.expect(.semicolon);
                return .break_stmt;
            },
            .keyword_continue => {
                self.advance();
                try self.expect(.semicolon);
                return .continue_stmt;
            },
            .keyword_match => return self.parseMatchStmt(),
            .keyword_unsafe => return self.parseUnsafeBlock(),
            .identifier => {
                // Check for assignment: ident = expr;
                // We need to peek ahead
                if (self.peekIsAssignment()) {
                    return self.parseAssignStmt();
                }
                return self.parseExprStmt();
            },
            else => return self.parseExprStmt(),
        }
    }

    fn peekIsAssignment(self: *Parser) bool {
        // Save state
        const saved_lexer = self.lexer;
        const saved_current = self.current;
        // Advance past identifier
        self.advance();
        // Skip field access chain: ident.field.field = ...
        while (self.current.tag == .dot) {
            self.advance(); // consume '.'
            if (self.current.tag == .identifier) {
                self.advance(); // consume field name
            } else {
                break;
            }
        }
        // Skip index access: ident[expr] = ...
        if (self.current.tag == .lbracket) {
            // Skip bracket contents by counting nesting
            self.advance(); // consume '['
            var depth: u32 = 1;
            while (depth > 0 and self.current.tag != .eof) {
                if (self.current.tag == .lbracket) depth += 1;
                if (self.current.tag == .rbracket) depth -= 1;
                self.advance();
            }
        }
        const is_assign = self.current.tag == .equal;
        // Restore state
        self.lexer = saved_lexer;
        self.current = saved_current;
        return is_assign;
    }

    fn parseReturnStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'return'

        // For void return, check if next is ';'
        if (self.current.tag == .semicolon) {
            self.advance();
            // Create a void expression (we'll use a special value)
            return .{ .return_stmt = .{ .expr = Ast.null_expr } };
        }

        const expr = try self.parseExpr();
        try self.expect(.semicolon);
        return .{ .return_stmt = .{ .expr = expr } };
    }

    fn parseVarDecl(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'let'

        var is_mutable = false;
        if (self.current.tag == .keyword_mut) {
            is_mutable = true;
            self.advance();
        }

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        try self.expect(.colon);
        const type_expr = try self.parseType();

        try self.expect(.equal);
        const init_expr = try self.parseExpr();

        try self.expect(.semicolon);
        return .{ .var_decl = .{
            .name = name,
            .is_mutable = is_mutable,
            .type_expr = type_expr,
            .init_expr = init_expr,
        } };
    }

    fn parseAssignStmt(self: *Parser) ParseError!Ast.Stmt {
        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        // Parse field access chain: ident.field.field = ...
        var target_fields = std.ArrayList([]const u8){};
        while (self.current.tag == .dot) {
            self.advance(); // consume '.'
            const field_token = self.current;
            try self.expect(.identifier);
            try target_fields.append(self.ast.allocator, field_token.slice(self.source));
        }

        // Parse index access: ident[expr] = ...
        var target_index: ?Ast.ExprIndex = null;
        if (self.current.tag == .lbracket) {
            self.advance(); // consume '['
            target_index = try self.parseExpr();
            try self.expect(.rbracket);
        }

        try self.expect(.equal);
        const value = try self.parseExpr();
        try self.expect(.semicolon);

        return .{ .assign = .{
            .target = name,
            .target_fields = target_fields,
            .target_index = target_index,
            .value = value,
        } };
    }

    fn parseIfStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'if'
        const condition = try self.parseExpr();
        const then_block = try self.ast.allocator.create(Ast.Block);
        then_block.* = try self.parseBlock();

        var else_block: ?*Ast.Block = null;
        if (self.current.tag == .keyword_else) {
            self.advance();
            if (self.current.tag == .keyword_if) {
                // else if: wrap in a block with a single if stmt
                const inner_if = try self.parseIfStmt();
                const eb = try self.ast.allocator.create(Ast.Block);
                var stmts = std.ArrayList(Ast.Stmt){};
                try stmts.append(self.ast.allocator, inner_if);
                eb.* = .{ .stmts = stmts };
                else_block = eb;
            } else {
                const eb = try self.ast.allocator.create(Ast.Block);
                eb.* = try self.parseBlock();
                else_block = eb;
            }
        }

        return .{ .if_stmt = .{
            .condition = condition,
            .then_block = then_block,
            .else_block = else_block,
        } };
    }

    fn parseForStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'for'

        const iter_var_token = self.current;
        try self.expect(.identifier);
        const iter_var = iter_var_token.slice(self.source);

        try self.expect(.keyword_in);
        const iterable = try self.parseExpr();

        const body = try self.ast.allocator.create(Ast.Block);
        body.* = try self.parseBlock();

        return .{ .for_stmt = .{
            .iter_var = iter_var,
            .iterable = iterable,
            .body = body,
        } };
    }

    fn parseMatchStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'match'
        const expr = try self.parseExpr();

        try self.expect(.lbrace);
        var arms = std.ArrayList(Ast.MatchArm){};

        while (self.current.tag != .rbrace and self.current.tag != .eof) {
            const pattern = try self.parsePattern();

            // Expect '=>'
            try self.expect(.fat_arrow);

            const body = try self.ast.allocator.create(Ast.Block);
            body.* = try self.parseBlock();

            try arms.append(self.ast.allocator, .{ .pattern = pattern, .body = body });
        }

        try self.expect(.rbrace);
        return .{ .match_stmt = .{ .expr = expr, .arms = arms } };
    }

    fn parsePattern(self: *Parser) ParseError!Ast.Pattern {
        // Handle Option/Result keyword as a pattern name
        const first_name = if (self.current.tag == .keyword_Option) blk: {
            self.advance();
            break :blk "Option";
        } else if (self.current.tag == .keyword_Result) blk: {
            self.advance();
            break :blk "Result";
        } else blk: {
            const first_token = self.current;
            try self.expect(.identifier);
            break :blk first_token.slice(self.source);
        };

        if (self.current.tag == .dot) {
            // Enum pattern: EnumName.VariantName or EnumName.VariantName(bindings)
            self.advance(); // consume '.'
            const variant_token = self.current;
            try self.expect(.identifier);
            const variant_name = variant_token.slice(self.source);

            var bindings = std.ArrayList([]const u8){};
            if (self.current.tag == .lparen) {
                self.advance(); // consume '('
                while (self.current.tag != .rparen and self.current.tag != .eof) {
                    const binding_token = self.current;
                    try self.expect(.identifier);
                    try bindings.append(self.ast.allocator, binding_token.slice(self.source));
                    if (self.current.tag == .comma) self.advance();
                }
                try self.expect(.rparen);
            }

            return .{ .enum_pattern = .{
                .enum_name = first_name,
                .variant_name = variant_name,
                .bindings = bindings,
            } };
        }

        return .{ .identifier = first_name };
    }

    fn parseUnsafeBlock(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'unsafe'
        const block = try self.ast.allocator.create(Ast.Block);
        block.* = try self.parseBlock();
        return .{ .unsafe_block = block };
    }

    fn parseExprStmt(self: *Parser) ParseError!Ast.Stmt {
        const expr = try self.parseExpr();
        try self.expect(.semicolon);
        return .{ .expr_stmt = expr };
    }

    pub const ParseError = error{ ParseError, OutOfMemory };

    // Expression parsing with precedence climbing
    fn parseExpr(self: *Parser) ParseError!Ast.ExprIndex {
        return self.parseLogicOr();
    }

    fn parseLogicOr(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseLogicAnd();
        while (self.current.tag == .double_pipe) {
            self.advance();
            const right = try self.parseLogicAnd();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .logic_or, .rhs = right } });
        }
        return left;
    }

    fn parseLogicAnd(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseBitwiseOr();
        while (self.current.tag == .double_ampersand) {
            self.advance();
            const right = try self.parseBitwiseOr();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .logic_and, .rhs = right } });
        }
        return left;
    }

    fn parseBitwiseOr(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseBitwiseXor();
        while (self.current.tag == .pipe) {
            self.advance();
            const right = try self.parseBitwiseXor();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .bit_or, .rhs = right } });
        }
        return left;
    }

    fn parseBitwiseXor(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseBitwiseAnd();
        while (self.current.tag == .caret) {
            self.advance();
            const right = try self.parseBitwiseAnd();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .bit_xor, .rhs = right } });
        }
        return left;
    }

    fn parseBitwiseAnd(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseEquality();
        while (self.current.tag == .ampersand) {
            self.advance();
            const right = try self.parseEquality();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = .bit_and, .rhs = right } });
        }
        return left;
    }

    fn parseEquality(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseRelational();
        while (self.current.tag == .double_equal or self.current.tag == .bang_equal) {
            const op: Ast.BinaryOp = if (self.current.tag == .double_equal) .eq else .neq;
            self.advance();
            const right = try self.parseRelational();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
        }
        return left;
    }

    fn parseRelational(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseShift();
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
            const right = try self.parseShift();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
        }
        return left;
    }

    fn parseShift(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseAdd();
        while (self.current.tag == .shift_left or self.current.tag == .shift_right) {
            const op: Ast.BinaryOp = if (self.current.tag == .shift_left) .shift_left else .shift_right;
            self.advance();
            const right = try self.parseAdd();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
        }
        return left;
    }

    fn parseAdd(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseMult();
        while (self.current.tag == .plus or self.current.tag == .minus) {
            const op: Ast.BinaryOp = if (self.current.tag == .plus) .add else .sub;
            self.advance();
            const right = try self.parseMult();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
        }
        return left;
    }

    fn parseMult(self: *Parser) ParseError!Ast.ExprIndex {
        var left = try self.parseUnary();
        while (self.current.tag == .star or self.current.tag == .slash or self.current.tag == .percent) {
            const op: Ast.BinaryOp = switch (self.current.tag) {
                .star => .mul,
                .slash => .div,
                .percent => .mod,
                else => unreachable,
            };
            self.advance();
            const right = try self.parseUnary();
            left = try self.ast.addExpr(.{ .binary = .{ .lhs = left, .op = op, .rhs = right } });
        }
        return left;
    }

    fn parseUnary(self: *Parser) ParseError!Ast.ExprIndex {
        if (self.current.tag == .minus) {
            self.advance();
            const operand = try self.parsePostfix();
            return self.ast.addExpr(.{ .unary = .{ .op = .negate, .operand = operand } });
        }
        if (self.current.tag == .bang) {
            self.advance();
            const operand = try self.parsePostfix();
            return self.ast.addExpr(.{ .unary = .{ .op = .logic_not, .operand = operand } });
        }
        if (self.current.tag == .tilde) {
            self.advance();
            const operand = try self.parsePostfix();
            return self.ast.addExpr(.{ .unary = .{ .op = .bit_not, .operand = operand } });
        }
        return self.parsePostfix();
    }

    fn parsePostfix(self: *Parser) ParseError!Ast.ExprIndex {
        var expr = try self.parsePrimary();

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
                // Index access: expr[index]
                self.advance(); // consume '['
                const index = try self.parseExpr();
                try self.expect(.rbracket);
                expr = try self.ast.addExpr(.{ .index_access = .{ .object = expr, .index = index } });
            } else {
                break;
            }
        }

        return expr;
    }

    fn parsePrimary(self: *Parser) ParseError!Ast.ExprIndex {
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
            .string_literal => {
                const raw = self.current.slice(self.source);
                // Strip quotes
                const value = if (raw.len >= 2) raw[1 .. raw.len - 1] else raw;
                self.advance();
                return self.ast.addExpr(.{ .string_literal = value });
            },
            .identifier => {
                const name = self.current.slice(self.source);
                self.advance();
                // Check for struct literal: Name { field: value, ... }
                if (self.current.tag == .lbrace) {
                    // Peek ahead to distinguish struct literal from block
                    // Struct literal: identifier '{' identifier ':' ...
                    if (self.peekIsStructLiteral()) {
                        return self.parseStructLiteral(name);
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
                // Array literal: [expr, expr, ...]
                self.advance(); // consume '['
                var elements = std.ArrayList(Ast.ExprIndex){};
                if (self.current.tag != .rbracket) {
                    while (true) {
                        const elem = try self.parseExpr();
                        try elements.append(self.ast.allocator, elem);
                        if (self.current.tag != .comma) break;
                        self.advance(); // consume comma
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
            // Type conversion calls: i32(expr), f64(expr), etc.
            .keyword_i8, .keyword_i16, .keyword_i32, .keyword_i64, .keyword_i128, .keyword_u8, .keyword_u16, .keyword_u32, .keyword_u64, .keyword_u128, .keyword_f32, .keyword_f64, .keyword_usize, .keyword_isize, .keyword_bool => {
                // Treat as an identifier for now (type conversion is a call)
                const name = self.current.slice(self.source);
                self.advance();
                return self.ast.addExpr(.{ .identifier = name });
            },
            else => {
                self.reportError("expected expression");
                return error.ParseError;
            },
        }
    }

    fn peekIsStructLiteral(self: *Parser) bool {
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

    fn parseStructLiteral(self: *Parser, name: []const u8) ParseError!Ast.ExprIndex {
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
        return self.ast.addExpr(.{ .struct_literal = .{ .name = name, .field_inits = field_inits } });
    }

    // Helpers
    fn advance(self: *Parser) void {
        self.previous = self.current;
        self.current = self.lexer.next();
    }

    fn expect(self: *Parser, tag: Tag) ParseError!void {
        if (self.current.tag == tag) {
            self.advance();
            return;
        }
        self.reportErrorExpected(tag);
        return error.ParseError;
    }

    fn reportError(self: *Parser, message: []const u8) void {
        self.had_error = true;
        self.diagnostics.report(.@"error", self.current.loc.start, "{s}", .{message});
    }

    fn reportErrorExpected(self: *Parser, expected: Tag) void {
        self.had_error = true;
        self.diagnostics.report(.@"error", self.current.loc.start, "expected {s}, found '{s}'", .{
            @tagName(expected),
            self.current.slice(self.source),
        });
    }

    fn synchronize(self: *Parser) void {
        while (self.current.tag != .eof) {
            if (self.previous.tag == .semicolon) return;
            switch (self.current.tag) {
                .keyword_fn, .keyword_let, .keyword_if, .keyword_for, .keyword_return, .keyword_pub, .keyword_priv, .rbrace => return,
                else => self.advance(),
            }
        }
    }
};

fn parseIntLiteral(text: []const u8) i64 {
    if (text.len > 2) {
        if (text[0] == '0' and (text[1] == 'x' or text[1] == 'X')) {
            return std.fmt.parseInt(i64, text[2..], 16) catch 0;
        }
        if (text[0] == '0' and (text[1] == 'b' or text[1] == 'B')) {
            return std.fmt.parseInt(i64, text[2..], 2) catch 0;
        }
        if (text[0] == '0' and text[1] == 'o') {
            return std.fmt.parseInt(i64, text[2..], 8) catch 0;
        }
    }
    return std.fmt.parseInt(i64, text, 10) catch 0;
}

// --- Tests ---

const testing = std.testing;

fn testParse(source: []const u8) !struct { ast: Ast, diag: Diagnostics } {
    var ast = Ast.init(testing.allocator);
    var diag = Diagnostics.init(testing.allocator, source, "test.ast");
    var parser = Parser.init(source, &ast, &diag);
    parser.parse() catch {};
    return .{ .ast = ast, .diag = diag };
}

test "parse empty main function" {
    var result = try testParse("package main;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqualStrings("main", result.ast.program.package_name);
    try testing.expectEqual(@as(usize, 1), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(Ast.Visibility.public, decl.visibility);
    switch (decl.decl) {
        .function => |f| try testing.expectEqualStrings("main", f.name),
    }
}

test "parse function with parameters" {
    var result = try testParse("package main;\npub fn add(a: i32, b: i32) -> i32 { return a; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("add", f.name);
            try testing.expectEqual(@as(usize, 2), f.params.items.len);
            try testing.expectEqualStrings("a", f.params.items[0].name);
            try testing.expectEqualStrings("b", f.params.items[1].name);
        },
    }
}

test "parse variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let x: i32 = 42; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 2), f.body.stmts.items.len);
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("x", vd.name);
                    try testing.expect(!vd.is_mutable);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse mutable variable declaration" {
    var result = try testParse("package main;\npub fn main() -> void { let mut y: i32 = 0; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("y", vd.name);
                    try testing.expect(vd.is_mutable);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse if/else statement" {
    var result = try testParse("package main;\npub fn main() -> void { if x > 0 { return; } else { return; } }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .if_stmt => |if_s| {
                    try testing.expect(if_s.else_block != null);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse for loop" {
    var result = try testParse("package main;\npub fn main() -> void { for i in range(10) { log(i); } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .for_stmt => |for_s| {
                    try testing.expectEqualStrings("i", for_s.iter_var);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse arithmetic expression precedence" {
    var result = try testParse("package main;\npub fn main() -> void { let z: i32 = 1 + 2 * 3; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    // The init expr should be a binary add at the top
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .binary => |bin| {
                            try testing.expectEqual(Ast.BinaryOp.add, bin.op);
                            // RHS should be mul
                            const rhs = result.ast.getExpr(bin.rhs);
                            switch (rhs) {
                                .binary => |rhs_bin| try testing.expectEqual(Ast.BinaryOp.mul, rhs_bin.op),
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse error on missing semicolon" {
    var result = try testParse("package main;\npub fn main() -> void { return }");
    defer result.diag.deinit();
    try testing.expect(result.diag.hasErrors());
}

test "parse error on missing visibility" {
    var result = try testParse("package main;\nfn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(result.diag.hasErrors());
}

test "parse multiple functions" {
    var result = try testParse("package main;\npub fn foo() -> void { return; }\npriv fn bar() -> i32 { return 0; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    try testing.expectEqual(Ast.Visibility.public, result.ast.program.decls.items[0].visibility);
    try testing.expectEqual(Ast.Visibility.private, result.ast.program.decls.items[1].visibility);
}

test "parseIntLiteral decimal" {
    try testing.expectEqual(@as(i64, 42), parseIntLiteral("42"));
    try testing.expectEqual(@as(i64, 0), parseIntLiteral("0"));
    try testing.expectEqual(@as(i64, 12345), parseIntLiteral("12345"));
}

test "parseIntLiteral hex" {
    try testing.expectEqual(@as(i64, 255), parseIntLiteral("0xFF"));
    try testing.expectEqual(@as(i64, 255), parseIntLiteral("0XFF"));
    try testing.expectEqual(@as(i64, 0x1A), parseIntLiteral("0x1A"));
}

test "parseIntLiteral binary" {
    try testing.expectEqual(@as(i64, 10), parseIntLiteral("0b1010"));
    try testing.expectEqual(@as(i64, 10), parseIntLiteral("0B1010"));
}

test "parseIntLiteral octal" {
    try testing.expectEqual(@as(i64, 63), parseIntLiteral("0o77"));
}

test "parse break statement" {
    var result = try testParse("package main;\npub fn main() -> void { for i in range(10) { break; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .for_stmt => |for_s| {
                    try testing.expectEqual(Ast.Stmt.break_stmt, for_s.body.stmts.items[0]);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse continue statement" {
    var result = try testParse("package main;\npub fn main() -> void { for i in range(10) { continue; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .for_stmt => |for_s| {
                    try testing.expectEqual(Ast.Stmt.continue_stmt, for_s.body.stmts.items[0]);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse float literal" {
    var result = try testParse("package main;\npub fn main() -> void { let x: f64 = 3.14; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("x", vd.name);
                    try testing.expectEqual(Ast.BuiltinType.f64_type, vd.type_expr.builtin);
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .float_literal => |val| try testing.expectApproxEqAbs(3.14, val, 0.001),
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse struct declaration" {
    var result = try testParse("package main;\npub struct Point { pub x: i32, pub y: i32 }\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .struct_decl => |sd| {
            try testing.expectEqualStrings("Point", sd.name);
            try testing.expectEqual(@as(usize, 2), sd.fields.items.len);
            try testing.expectEqualStrings("x", sd.fields.items[0].name);
            try testing.expectEqualStrings("y", sd.fields.items[1].name);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse struct literal" {
    var result = try testParse("package main;\npub fn main() -> void { let p: Point = Point { x: 1, y: 2 }; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .struct_literal => |sl| {
                            try testing.expectEqualStrings("Point", sl.name);
                            try testing.expectEqual(@as(usize, 2), sl.field_inits.items.len);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse type conversion call" {
    var result = try testParse("package main;\npub fn main() -> void { let x: f64 = f64(42); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .call => |call| {
                            const callee = result.ast.getExpr(call.callee);
                            switch (callee) {
                                .identifier => |name| try testing.expectEqualStrings("f64", name),
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse array type and literal" {
    var result = try testParse("package main;\npub fn main() -> void { let a: [3]i32 = [1, 2, 3]; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .array_type => |arr| {
                            try testing.expectEqual(@as(u32, 3), arr.size);
                            try testing.expectEqual(Ast.BuiltinType.i32_type, arr.element_type.builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .array_literal => |al| {
                            try testing.expectEqual(@as(usize, 3), al.elements.items.len);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse indexed assignment" {
    var result = try testParse("package main;\npub fn main() -> void { let mut a: [2]i32 = [1, 2]; a[0] = 99; return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[1]) {
                .assign => |assign| {
                    try testing.expectEqualStrings("a", assign.target);
                    try testing.expect(assign.target_index != null);
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse Option type and constructors" {
    var result = try testParse("package main;\npub fn main() -> void { let a: Option<i32> = Option.Some(42); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .option_type => |opt| {
                            try testing.expectEqual(Ast.BuiltinType.i32_type, opt.inner_type.builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse Result type and constructors" {
    var result = try testParse("package main;\npub fn main() -> void { let a: Result<i32, string> = Result.Ok(42); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("a", vd.name);
                    switch (vd.type_expr) {
                        .result_type => |res| {
                            try testing.expectEqual(Ast.BuiltinType.i32_type, res.ok_type.builtin);
                            try testing.expectEqual(Ast.BuiltinType.string_type, res.err_type.builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse Result return type" {
    var result = try testParse("package main;\npub fn divide(a: i32, b: i32) -> Result<i32, string> { return Result.Ok(a); }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("divide", f.name);
            switch (f.return_type) {
                .result_type => |res| {
                    try testing.expectEqual(Ast.BuiltinType.i32_type, res.ok_type.builtin);
                    try testing.expectEqual(Ast.BuiltinType.string_type, res.err_type.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse Option return type" {
    var result = try testParse("package main;\npub fn find(x: i32) -> Option<i32> { return Option.None(); }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("find", f.name);
            switch (f.return_type) {
                .option_type => |opt| {
                    try testing.expectEqual(Ast.BuiltinType.i32_type, opt.inner_type.builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse tuple type" {
    var result = try testParse("package main;\npub fn main() -> void { let p: (i32, f64) = (1, 2.0); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    try testing.expectEqualStrings("p", vd.name);
                    switch (vd.type_expr) {
                        .tuple_type => |tt| {
                            try testing.expectEqual(@as(usize, 2), tt.element_types.items.len);
                            try testing.expectEqual(Ast.BuiltinType.i32_type, tt.element_types.items[0].builtin);
                            try testing.expectEqual(Ast.BuiltinType.f64_type, tt.element_types.items[1].builtin);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                    // Check the init is a tuple literal
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .tuple_literal => |tl| {
                            try testing.expectEqual(@as(usize, 2), tl.elements.items.len);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse tuple return type" {
    var result = try testParse("package main;\npub fn swap(a: i32, b: i32) -> (i32, i32) { return (b, a); }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqualStrings("swap", f.name);
            switch (f.return_type) {
                .tuple_type => |tt| {
                    try testing.expectEqual(@as(usize, 2), tt.element_types.items.len);
                    try testing.expectEqual(Ast.BuiltinType.i32_type, tt.element_types.items[0].builtin);
                    try testing.expectEqual(Ast.BuiltinType.i32_type, tt.element_types.items[1].builtin);
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse tuple element access" {
    var result = try testParse("package main;\npub fn main() -> void { let p: (i32, i32) = (1, 2); log(p.0); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            // Second statement is log(p.0) which is an expr_stmt
            switch (f.body.stmts.items[1]) {
                .expr_stmt => |expr_idx| {
                    const expr = result.ast.getExpr(expr_idx);
                    switch (expr) {
                        .call => |call_e| {
                            // The argument is p.0 (field_access)
                            const arg_expr = result.ast.getExpr(call_e.args.items[0]);
                            switch (arg_expr) {
                                .field_access => |fa| {
                                    try testing.expectEqualStrings("0", fa.field);
                                },
                                else => return error.TestUnexpectedResult,
                            }
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse grouped expression still works" {
    var result = try testParse("package main;\npub fn main() -> void { let x: i32 = (1 + 2); return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            switch (f.body.stmts.items[0]) {
                .var_decl => |vd| {
                    const expr = result.ast.getExpr(vd.init_expr);
                    switch (expr) {
                        .grouped => {},
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
    }
}

test "parse extern function declaration" {
    var result = try testParse("package main;\npub extern fn abs(x: i32) -> i32;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    try testing.expectEqual(@as(usize, 2), result.ast.program.decls.items.len);
    const decl = result.ast.program.decls.items[0];
    try testing.expectEqual(Ast.Visibility.public, decl.visibility);
    switch (decl.decl) {
        .extern_decl => |ed| {
            try testing.expectEqualStrings("abs", ed.name);
            try testing.expectEqual(@as(usize, 1), ed.params.items.len);
            try testing.expectEqualStrings("x", ed.params.items[0].name);
            try testing.expectEqual(Ast.BuiltinType.i32_type, ed.return_type.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse extern function with string parameter" {
    var result = try testParse("package main;\npub extern fn atoi(s: string) -> i32;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .extern_decl => |ed| {
            try testing.expectEqualStrings("atoi", ed.name);
            try testing.expectEqual(@as(usize, 1), ed.params.items.len);
            try testing.expectEqual(Ast.BuiltinType.string_type, ed.params.items[0].type_expr.builtin);
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse unsafe block" {
    var result = try testParse("package main;\npub fn main() -> void { unsafe { let x: i32 = 42; } return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .function => |f| {
            try testing.expectEqual(@as(usize, 2), f.body.stmts.items.len);
            switch (f.body.stmts.items[0]) {
                .unsafe_block => |block| {
                    try testing.expectEqual(@as(usize, 1), block.stmts.items.len);
                    switch (block.stmts.items[0]) {
                        .var_decl => |vd| {
                            try testing.expectEqualStrings("x", vd.name);
                        },
                        else => return error.TestUnexpectedResult,
                    }
                },
                else => return error.TestUnexpectedResult,
            }
        },
        else => return error.TestUnexpectedResult,
    }
}

test "parse extern with link name" {
    var result = try testParse("package main;\npub extern \"c\" fn puts(s: string) -> i32;\npub fn main() -> void { return; }");
    defer result.diag.deinit();
    try testing.expect(!result.diag.hasErrors());
    const decl = result.ast.program.decls.items[0];
    switch (decl.decl) {
        .extern_decl => |ed| {
            try testing.expectEqualStrings("puts", ed.name);
        },
        else => return error.TestUnexpectedResult,
    }
}
