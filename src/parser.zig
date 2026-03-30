const std = @import("std");
const Token = @import("token.zig").Token;
const Tag = @import("token.zig").Tag;
const Lexer = @import("lexer.zig").Lexer;
const Ast = @import("ast.zig").Ast;
const Diagnostics = @import("diagnostics.zig").Diagnostics;
const parser_stmts = @import("parser_stmts.zig");
const parser_exprs = @import("parser_exprs.zig");

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

        // Parse import declarations
        while (self.current.tag == .keyword_import) {
            const import_decl = try self.parseImportDecl();
            try self.ast.program.imports.append(self.ast.allocator, import_decl);
        }

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

    fn parseImportDecl(self: *Parser) ParseError!Ast.ImportDecl {
        try self.expect(.keyword_import);

        // Expect string literal for the import path
        if (self.current.tag != .string_literal) {
            self.reportError("expected string literal for import path");
            return error.ParseError;
        }
        const raw_path = self.current.slice(self.source);
        // Strip quotes
        const path = if (raw_path.len >= 2) raw_path[1 .. raw_path.len - 1] else raw_path;
        self.advance();

        // Optional alias: 'as' identifier
        var alias: ?[]const u8 = null;
        if (self.current.tag == .keyword_as) {
            self.advance(); // consume 'as'
            const alias_token = self.current;
            try self.expect(.identifier);
            alias = alias_token.slice(self.source);
        }

        try self.expect(.semicolon);
        return .{ .path = path, .alias = alias };
    }

    fn parseAnnotation(self: *Parser) ParseError!Ast.Annotation {
        try self.expect(.hash_lbracket); // consume '#['

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        var args = std.ArrayList([]const u8){};

        // Optional arguments: (arg1, arg2, ...)
        if (self.current.tag == .lparen) {
            self.advance(); // consume '('
            while (self.current.tag != .rparen and self.current.tag != .eof) {
                const arg_token = self.current;
                // Args can be identifiers, keywords, or string literals
                if (self.current.tag == .identifier) {
                    try args.append(self.ast.allocator, arg_token.slice(self.source));
                    self.advance();
                } else if (self.current.tag == .string_literal) {
                    const raw = arg_token.slice(self.source);
                    const value = if (raw.len >= 2) raw[1 .. raw.len - 1] else raw;
                    try args.append(self.ast.allocator, value);
                    self.advance();
                } else {
                    // Accept any keyword as an annotation arg (e.g., "high", "low")
                    try args.append(self.ast.allocator, arg_token.slice(self.source));
                    self.advance();
                }
                if (self.current.tag == .comma) {
                    self.advance(); // consume ','
                }
            }
            try self.expect(.rparen); // consume ')'
        }

        try self.expect(.rbracket); // consume ']'

        return .{ .name = name, .args = args };
    }

    fn parseTopLevelDecl(self: *Parser) ParseError!Ast.TopLevelDecl {
        const start = self.current.loc.start;

        // Parse optional annotations: #[...] #[...] ...
        var annotations = std.ArrayList(Ast.Annotation){};
        while (self.current.tag == .hash_lbracket) {
            const annotation = try self.parseAnnotation();
            try annotations.append(self.ast.allocator, annotation);
        }

        const visibility = try self.parseVisibility();

        if (self.current.tag == .keyword_extern) {
            const extern_decl = try self.parseExternDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .extern_decl = extern_decl },
                .start = start,
                .annotations = annotations,
            };
        }

        if (self.current.tag == .keyword_fn) {
            const fn_decl = try self.parseFnDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .function = fn_decl },
                .start = start,
                .annotations = annotations,
            };
        }

        if (self.current.tag == .keyword_struct) {
            const struct_decl = try self.parseStructDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .struct_decl = struct_decl },
                .start = start,
                .annotations = annotations,
            };
        }

        if (self.current.tag == .keyword_impl) {
            const impl_decl = try self.parseImplDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .impl_decl = impl_decl },
                .start = start,
                .annotations = annotations,
            };
        }

        if (self.current.tag == .keyword_trait) {
            const trait_decl = try self.parseTraitDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .trait_decl = trait_decl },
                .start = start,
                .annotations = annotations,
            };
        }

        if (self.current.tag == .keyword_enum) {
            const enum_decl = try self.parseEnumDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .enum_decl = enum_decl },
                .start = start,
                .annotations = annotations,
            };
        }

        if (self.current.tag == .keyword_const) {
            const const_decl = try self.parseConstDecl();
            return .{
                .visibility = visibility,
                .decl = .{ .const_decl = const_decl },
                .start = start,
                .annotations = annotations,
            };
        }

        // Type alias: type Name = ExistingType;
        if (self.current.tag == .identifier and std.mem.eql(u8, self.current.slice(self.source), "type")) {
            const type_alias = try self.parseTypeAlias();
            return .{
                .visibility = visibility,
                .decl = .{ .type_alias = type_alias },
                .start = start,
                .annotations = annotations,
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

        // Parse optional type parameters: <T, U, ...>
        var type_params = std.ArrayList([]const u8){};
        if (self.current.tag == .less) {
            self.advance(); // consume '<'
            while (true) {
                const tp_token = self.current;
                try self.expect(.identifier);
                try type_params.append(self.ast.allocator, tp_token.slice(self.source));
                if (self.current.tag != .comma) break;
                self.advance();
            }
            try self.expect(.greater);
        }

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
            .type_params = type_params,
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

        // Parse optional type parameters: <T, U, ...>
        var type_params = std.ArrayList([]const u8){};
        if (self.current.tag == .less) {
            self.advance(); // consume '<'
            while (true) {
                const tp_token = self.current;
                try self.expect(.identifier);
                try type_params.append(self.ast.allocator, tp_token.slice(self.source));
                if (self.current.tag != .comma) break;
                self.advance(); // consume ','
            }
            try self.expect(.greater);
        }

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
        return .{ .name = name, .type_params = type_params, .fields = fields };
    }

    fn parseEnumDecl(self: *Parser) ParseError!Ast.EnumDecl {
        self.advance(); // consume 'enum'

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        // Parse optional type parameters: <T, U, ...>
        var type_params = std.ArrayList([]const u8){};
        if (self.current.tag == .less) {
            self.advance(); // consume '<'
            while (true) {
                const tp_token = self.current;
                try self.expect(.identifier);
                try type_params.append(self.ast.allocator, tp_token.slice(self.source));
                if (self.current.tag != .comma) break;
                self.advance(); // consume ','
            }
            try self.expect(.greater);
        }

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
        return .{ .name = name, .type_params = type_params, .variants = variants };
    }

    fn parseConstDecl(self: *Parser) ParseError!Ast.ConstDecl {
        self.advance(); // consume 'const'

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        try self.expect(.colon);
        const type_expr = try self.parseType();

        try self.expect(.equal);
        const init_expr = try self.parseExpr();

        try self.expect(.semicolon);
        return .{
            .name = name,
            .type_expr = type_expr,
            .init_expr = init_expr,
        };
    }

    fn parseTypeAlias(self: *Parser) ParseError!Ast.TypeAliasDecl {
        self.advance(); // consume 'type' identifier

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        try self.expect(.equal);
        const target = try self.parseType();

        try self.expect(.semicolon);
        return .{
            .name = name,
            .target = target,
        };
    }

    fn parseImplDecl(self: *Parser) ParseError!Ast.ImplDecl {
        self.advance(); // consume 'impl'

        const first_name_token = self.current;
        try self.expect(.identifier);
        const first_name = first_name_token.slice(self.source);

        // Check for `impl Trait for Type` syntax
        var type_name: []const u8 = first_name;
        var trait_name: ?[]const u8 = null;
        if (self.current.tag == .keyword_for) {
            self.advance(); // consume 'for'
            trait_name = first_name;
            const type_token = self.current;
            try self.expect(.identifier);
            type_name = type_token.slice(self.source);
        }

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
        return .{ .type_name = type_name, .trait_name = trait_name, .methods = methods };
    }

    fn parseTraitDecl(self: *Parser) ParseError!Ast.TraitDecl {
        self.advance(); // consume 'trait'

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        try self.expect(.lbrace);
        var methods = std.ArrayList(Ast.TraitMethodSig){};

        while (self.current.tag != .rbrace and self.current.tag != .eof) {
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
                    while (self.current.tag != .rparen) {
                        const param = try self.parseParam();
                        try params.append(self.ast.allocator, param);
                        if (self.current.tag != .comma) break;
                        self.advance();
                    }
                }
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

            try methods.append(self.ast.allocator, .{
                .name = method_name,
                .has_self = has_self,
                .params = params,
                .return_type = return_type,
            });
        }

        try self.expect(.rbrace);
        return .{ .name = name, .methods = methods };
    }

    pub fn parseParam(self: *Parser) ParseError!Ast.Param {
        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);
        try self.expect(.colon);
        const type_expr = try self.parseType();
        return .{ .name = name, .type_expr = type_expr };
    }

    pub fn parseType(self: *Parser) ParseError!Ast.TypeExpr {
        const tag = self.current.tag;

        // Handle pointer types: *mut T, *const T
        if (tag == .star) {
            self.advance(); // consume '*'
            var is_mutable = false;
            if (self.current.tag == .keyword_mut) {
                is_mutable = true;
                self.advance();
            } else if (self.current.tag == .keyword_const) {
                is_mutable = false;
                self.advance();
            } else {
                self.reportError("expected 'mut' or 'const' after '*' in pointer type");
                return error.ParseError;
            }
            const pointee = try self.parseType();
            const pointee_ptr = self.ast.allocator.create(Ast.TypeExpr) catch return error.ParseError;
            pointee_ptr.* = pointee;
            return .{ .ptr_type = .{ .is_mutable = is_mutable, .pointee = pointee_ptr } };
        }

        // Handle dyn Trait type
        if (tag == .keyword_dyn) {
            self.advance(); // consume 'dyn'
            const trait_name_token = self.current;
            try self.expect(.identifier);
            return .{ .dyn_type = trait_name_token.slice(self.source) };
        }

        // Handle function type: fn(T1, T2) -> R
        if (tag == .keyword_fn) {
            self.advance(); // consume 'fn'
            try self.expect(.lparen);
            var param_types = std.ArrayList(Ast.TypeExpr){};
            if (self.current.tag != .rparen) {
                while (true) {
                    const pt = try self.parseType();
                    try param_types.append(self.ast.allocator, pt);
                    if (self.current.tag != .comma) break;
                    self.advance();
                }
            }
            try self.expect(.rparen);
            try self.expect(.arrow);
            const ret_type = try self.parseType();
            const ret_ptr = self.ast.allocator.create(Ast.TypeExpr) catch return error.ParseError;
            ret_ptr.* = ret_type;
            return .{ .fn_type = .{ .param_types = param_types, .return_type = ret_ptr } };
        }

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
                .keyword_char => .char_type,
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
            self.advance(); // consume '['
            if (self.current.tag == .rbracket) {
                // Slice type: []T
                self.advance(); // consume ']'
                const elem_type = try self.parseType();
                const elem_ptr = self.ast.allocator.create(Ast.TypeExpr) catch return error.ParseError;
                elem_ptr.* = elem_type;
                return .{ .slice_type = .{ .element_type = elem_ptr } };
            }
            // Array type: [N]T
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
            // Check for generic type: Name<Type, ...>
            if (self.current.tag == .less) {
                // Save state to backtrack if this isn't a generic type
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
                    return .{ .named = name };
                };
                try type_args.append(self.ast.allocator, first_arg);
                while (self.current.tag == .comma) {
                    self.advance(); // consume ','
                    const arg = self.parseType() catch {
                        self.lexer = saved_lexer;
                        self.current = saved_current;
                        self.had_error = saved_had_error;
                        self.restoreDiagnostics(saved_diag_count);
                        return .{ .named = name };
                    };
                    try type_args.append(self.ast.allocator, arg);
                }
                if (self.current.tag != .greater) {
                    // Not a generic type, restore state
                    self.lexer = saved_lexer;
                    self.current = saved_current;
                    self.had_error = saved_had_error;
                    self.restoreDiagnostics(saved_diag_count);
                    return .{ .named = name };
                }
                self.advance(); // consume '>'
                return .{ .generic_type = .{ .name = name, .type_args = type_args } };
            }
            return .{ .named = name };
        }

        self.reportError("expected type");
        return error.ParseError;
    }

    // Delegating methods for statement parsing (implemented in parser_stmts.zig)
    pub fn parseBlock(self: *Parser) ParseError!Ast.Block {
        return parser_stmts.parseBlock(self);
    }
    pub fn parseStatement(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseStatement(self);
    }
    pub fn parseReturnStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseReturnStmt(self);
    }
    pub fn parseVarDecl(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseVarDecl(self);
    }
    pub fn peekIsAssignment(self: *Parser) bool {
        return parser_stmts.peekIsAssignment(self);
    }
    pub fn parseAssignStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseAssignStmt(self);
    }
    pub fn peekIsDerefAssignment(self: *Parser) bool {
        return parser_stmts.peekIsDerefAssignment(self);
    }
    pub fn parseDerefAssignStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseDerefAssignStmt(self);
    }
    pub fn parseIfStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseIfStmt(self);
    }
    pub fn peekIsIfLet(self: *Parser) bool {
        return parser_stmts.peekIsIfLet(self);
    }
    pub fn parseIfLetStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseIfLetStmt(self);
    }
    pub fn parseForStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseForStmt(self);
    }
    pub fn parseWhileStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseWhileStmt(self);
    }
    pub fn parseMatchStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseMatchStmt(self);
    }
    pub fn parsePattern(self: *Parser) ParseError!Ast.Pattern {
        return parser_stmts.parsePattern(self);
    }
    pub fn parseUnsafeBlock(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseUnsafeBlock(self);
    }
    pub fn parseSpawnStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseSpawnStmt(self);
    }
    pub fn parseSpawnHandleStmt(self: *Parser) ParseError!Ast.Stmt {
        return parser_stmts.parseSpawnHandleStmt(self);
    }

    pub const ParseError = error{ ParseError, OutOfMemory };

    // Delegating methods for expression parsing (implemented in parser_exprs.zig)
    pub fn parseExpr(self: *Parser) ParseError!Ast.ExprIndex {
        return parser_exprs.parseExpr(self);
    }
    pub fn parseUnary(self: *Parser) ParseError!Ast.ExprIndex {
        return parser_exprs.parseUnary(self);
    }
    pub fn peekIsStructLiteral(self: *Parser) bool {
        return parser_exprs.peekIsStructLiteral(self);
    }
    pub fn tryParseGenericStructLiteral(self: *Parser, name: []const u8) ParseError!?Ast.ExprIndex {
        return parser_exprs.tryParseGenericStructLiteral(self, name);
    }

    pub fn restoreDiagnostics(self: *Parser, saved_count: usize) void {
        // Free and remove diagnostics added during speculative parsing
        for (self.diagnostics.errors.items[saved_count..]) |err| {
            self.diagnostics.allocator.free(err.message);
        }
        self.diagnostics.errors.items.len = saved_count;
    }

    // Helpers
    pub fn advance(self: *Parser) void {
        self.previous = self.current;
        self.current = self.lexer.next();
    }

    pub fn expect(self: *Parser, tag: Tag) ParseError!void {
        if (self.current.tag == tag) {
            self.advance();
            return;
        }
        self.reportErrorExpected(tag);
        return error.ParseError;
    }

    pub fn reportError(self: *Parser, message: []const u8) void {
        self.had_error = true;
        self.diagnostics.report(.@"error", self.current.loc.start, "{s}", .{message});
    }

    pub fn reportErrorExpected(self: *Parser, expected: Tag) void {
        self.had_error = true;
        self.diagnostics.report(.@"error", self.current.loc.start, "expected {s}, found '{s}'", .{
            @tagName(expected),
            self.current.slice(self.source),
        });
    }

    pub fn synchronize(self: *Parser) void {
        while (self.current.tag != .eof) {
            if (self.previous.tag == .semicolon) return;
            switch (self.current.tag) {
                .keyword_fn, .keyword_let, .keyword_if, .keyword_for, .keyword_while, .keyword_return, .keyword_pub, .keyword_priv, .hash_lbracket, .rbrace => return,
                else => self.advance(),
            }
        }
    }
};

