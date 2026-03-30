const std = @import("std");
const Ast = @import("ast.zig").Ast;
const Tag = @import("token.zig").Tag;
const Parser = @import("parser.zig").Parser;

pub fn parseBlock(self: *Parser) Parser.ParseError!Ast.Block {
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

pub fn parseStatement(self: *Parser) Parser.ParseError!Ast.Stmt {
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
            return parseExprStmt(self);
        },
        .identifier => {
            // Check for assignment: ident = expr;
            // We need to peek ahead
            if (self.peekIsAssignment()) {
                return self.parseAssignStmt();
            }
            return parseExprStmt(self);
        },
        else => return parseExprStmt(self),
    }
}

pub fn peekIsAssignment(self: *Parser) bool {
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

pub fn parseReturnStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
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

pub fn parseVarDecl(self: *Parser) Parser.ParseError!Ast.Stmt {
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

pub fn parseAssignStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
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

pub fn peekIsDerefAssignment(self: *Parser) bool {
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

pub fn parseDerefAssignStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
    // Parse the LHS as an expression (which will be a deref expression)
    // We consume '*' and parse the target
    self.advance(); // consume '*'
    const target = try self.parseUnary();
    try self.expect(.equal);
    const value = try self.parseExpr();
    try self.expect(.semicolon);
    return .{ .deref_assign = .{ .target = target, .value = value } };
}

pub fn parseIfStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
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

pub fn peekIsIfLet(self: *Parser) bool {
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

pub fn parseIfLetStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
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

pub fn parseForStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
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

pub fn parseWhileStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
    self.advance(); // consume 'while'
    const condition = try self.parseExpr();

    const body = try self.ast.allocator.create(Ast.Block);
    body.* = try self.parseBlock();

    return .{ .while_stmt = .{
        .condition = condition,
        .body = body,
    } };
}

pub fn parseMatchStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
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

pub fn parsePattern(self: *Parser) Parser.ParseError!Ast.Pattern {
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

pub fn parseUnsafeBlock(self: *Parser) Parser.ParseError!Ast.Stmt {
    self.advance(); // consume 'unsafe'
    const block = try self.ast.allocator.create(Ast.Block);
    block.* = try self.parseBlock();
    return .{ .unsafe_block = block };
}

pub fn parseSpawnStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
    self.advance(); // consume 'spawn'
    const expr = try self.parseExpr();
    try self.expect(.semicolon);
    return .{ .spawn_stmt = .{ .expr = expr } };
}

pub fn parseSpawnHandleStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
    self.advance(); // consume 'spawn_with_handle'
    const name_token = self.current;
    try self.expect(.identifier);
    const handle_name = name_token.slice(self.source);
    try self.expect(.equal);
    const expr = try self.parseExpr();
    try self.expect(.semicolon);
    return .{ .spawn_handle_stmt = .{ .handle_name = handle_name, .expr = expr } };
}

fn parseExprStmt(self: *Parser) Parser.ParseError!Ast.Stmt {
    const expr = try self.parseExpr();
    try self.expect(.semicolon);
    return .{ .expr_stmt = expr };
}
