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
            .keyword_if => {
                // Peek ahead to distinguish 'if let' from regular 'if'
                if (self.peekIsIfLet()) {
                    return self.parseIfLetStmt();
                }
                return self.parseIfStmt();
            },
            .keyword_for => return self.parseForStmt(),
            .keyword_while => return self.parseWhileStmt(),
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
            .keyword_spawn => return self.parseSpawnStmt(),
            .keyword_spawn_with_handle => return self.parseSpawnHandleStmt(),
            .star => {
                // Deref assignment: *expr = value;
                // Check if this is a deref assignment by peeking
                if (self.peekIsDerefAssignment()) {
                    return self.parseDerefAssignStmt();
                }
                return self.parseExprStmt();
            },
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
        const start = self.current.loc.start;
        self.advance(); // consume 'let'

        var is_mutable = false;
        if (self.current.tag == .keyword_mut) {
            is_mutable = true;
            self.advance();
        }

        const name_token = self.current;
        try self.expect(.identifier);
        const name = name_token.slice(self.source);

        var type_expr: Ast.TypeExpr = .inferred;
        if (self.current.tag == .colon) {
            self.advance(); // consume ':'
            type_expr = try self.parseType();
        }

        try self.expect(.equal);
        const init_expr = try self.parseExpr();

        try self.expect(.semicolon);
        return .{ .var_decl = .{
            .name = name,
            .is_mutable = is_mutable,
            .type_expr = type_expr,
            .init_expr = init_expr,
            .start = start,
        } };
    }

    fn parseAssignStmt(self: *Parser) ParseError!Ast.Stmt {
        const start = self.current.loc.start;
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
            .start = start,
        } };
    }

    fn peekIsDerefAssignment(self: *Parser) bool {
        // Save state
        const saved_lexer = self.lexer;
        const saved_current = self.current;
        // Advance past '*'
        self.advance();
        // Skip the target expression (identifier, possibly with more derefs)
        // Simple heuristic: skip tokens until we find '=' or ';'
        var depth: u32 = 0;
        while (self.current.tag != .eof and self.current.tag != .semicolon) {
            if (self.current.tag == .lparen) depth += 1;
            if (self.current.tag == .rparen) {
                if (depth == 0) break;
                depth -= 1;
            }
            if (depth == 0 and self.current.tag == .equal) {
                // Restore state
                self.lexer = saved_lexer;
                self.current = saved_current;
                return true;
            }
            self.advance();
        }
        // Restore state
        self.lexer = saved_lexer;
        self.current = saved_current;
        return false;
    }

    fn parseDerefAssignStmt(self: *Parser) ParseError!Ast.Stmt {
        // Parse the LHS as an expression (which will be a deref expression)
        // We consume '*' and parse the target
        self.advance(); // consume '*'
        const target = try self.parseUnary();
        try self.expect(.equal);
        const value = try self.parseExpr();
        try self.expect(.semicolon);
        return .{ .deref_assign = .{ .target = target, .value = value } };
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

    fn peekIsIfLet(self: *Parser) bool {
        // Save state
        const saved_lexer = self.lexer;
        const saved_current = self.current;
        // Advance past 'if'
        self.advance();
        const is_let = self.current.tag == .keyword_let;
        // Restore state
        self.lexer = saved_lexer;
        self.current = saved_current;
        return is_let;
    }

    fn parseIfLetStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'if'
        self.advance(); // consume 'let'

        const pattern = try self.parsePattern();

        try self.expect(.equal);
        const expr = try self.parseExpr();

        const then_block = try self.ast.allocator.create(Ast.Block);
        then_block.* = try self.parseBlock();

        var else_block: ?*Ast.Block = null;
        if (self.current.tag == .keyword_else) {
            self.advance();
            const eb = try self.ast.allocator.create(Ast.Block);
            eb.* = try self.parseBlock();
            else_block = eb;
        }

        return .{ .if_let_stmt = .{
            .pattern = pattern,
            .expr = expr,
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

    fn parseWhileStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'while'
        const condition = try self.parseExpr();

        const body = try self.ast.allocator.create(Ast.Block);
        body.* = try self.parseBlock();

        return .{ .while_stmt = .{
            .condition = condition,
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
        // Handle wildcard pattern: _
        if (self.current.tag == .identifier) {
            const text = self.current.slice(self.source);
            if (std.mem.eql(u8, text, "_")) {
                self.advance();
                return .wildcard;
            }
        }

        // Handle tuple pattern: (pattern, pattern, ...)
        if (self.current.tag == .lparen) {
            self.advance(); // consume '('
            var elements = std.ArrayList(Ast.Pattern){};
            const first = try self.parsePattern();
            try elements.append(self.ast.allocator, first);
            try self.expect(.comma);
            const second = try self.parsePattern();
            try elements.append(self.ast.allocator, second);
            while (self.current.tag == .comma) {
                self.advance(); // consume ','
                const next = try self.parsePattern();
                try elements.append(self.ast.allocator, next);
            }
            try self.expect(.rparen);
            return .{ .tuple_pattern = .{ .elements = elements } };
        }

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

    fn parseSpawnStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'spawn'
        const expr = try self.parseExpr();
        try self.expect(.semicolon);
        return .{ .spawn_stmt = .{ .expr = expr } };
    }

    fn parseSpawnHandleStmt(self: *Parser) ParseError!Ast.Stmt {
        self.advance(); // consume 'spawn_with_handle'
        const name_token = self.current;
        try self.expect(.identifier);
        const handle_name = name_token.slice(self.source);
        try self.expect(.equal);
        const expr = try self.parseExpr();
        try self.expect(.semicolon);
        return .{ .spawn_handle_stmt = .{ .handle_name = handle_name, .expr = expr } };
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
        if (self.current.tag == .keyword_await) {
            self.advance();
            const operand = try self.parsePostfix();
            return self.ast.addExpr(.{ .await_expr = operand });
        }
        if (self.current.tag == .ampersand) {
            self.advance(); // consume '&'
            const operand = try self.parsePostfix();
            return self.ast.addExpr(.{ .address_of = operand });
        }
        if (self.current.tag == .star) {
            self.advance(); // consume '*'
            const operand = try self.parseUnary();
            return self.ast.addExpr(.{ .deref = operand });
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
                        return self.parseStructLiteral(name, .{});
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
                return self.parseClosure();
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

    fn parseClosure(self: *Parser) ParseError!Ast.ExprIndex {
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

    fn parseStructLiteral(self: *Parser, name: []const u8, type_args: std.ArrayList(Ast.TypeExpr)) ParseError!Ast.ExprIndex {
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

    fn tryParseGenericStructLiteral(self: *Parser, name: []const u8) ParseError!?Ast.ExprIndex {
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
            const result = try self.parseStructLiteral(name, type_args);
            return result;
        }

        // Not a struct literal, restore state
        self.lexer = saved_lexer;
        self.current = saved_current;
        self.had_error = saved_had_error;
        self.restoreDiagnostics(saved_diag_count);
        return null;
    }

    fn restoreDiagnostics(self: *Parser, saved_count: usize) void {
        // Free and remove diagnostics added during speculative parsing
        for (self.diagnostics.errors.items[saved_count..]) |err| {
            self.diagnostics.allocator.free(err.message);
        }
        self.diagnostics.errors.items.len = saved_count;
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
}
