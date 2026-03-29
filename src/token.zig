const std = @import("std");

pub const Token = struct {
    tag: Tag,
    loc: Loc,

    pub const Loc = struct {
        start: u32,
        end: u32,
    };

    pub fn slice(self: Token, source: []const u8) []const u8 {
        return source[self.loc.start..self.loc.end];
    }
};

pub const Tag = enum {
    // Literals
    int_literal,
    float_literal,
    string_literal,
    char_literal,
    identifier,

    // Delimiters
    lparen,
    rparen,
    lbrace,
    rbrace,
    lbracket,
    rbracket,
    semicolon,
    colon,
    comma,
    dot,
    hash_lbracket, // #[

    // Operators
    plus,
    minus,
    star,
    slash,
    percent,
    ampersand,
    pipe,
    caret,
    tilde,
    bang,
    equal,
    double_equal,
    bang_equal,
    less,
    less_equal,
    greater,
    greater_equal,
    shift_left,
    shift_right,
    double_ampersand,
    double_pipe,
    arrow, // ->
    double_colon, // ::

    // Keywords
    keyword_package,
    keyword_import,
    keyword_as,
    keyword_fn,
    keyword_let,
    keyword_const,
    keyword_mut,
    keyword_if,
    keyword_else,
    keyword_for,
    keyword_in,
    keyword_match,
    keyword_return,
    keyword_break,
    keyword_continue,
    keyword_pub,
    keyword_priv,
    keyword_struct,
    keyword_enum,
    keyword_impl,
    keyword_self,
    keyword_extern,
    keyword_unsafe,
    keyword_spawn,
    keyword_spawn_with_handle,
    keyword_await,
    keyword_sizeof,
    keyword_void,
    keyword_none,
    keyword_true,
    keyword_false,

    // Built-in type keywords
    keyword_Result,
    keyword_Option,
    keyword_Never,
    keyword_int,
    keyword_float,
    keyword_bool,
    keyword_string,
    keyword_usize,
    keyword_isize,
    keyword_i8,
    keyword_i16,
    keyword_i32,
    keyword_i64,
    keyword_i128,
    keyword_u8,
    keyword_u16,
    keyword_u32,
    keyword_u64,
    keyword_u128,
    keyword_f32,
    keyword_f64,

    // Special
    eof,
    invalid,

    pub fn isTypeKeyword(self: Tag) bool {
        return switch (self) {
            .keyword_int,
            .keyword_float,
            .keyword_bool,
            .keyword_string,
            .keyword_usize,
            .keyword_isize,
            .keyword_i8,
            .keyword_i16,
            .keyword_i32,
            .keyword_i64,
            .keyword_i128,
            .keyword_u8,
            .keyword_u16,
            .keyword_u32,
            .keyword_u64,
            .keyword_u128,
            .keyword_f32,
            .keyword_f64,
            .keyword_void,
            .keyword_Never,
            .keyword_Result,
            .keyword_Option,
            => true,
            else => false,
        };
    }
};

pub const keywords = std.StaticStringMap(Tag).initComptime(.{
    .{ "package", .keyword_package },
    .{ "import", .keyword_import },
    .{ "as", .keyword_as },
    .{ "fn", .keyword_fn },
    .{ "let", .keyword_let },
    .{ "const", .keyword_const },
    .{ "mut", .keyword_mut },
    .{ "if", .keyword_if },
    .{ "else", .keyword_else },
    .{ "for", .keyword_for },
    .{ "in", .keyword_in },
    .{ "match", .keyword_match },
    .{ "return", .keyword_return },
    .{ "break", .keyword_break },
    .{ "continue", .keyword_continue },
    .{ "pub", .keyword_pub },
    .{ "priv", .keyword_priv },
    .{ "struct", .keyword_struct },
    .{ "enum", .keyword_enum },
    .{ "impl", .keyword_impl },
    .{ "self", .keyword_self },
    .{ "extern", .keyword_extern },
    .{ "unsafe", .keyword_unsafe },
    .{ "spawn", .keyword_spawn },
    .{ "spawn_with_handle", .keyword_spawn_with_handle },
    .{ "await", .keyword_await },
    .{ "sizeof", .keyword_sizeof },
    .{ "void", .keyword_void },
    .{ "none", .keyword_none },
    .{ "true", .keyword_true },
    .{ "false", .keyword_false },
    .{ "Result", .keyword_Result },
    .{ "Option", .keyword_Option },
    .{ "Never", .keyword_Never },
    .{ "int", .keyword_int },
    .{ "float", .keyword_float },
    .{ "bool", .keyword_bool },
    .{ "string", .keyword_string },
    .{ "usize", .keyword_usize },
    .{ "isize", .keyword_isize },
    .{ "i8", .keyword_i8 },
    .{ "i16", .keyword_i16 },
    .{ "i32", .keyword_i32 },
    .{ "i64", .keyword_i64 },
    .{ "i128", .keyword_i128 },
    .{ "u8", .keyword_u8 },
    .{ "u16", .keyword_u16 },
    .{ "u32", .keyword_u32 },
    .{ "u64", .keyword_u64 },
    .{ "u128", .keyword_u128 },
    .{ "f32", .keyword_f32 },
    .{ "f64", .keyword_f64 },
});