/// Process escape sequences in a string literal.
/// Handles: \n, \t, \r, \\, \", \', \0
/// Returns the original slice unchanged if no escapes are present (no allocation).
pub fn processEscapes(allocator: std.mem.Allocator, input: []const u8) error{OutOfMemory}![]const u8 {
    // Quick check: if no backslashes, return as-is (no allocation needed)
    if (std.mem.indexOfScalar(u8, input, '\\') == null) {
        return input;
    }

    var buf = allocator.alloc(u8, input.len) catch return error.OutOfMemory;
    var out: usize = 0;
    var i: usize = 0;
    while (i < input.len) {
        if (input[i] == '\\' and i + 1 < input.len) {
            const next = input[i + 1];
            const replacement: u8 = switch (next) {
                'n' => '\n',
                't' => '\t',
                'r' => '\r',
                '\\' => '\\',
                '"' => '"',
                '\'' => '\'',
                '0' => 0,
                else => {
                    // Unknown escape: keep both characters
                    buf[out] = '\\';
                    out += 1;
                    buf[out] = next;
                    out += 1;
                    i += 2;
                    continue;
                },
            };
            buf[out] = replacement;
            out += 1;
            i += 2;
        } else {
            buf[out] = input[i];
            out += 1;
            i += 1;
        }
    }
    return buf[0..out];
}

pub fn parseIntLiteral(text: []const u8) i64 {
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

test {
    _ = @import("parser_tests.zig");
    _ = @import("parser_tests_exprs.zig");
    _ = @import("parser_tests_stmts.zig");
}
