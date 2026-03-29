const std = @import("std");

pub const NodeIndex = u32;
pub const null_node: NodeIndex = std.math.maxInt(u32);

pub const Ast = struct {
    allocator: std.mem.Allocator,
    program: Program,
    exprs: std.ArrayList(Expr) = .{},

    pub const Program = struct {
        package_name: []const u8 = "",
        decls: std.ArrayList(TopLevelDecl) = .{},
    };

    pub const Visibility = enum {
        public,
        private,
    };

    pub const TopLevelDecl = struct {
        visibility: Visibility,
        decl: DeclKind,
        start: u32,
    };

    pub const DeclKind = union(enum) {
        function: FnDecl,
        struct_decl: StructDecl,
        impl_decl: ImplDecl,
        enum_decl: EnumDecl,
    };

    pub const EnumDecl = struct {
        name: []const u8,
        variants: std.ArrayList(EnumVariant),
    };

    pub const EnumVariant = struct {
        name: []const u8,
        visibility: Visibility,
        data_types: std.ArrayList(TypeExpr),
    };

    pub const ImplDecl = struct {
        type_name: []const u8,
        methods: std.ArrayList(MethodDecl),
    };

    pub const MethodDecl = struct {
        visibility: Visibility,
        name: []const u8,
        has_self: bool,
        params: std.ArrayList(Param),
        return_type: TypeExpr,
        body: Block,
    };

    pub const StructDecl = struct {
        name: []const u8,
        fields: std.ArrayList(StructField),
    };

    pub const StructField = struct {
        name: []const u8,
        type_expr: TypeExpr,
        visibility: Visibility,
    };

    pub const FnDecl = struct {
        name: []const u8,
        params: std.ArrayList(Param),
        return_type: TypeExpr,
        body: Block,
    };

    pub const Param = struct {
        name: []const u8,
        type_expr: TypeExpr,
    };

    pub const Block = struct {
        stmts: std.ArrayList(Stmt),
    };

    pub const Stmt = union(enum) {
        return_stmt: ReturnStmt,
        expr_stmt: ExprIndex,
        var_decl: VarDecl,
        assign: AssignStmt,
        if_stmt: IfStmt,
        for_stmt: ForStmt,
        break_stmt,
        continue_stmt,
        match_stmt: MatchStmt,
    };

    pub const MatchStmt = struct {
        expr: ExprIndex,
        arms: std.ArrayList(MatchArm),
    };

    pub const MatchArm = struct {
        pattern: Pattern,
        body: *Block,
    };

    pub const Pattern = union(enum) {
        enum_pattern: EnumPattern,
        identifier: []const u8,
    };

    pub const EnumPattern = struct {
        enum_name: []const u8,
        variant_name: []const u8,
        bindings: std.ArrayList([]const u8),
    };

    pub const ReturnStmt = struct {
        expr: ExprIndex,
    };

    pub const VarDecl = struct {
        name: []const u8,
        is_mutable: bool,
        type_expr: TypeExpr,
        init_expr: ExprIndex,
    };

    pub const AssignStmt = struct {
        target: []const u8,
        target_fields: std.ArrayList([]const u8),
        target_index: ?ExprIndex = null,
        value: ExprIndex,
    };

    pub const IfStmt = struct {
        condition: ExprIndex,
        then_block: *Block,
        else_block: ?*Block,
    };

    pub const ForStmt = struct {
        iter_var: []const u8,
        iterable: ExprIndex,
        body: *Block,
    };

    // Expression system
    pub const ExprIndex = u32;
    pub const null_expr: ExprIndex = std.math.maxInt(u32);

    pub const Expr = union(enum) {
        int_literal: i64,
        float_literal: f64,
        bool_literal: bool,
        string_literal: []const u8,
        identifier: []const u8,
        binary: BinaryExpr,
        unary: UnaryExpr,
        call: CallExpr,
        grouped: ExprIndex,
        field_access: FieldAccessExpr,
        struct_literal: StructLiteralExpr,
        method_call: MethodCallExpr,
        enum_constructor: EnumConstructorExpr,
        array_literal: ArrayLiteralExpr,
        index_access: IndexAccessExpr,
        tuple_literal: TupleLiteralExpr,
    };

    pub const TupleLiteralExpr = struct {
        elements: std.ArrayList(ExprIndex),
    };

    pub const ArrayLiteralExpr = struct {
        elements: std.ArrayList(ExprIndex),
    };

    pub const IndexAccessExpr = struct {
        object: ExprIndex,
        index: ExprIndex,
    };

    pub const EnumConstructorExpr = struct {
        enum_name: []const u8,
        variant_name: []const u8,
        args: std.ArrayList(ExprIndex),
    };

    pub const MethodCallExpr = struct {
        object: ExprIndex,
        method: []const u8,
        args: std.ArrayList(ExprIndex),
    };

    pub const FieldAccessExpr = struct {
        object: ExprIndex,
        field: []const u8,
    };

    pub const StructLiteralExpr = struct {
        name: []const u8,
        field_inits: std.ArrayList(FieldInitExpr),
    };

    pub const FieldInitExpr = struct {
        name: []const u8,
        value: ExprIndex,
    };

    pub const BinaryExpr = struct {
        lhs: ExprIndex,
        op: BinaryOp,
        rhs: ExprIndex,
    };

    pub const BinaryOp = enum {
        add,
        sub,
        mul,
        div,
        mod,
        eq,
        neq,
        lt,
        le,
        gt,
        ge,
        logic_and,
        logic_or,
        bit_and,
        bit_or,
        bit_xor,
        shift_left,
        shift_right,
    };

    pub const UnaryExpr = struct {
        op: UnaryOp,
        operand: ExprIndex,
    };

    pub const UnaryOp = enum {
        negate,
        logic_not,
        bit_not,
    };

    pub const CallExpr = struct {
        callee: ExprIndex,
        args: std.ArrayList(ExprIndex),
    };

    // Type expressions
    pub const TypeExpr = union(enum) {
        builtin: BuiltinType,
        named: []const u8,
        array_type: ArrayTypeExpr,
        option_type: OptionTypeExpr,
        result_type: ResultTypeExpr,
        tuple_type: TupleTypeExpr,
    };

    pub const TupleTypeExpr = struct {
        element_types: std.ArrayList(TypeExpr),
    };

    pub const OptionTypeExpr = struct {
        inner_type: *const TypeExpr,
    };

    pub const ResultTypeExpr = struct {
        ok_type: *const TypeExpr,
        err_type: *const TypeExpr,
    };

    pub const ArrayTypeExpr = struct {
        size: u32,
        element_type: *const TypeExpr,
    };

    pub const BuiltinType = enum {
        void,
        bool_type,
        int_type,
        float_type,
        string_type,
        i8_type,
        i16_type,
        i32_type,
        i64_type,
        i128_type,
        u8_type,
        u16_type,
        u32_type,
        u64_type,
        u128_type,
        f32_type,
        f64_type,
        usize_type,
        isize_type,
        never_type,
    };

    pub fn init(allocator: std.mem.Allocator) Ast {
        return .{
            .allocator = allocator,
            .program = .{},
        };
    }

    pub fn addExpr(self: *Ast, expr: Expr) !ExprIndex {
        const index: ExprIndex = @intCast(self.exprs.items.len);
        try self.exprs.append(self.allocator, expr);
        return index;
    }

    pub fn getExpr(self: *const Ast, index: ExprIndex) Expr {
        return self.exprs.items[index];
    }
};