pub fn getKeyword(bytes: []const u8) ?Tag {
    return keywords.get(bytes);
}

// --- Tests ---

const testing = std.testing;

test "getKeyword returns correct tags for keywords" {
    try testing.expectEqual(Tag.keyword_fn, getKeyword("fn").?);
    try testing.expectEqual(Tag.keyword_let, getKeyword("let").?);
    try testing.expectEqual(Tag.keyword_pub, getKeyword("pub").?);
    try testing.expectEqual(Tag.keyword_priv, getKeyword("priv").?);
    try testing.expectEqual(Tag.keyword_return, getKeyword("return").?);
    try testing.expectEqual(Tag.keyword_if, getKeyword("if").?);
    try testing.expectEqual(Tag.keyword_else, getKeyword("else").?);
    try testing.expectEqual(Tag.keyword_for, getKeyword("for").?);
    try testing.expectEqual(Tag.keyword_mut, getKeyword("mut").?);
    try testing.expectEqual(Tag.keyword_void, getKeyword("void").?);
    try testing.expectEqual(Tag.keyword_i32, getKeyword("i32").?);
    try testing.expectEqual(Tag.keyword_true, getKeyword("true").?);
    try testing.expectEqual(Tag.keyword_false, getKeyword("false").?);
    try testing.expectEqual(Tag.keyword_package, getKeyword("package").?);
    try testing.expectEqual(Tag.keyword_Result, getKeyword("Result").?);
    try testing.expectEqual(Tag.keyword_Option, getKeyword("Option").?);
}

test "getKeyword returns null for non-keywords" {
    try testing.expect(getKeyword("foo") == null);
    try testing.expect(getKeyword("main") == null);
    try testing.expect(getKeyword("x") == null);
    try testing.expect(getKeyword("println") == null);
    try testing.expect(getKeyword("") == null);
}

test "isTypeKeyword identifies type keywords" {
    try testing.expect(Tag.keyword_i32.isTypeKeyword());
    try testing.expect(Tag.keyword_f64.isTypeKeyword());
    try testing.expect(Tag.keyword_bool.isTypeKeyword());
    try testing.expect(Tag.keyword_string.isTypeKeyword());
    try testing.expect(Tag.keyword_void.isTypeKeyword());
    try testing.expect(Tag.keyword_Result.isTypeKeyword());
    try testing.expect(Tag.keyword_Option.isTypeKeyword());
    try testing.expect(Tag.keyword_Never.isTypeKeyword());
    try testing.expect(Tag.keyword_usize.isTypeKeyword());
}

test "isTypeKeyword rejects non-type keywords" {
    try testing.expect(!Tag.keyword_fn.isTypeKeyword());
    try testing.expect(!Tag.keyword_let.isTypeKeyword());
    try testing.expect(!Tag.keyword_if.isTypeKeyword());
    try testing.expect(!Tag.keyword_return.isTypeKeyword());
    try testing.expect(!Tag.identifier.isTypeKeyword());
    try testing.expect(!Tag.int_literal.isTypeKeyword());
}

test "Token.slice extracts correct text" {
    const source = "let x: i32 = 42;";
    const tok = Token{ .tag = .keyword_let, .loc = .{ .start = 0, .end = 3 } };
    try testing.expectEqualStrings("let", tok.slice(source));

    const tok2 = Token{ .tag = .int_literal, .loc = .{ .start = 14, .end = 16 } };
    try testing.expectEqualStrings("42", tok2.slice(source));
}
