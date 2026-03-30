const std = @import("std");
const Ast = @import("ast.zig").Ast;
const codegen_mod = @import("codegen.zig");
const CodeGen = codegen_mod.CodeGen;
const c = codegen_mod.c;
const codegen_exprs = @import("codegen_exprs.zig");

pub fn genStdlibCall(self: *CodeGen, namespace: []const u8, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, namespace, "math")) {
        return genMathCall(self, func, args);
    } else if (std.mem.eql(u8, namespace, "str")) {
        return genStrCall(self, func, args);
    } else if (std.mem.eql(u8, namespace, "io")) {
        return genIoCall(self, func, args);
    } else if (std.mem.eql(u8, namespace, "os")) {
        return genOsCall(self, func, args);
    }
    self.diagnostics.report(.@"error", 0, "unknown stdlib namespace '{s}'", .{namespace});
    return error.CodeGenError;
}

pub fn genMathConstant(self: *CodeGen, field: []const u8) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, field, "PI")) {
        return .{
            .value = c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), 3.14159265358979323846),
            .type_tag = .f64_type,
        };
    } else if (std.mem.eql(u8, field, "E")) {
        return .{
            .value = c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), 2.71828182845904523536),
            .type_tag = .f64_type,
        };
    }
    self.diagnostics.report(.@"error", 0, "unknown math constant '{s}'", .{field});
    return error.CodeGenError;
}

pub fn ensureF64(self: *CodeGen, result: CodeGen.ExprResult) c.LLVMValueRef {
    return switch (result.type_tag) {
        .i32_type => c.LLVMBuildSIToFP(self.builder, result.value, c.LLVMDoubleTypeInContext(self.context), "i2f"),
        .i64_type => c.LLVMBuildSIToFP(self.builder, result.value, c.LLVMDoubleTypeInContext(self.context), "i64_2f"),
        else => result.value,
    };
}

fn genMathCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, func, "sqrt")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.sqrt() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const f64_val = ensureF64(self, arg);
        const fn_type = c.LLVMGlobalGetValueType(self.sqrt_fn);
        var call_args = [_]c.LLVMValueRef{f64_val};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.sqrt_fn, &call_args, 1, "sqrt_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "pow")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "math.pow() expects 2 arguments", .{}); return error.CodeGenError; }
        const a = try codegen_exprs.genExpr(self, args.items[0]);
        const b = try codegen_exprs.genExpr(self, args.items[1]);
        const fn_type = c.LLVMGlobalGetValueType(self.pow_fn);
        var call_args = [_]c.LLVMValueRef{ ensureF64(self, a), ensureF64(self, b) };
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.pow_fn, &call_args, 2, "pow_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "abs")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.abs() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        if (arg.type_tag == .i32_type) {
            const zero = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);
            const is_neg = c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, arg.value, zero, "is_neg");
            const neg_val = c.LLVMBuildNSWSub(self.builder, zero, arg.value, "neg");
            return .{ .value = c.LLVMBuildSelect(self.builder, is_neg, neg_val, arg.value, "abs_res"), .type_tag = .i32_type };
        } else {
            const f64_val = ensureF64(self, arg);
            const fn_type = c.LLVMGlobalGetValueType(self.fabs_fn);
            var call_args = [_]c.LLVMValueRef{f64_val};
            return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.fabs_fn, &call_args, 1, "fabs_res"), .type_tag = .f64_type };
        }
    } else if (std.mem.eql(u8, func, "min")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "math.min() expects 2 arguments", .{}); return error.CodeGenError; }
        const a = try codegen_exprs.genExpr(self, args.items[0]);
        const b = try codegen_exprs.genExpr(self, args.items[1]);
        if (a.type_tag == .i32_type and b.type_tag == .i32_type) {
            const cmp = c.LLVMBuildICmp(self.builder, c.LLVMIntSLT, a.value, b.value, "min_cmp");
            return .{ .value = c.LLVMBuildSelect(self.builder, cmp, a.value, b.value, "min_res"), .type_tag = .i32_type };
        } else {
            const fn_type = c.LLVMGlobalGetValueType(self.fmin_fn);
            var call_args = [_]c.LLVMValueRef{ ensureF64(self, a), ensureF64(self, b) };
            return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.fmin_fn, &call_args, 2, "fmin_res"), .type_tag = .f64_type };
        }
    } else if (std.mem.eql(u8, func, "max")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "math.max() expects 2 arguments", .{}); return error.CodeGenError; }
        const a = try codegen_exprs.genExpr(self, args.items[0]);
        const b = try codegen_exprs.genExpr(self, args.items[1]);
        if (a.type_tag == .i32_type and b.type_tag == .i32_type) {
            const cmp = c.LLVMBuildICmp(self.builder, c.LLVMIntSGT, a.value, b.value, "max_cmp");
            return .{ .value = c.LLVMBuildSelect(self.builder, cmp, a.value, b.value, "max_res"), .type_tag = .i32_type };
        } else {
            const fn_type = c.LLVMGlobalGetValueType(self.fmax_fn);
            var call_args = [_]c.LLVMValueRef{ ensureF64(self, a), ensureF64(self, b) };
            return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.fmax_fn, &call_args, 2, "fmax_res"), .type_tag = .f64_type };
        }
    } else if (std.mem.eql(u8, func, "floor")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.floor() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.floor_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.floor_fn, &call_args, 1, "floor_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "ceil")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.ceil() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.ceil_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.ceil_fn, &call_args, 1, "ceil_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "sin")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.sin() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.sin_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.sin_fn, &call_args, 1, "sin_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "cos")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.cos() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.cos_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.cos_fn, &call_args, 1, "cos_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "tan")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.tan() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.tan_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.tan_fn, &call_args, 1, "tan_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "log")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.log() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.math_log_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.math_log_fn, &call_args, 1, "log_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "log10")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.log10() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.log10_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.log10_fn, &call_args, 1, "log10_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "exp")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.exp() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.exp_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.exp_fn, &call_args, 1, "exp_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "round")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "math.round() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.round_fn);
        var call_args = [_]c.LLVMValueRef{ensureF64(self, arg)};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.round_fn, &call_args, 1, "round_res"), .type_tag = .f64_type };
    }
    self.diagnostics.report(.@"error", 0, "unknown math function '{s}'", .{func});
    return error.CodeGenError;
}

fn genStrCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, func, "to_int")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.to_int() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.atoi_fn);
        var call_args = [_]c.LLVMValueRef{arg.value};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.atoi_fn, &call_args, 1, "atoi_res"), .type_tag = .i32_type };
    } else if (std.mem.eql(u8, func, "to_float")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.to_float() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.atof_fn);
        var call_args = [_]c.LLVMValueRef{arg.value};
        return .{ .value = c.LLVMBuildCall2(self.builder, fn_type, self.atof_fn, &call_args, 1, "atof_res"), .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "from_int")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.from_int() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        // malloc(32) + sprintf(buf, "%d", n)
        const buf_size = c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 32, 0);
        const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
        var malloc_args = [_]c.LLVMValueRef{buf_size};
        const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "buf");
        const sprintf_fn_type = c.LLVMGlobalGetValueType(self.sprintf_fn);
        var sprintf_args = [_]c.LLVMValueRef{ buf, self.fmt_int_raw, arg.value };
        _ = c.LLVMBuildCall2(self.builder, sprintf_fn_type, self.sprintf_fn, &sprintf_args, 3, "");
        return .{ .value = buf, .type_tag = .string_type };
    } else if (std.mem.eql(u8, func, "from_float")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.from_float() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const buf_size = c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 64, 0);
        const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
        var malloc_args = [_]c.LLVMValueRef{buf_size};
        const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "buf");
        const sprintf_fn_type = c.LLVMGlobalGetValueType(self.sprintf_fn);
        var sprintf_args = [_]c.LLVMValueRef{ buf, self.fmt_float_raw, ensureF64(self, arg) };
        _ = c.LLVMBuildCall2(self.builder, sprintf_fn_type, self.sprintf_fn, &sprintf_args, 3, "");
        return .{ .value = buf, .type_tag = .string_type };
    } else if (std.mem.eql(u8, func, "contains")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.contains() expects 2 arguments", .{}); return error.CodeGenError; }
        const s = try codegen_exprs.genExpr(self, args.items[0]);
        const sub = try codegen_exprs.genExpr(self, args.items[1]);
        const fn_type = c.LLVMGlobalGetValueType(self.strstr_fn);
        var call_args = [_]c.LLVMValueRef{ s.value, sub.value };
        const result = c.LLVMBuildCall2(self.builder, fn_type, self.strstr_fn, &call_args, 2, "strstr_res");
        const null_ptr = c.LLVMConstNull(c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0));
        const is_not_null = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, result, null_ptr, "contains_res");
        return .{ .value = is_not_null, .type_tag = .bool_type };
    } else if (std.mem.eql(u8, func, "starts_with")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.starts_with() expects 2 arguments", .{}); return error.CodeGenError; }
        const s = try codegen_exprs.genExpr(self, args.items[0]);
        const prefix = try codegen_exprs.genExpr(self, args.items[1]);
        // strlen(prefix)
        const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
        var strlen_args = [_]c.LLVMValueRef{prefix.value};
        const prefix_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "prefix_len");
        // strncmp(s, prefix, prefix_len) == 0
        const strncmp_fn_type = c.LLVMGlobalGetValueType(self.strncmp_fn);
        var cmp_args = [_]c.LLVMValueRef{ s.value, prefix.value, prefix_len };
        const cmp_result = c.LLVMBuildCall2(self.builder, strncmp_fn_type, self.strncmp_fn, &cmp_args, 3, "cmp_res");
        const zero = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);
        const is_eq = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, cmp_result, zero, "starts_with_res");
        return .{ .value = is_eq, .type_tag = .bool_type };
    } else if (std.mem.eql(u8, func, "ends_with")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.ends_with() expects 2 arguments", .{}); return error.CodeGenError; }
        const s = try codegen_exprs.genExpr(self, args.items[0]);
        const suffix = try codegen_exprs.genExpr(self, args.items[1]);
        const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
        // strlen(s) and strlen(suffix)
        var s_len_args = [_]c.LLVMValueRef{s.value};
        const s_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &s_len_args, 1, "s_len");
        var suf_len_args = [_]c.LLVMValueRef{suffix.value};
        const suf_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &suf_len_args, 1, "suf_len");
        // offset = s_len - suf_len
        const offset = c.LLVMBuildSub(self.builder, s_len, suf_len, "offset");
        // Check offset >= 0 (unsigned: suf_len <= s_len)
        const is_long_enough = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, s_len, suf_len, "long_enough");
        // s + offset
        var gep_offset = [_]c.LLVMValueRef{offset};
        const s_end = c.LLVMBuildGEP2(self.builder, c.LLVMInt8TypeInContext(self.context), s.value, &gep_offset, 1, "s_end");
        // strncmp(s + offset, suffix, suf_len) == 0
        const strncmp_fn_type = c.LLVMGlobalGetValueType(self.strncmp_fn);
        var cmp_args = [_]c.LLVMValueRef{ s_end, suffix.value, suf_len };
        const cmp_result = c.LLVMBuildCall2(self.builder, strncmp_fn_type, self.strncmp_fn, &cmp_args, 3, "cmp_res");
        const zero = c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 0, 0);
        const is_match = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, cmp_result, zero, "match");
        // result = is_long_enough AND is_match
        const result = c.LLVMBuildAnd(self.builder, is_long_enough, is_match, "ends_with_res");
        return .{ .value = result, .type_tag = .bool_type };
    } else if (std.mem.eql(u8, func, "upper") or std.mem.eql(u8, func, "lower")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.{s}() expects 1 argument", .{func}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const conv_fn = if (std.mem.eql(u8, func, "upper")) self.toupper_fn else self.tolower_fn;
        return genStrCaseConvert(self, arg.value, conv_fn);
    } else if (std.mem.eql(u8, func, "trim")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "str.trim() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        return genStrTrim(self, arg.value);
    } else if (std.mem.eql(u8, func, "index_of")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.index_of() expects 2 arguments", .{}); return error.CodeGenError; }
        const haystack = try codegen_exprs.genExpr(self, args.items[0]);
        const needle = try codegen_exprs.genExpr(self, args.items[1]);
        return genStrIndexOf(self, haystack.value, needle.value);
    } else if (std.mem.eql(u8, func, "replace")) {
        if (args.items.len != 3) { self.diagnostics.report(.@"error", 0, "str.replace() expects 3 arguments", .{}); return error.CodeGenError; }
        const text = try codegen_exprs.genExpr(self, args.items[0]);
        const find = try codegen_exprs.genExpr(self, args.items[1]);
        const replacement = try codegen_exprs.genExpr(self, args.items[2]);
        return genStrReplace(self, text.value, find.value, replacement.value);
    } else if (std.mem.eql(u8, func, "split")) {
        if (args.items.len != 2) { self.diagnostics.report(.@"error", 0, "str.split() expects 2 arguments", .{}); return error.CodeGenError; }
        const text = try codegen_exprs.genExpr(self, args.items[0]);
        const delimiter = try codegen_exprs.genExpr(self, args.items[1]);
        return genStrSplit(self, text.value, delimiter.value);
    }
    self.diagnostics.report(.@"error", 0, "unknown str function '{s}'", .{func});
    return error.CodeGenError;
}

fn genStrCaseConvert(self: *CodeGen, str_val: c.LLVMValueRef, conv_fn: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const i8_type = c.LLVMInt8TypeInContext(self.context);
    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);

    // Get string length
    const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    var strlen_args = [_]c.LLVMValueRef{str_val};
    const len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "len");

    // malloc(len + 1)
    const one = c.LLVMConstInt(i64_llvm, 1, 0);
    const buf_size = c.LLVMBuildAdd(self.builder, len, one, "buf_size");
    const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
    var malloc_args = [_]c.LLVMValueRef{buf_size};
    const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "buf");

    // Get current function for basic blocks
    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Loop: for i in 0..len: buf[i] = conv(src[i])
    const loop_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "case_loop");
    const loop_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "case_body");
    const loop_end = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "case_end");

    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Loop header: i = phi(0, entry)(i_next, body); if i >= len, exit
    c.LLVMPositionBuilderAtEnd(self.builder, loop_header);
    const i_phi = c.LLVMBuildPhi(self.builder, i64_llvm, "i");
    const done = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, i_phi, len, "done");
    _ = c.LLVMBuildCondBr(self.builder, done, loop_end, loop_body);

    // Loop body: load char, convert, store
    c.LLVMPositionBuilderAtEnd(self.builder, loop_body);
    var gep_i = [_]c.LLVMValueRef{i_phi};
    const src_ptr = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_i, 1, "src_ptr");
    const ch = c.LLVMBuildLoad2(self.builder, i8_type, src_ptr, "ch");
    const ch_ext = c.LLVMBuildZExt(self.builder, ch, i32_llvm, "ch_ext");
    const conv_fn_type = c.LLVMGlobalGetValueType(conv_fn);
    var conv_args = [_]c.LLVMValueRef{ch_ext};
    const converted = c.LLVMBuildCall2(self.builder, conv_fn_type, conv_fn, &conv_args, 1, "converted");
    const trunc = c.LLVMBuildTrunc(self.builder, converted, i8_type, "trunc");
    var gep_i2 = [_]c.LLVMValueRef{i_phi};
    const dst_ptr = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_i2, 1, "dst_ptr");
    _ = c.LLVMBuildStore(self.builder, trunc, dst_ptr);
    const i_next = c.LLVMBuildAdd(self.builder, i_phi, one, "i_next");
    _ = c.LLVMBuildBr(self.builder, loop_header);

    // Wire phi
    const zero_i64 = c.LLVMConstInt(i64_llvm, 0, 0);
    const entry_bb = c.LLVMGetPreviousBasicBlock(loop_header);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{zero_i64}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(i_phi, @constCast(&[_]c.LLVMValueRef{i_next}), @constCast(&[_]c.LLVMBasicBlockRef{loop_body}), 1);

    // Null-terminate
    c.LLVMPositionBuilderAtEnd(self.builder, loop_end);
    var gep_len = [_]c.LLVMValueRef{len};
    const null_ptr = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_len, 1, "null_ptr");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(i8_type, 0, 0), null_ptr);

    return .{ .value = buf, .type_tag = .string_type };
}

fn genStrTrim(self: *CodeGen, str_val: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const i8_type = c.LLVMInt8TypeInContext(self.context);
    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);
    const zero_i64 = c.LLVMConstInt(i64_llvm, 0, 0);
    const one_i64 = c.LLVMConstInt(i64_llvm, 1, 0);

    // Get string length
    const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    var strlen_args = [_]c.LLVMValueRef{str_val};
    const len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "len");

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Scan forward: find first non-space
    const fwd_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_fwd");
    const fwd_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_fwd_body");
    const fwd_done = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_fwd_done");

    _ = c.LLVMBuildBr(self.builder, fwd_header);
    c.LLVMPositionBuilderAtEnd(self.builder, fwd_header);
    const start_phi = c.LLVMBuildPhi(self.builder, i64_llvm, "start");
    const fwd_at_end = c.LLVMBuildICmp(self.builder, c.LLVMIntUGE, start_phi, len, "fwd_at_end");
    _ = c.LLVMBuildCondBr(self.builder, fwd_at_end, fwd_done, fwd_body);

    c.LLVMPositionBuilderAtEnd(self.builder, fwd_body);
    var gep_start = [_]c.LLVMValueRef{start_phi};
    const fwd_ptr = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_start, 1, "fwd_ptr");
    const fwd_ch = c.LLVMBuildLoad2(self.builder, i8_type, fwd_ptr, "fwd_ch");
    const fwd_ch_ext = c.LLVMBuildZExt(self.builder, fwd_ch, i32_llvm, "fwd_ch_ext");
    const isspace_fn_type = c.LLVMGlobalGetValueType(self.isspace_fn);
    var isspace_args = [_]c.LLVMValueRef{fwd_ch_ext};
    const is_space_fwd = c.LLVMBuildCall2(self.builder, isspace_fn_type, self.isspace_fn, &isspace_args, 1, "is_space");
    const is_space_bool = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, is_space_fwd, c.LLVMConstInt(i32_llvm, 0, 0), "is_sp");
    const start_next = c.LLVMBuildAdd(self.builder, start_phi, one_i64, "start_next");
    // If space, continue; else, done
    _ = c.LLVMBuildCondBr(self.builder, is_space_bool, fwd_header, fwd_done);

    // Wire start_phi
    const entry_bb = c.LLVMGetPreviousBasicBlock(fwd_header);
    c.LLVMAddIncoming(start_phi, @constCast(&[_]c.LLVMValueRef{zero_i64}), @constCast(&[_]c.LLVMBasicBlockRef{entry_bb}), 1);
    c.LLVMAddIncoming(start_phi, @constCast(&[_]c.LLVMValueRef{start_next}), @constCast(&[_]c.LLVMBasicBlockRef{fwd_body}), 1);

    // fwd_done: start = phi(start_phi from header, start_phi from body)
    c.LLVMPositionBuilderAtEnd(self.builder, fwd_done);
    const final_start = c.LLVMBuildPhi(self.builder, i64_llvm, "final_start");
    c.LLVMAddIncoming(final_start, @constCast(&[_]c.LLVMValueRef{ start_phi, start_phi }), @constCast(&[_]c.LLVMBasicBlockRef{ fwd_header, fwd_body }), 2);

    // Scan backward: find last non-space (end index, exclusive)
    const bwd_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_bwd");
    const bwd_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_bwd_body");
    const bwd_done = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "trim_bwd_done");

    _ = c.LLVMBuildBr(self.builder, bwd_header);
    c.LLVMPositionBuilderAtEnd(self.builder, bwd_header);
    const end_phi = c.LLVMBuildPhi(self.builder, i64_llvm, "end");
    const bwd_at_start = c.LLVMBuildICmp(self.builder, c.LLVMIntULE, end_phi, final_start, "bwd_at_start");
    _ = c.LLVMBuildCondBr(self.builder, bwd_at_start, bwd_done, bwd_body);

    c.LLVMPositionBuilderAtEnd(self.builder, bwd_body);
    const end_minus_1 = c.LLVMBuildSub(self.builder, end_phi, one_i64, "end_m1");
    var gep_end_m1 = [_]c.LLVMValueRef{end_minus_1};
    const bwd_ptr = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_end_m1, 1, "bwd_ptr");
    const bwd_ch = c.LLVMBuildLoad2(self.builder, i8_type, bwd_ptr, "bwd_ch");
    const bwd_ch_ext = c.LLVMBuildZExt(self.builder, bwd_ch, i32_llvm, "bwd_ch_ext");
    var bwd_isspace_args = [_]c.LLVMValueRef{bwd_ch_ext};
    const is_space_bwd = c.LLVMBuildCall2(self.builder, isspace_fn_type, self.isspace_fn, &bwd_isspace_args, 1, "is_space_bwd");
    const is_space_bwd_bool = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, is_space_bwd, c.LLVMConstInt(i32_llvm, 0, 0), "is_sp_bwd");
    _ = c.LLVMBuildCondBr(self.builder, is_space_bwd_bool, bwd_header, bwd_done);

    // Wire end_phi
    c.LLVMAddIncoming(end_phi, @constCast(&[_]c.LLVMValueRef{len}), @constCast(&[_]c.LLVMBasicBlockRef{fwd_done}), 1);
    c.LLVMAddIncoming(end_phi, @constCast(&[_]c.LLVMValueRef{end_minus_1}), @constCast(&[_]c.LLVMBasicBlockRef{bwd_body}), 1);

    // bwd_done: end = phi(end_phi from header, end_phi from body)
    c.LLVMPositionBuilderAtEnd(self.builder, bwd_done);
    const final_end = c.LLVMBuildPhi(self.builder, i64_llvm, "final_end");
    c.LLVMAddIncoming(final_end, @constCast(&[_]c.LLVMValueRef{ end_phi, end_phi }), @constCast(&[_]c.LLVMBasicBlockRef{ bwd_header, bwd_body }), 2);

    // Copy trimmed substring: len = final_end - final_start
    const trimmed_len = c.LLVMBuildSub(self.builder, final_end, final_start, "trimmed_len");
    const buf_size = c.LLVMBuildAdd(self.builder, trimmed_len, one_i64, "buf_size");
    const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
    var malloc_args = [_]c.LLVMValueRef{buf_size};
    const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "trim_buf");

    // src = str_val + final_start
    var gep_final_start = [_]c.LLVMValueRef{final_start};
    const src = c.LLVMBuildGEP2(self.builder, i8_type, str_val, &gep_final_start, 1, "trim_src");
    const memcpy_fn_type = c.LLVMGlobalGetValueType(self.memcpy_fn);
    var memcpy_args = [_]c.LLVMValueRef{ buf, src, trimmed_len };
    _ = c.LLVMBuildCall2(self.builder, memcpy_fn_type, self.memcpy_fn, &memcpy_args, 3, "");

    // Null-terminate
    var gep_trimmed = [_]c.LLVMValueRef{trimmed_len};
    const null_pos = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_trimmed, 1, "null_pos");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(i8_type, 0, 0), null_pos);

    return .{ .value = buf, .type_tag = .string_type };
}

fn genStrIndexOf(self: *CodeGen, haystack: c.LLVMValueRef, needle: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);
    const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);

    // Call strstr(haystack, needle)
    const strstr_fn_type = c.LLVMGlobalGetValueType(self.strstr_fn);
    var strstr_args = [_]c.LLVMValueRef{ haystack, needle };
    const match_ptr = c.LLVMBuildCall2(self.builder, strstr_fn_type, self.strstr_fn, &strstr_args, 2, "index_match");

    // Check if null
    const null_ptr = c.LLVMConstNull(i8ptr_ty);
    const is_found = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, match_ptr, null_ptr, "is_found");

    // Compute offset: match_ptr - haystack
    const haystack_int = c.LLVMBuildPtrToInt(self.builder, haystack, i64_llvm, "h_int");
    const match_int = c.LLVMBuildPtrToInt(self.builder, match_ptr, i64_llvm, "m_int");
    const offset_i64 = c.LLVMBuildSub(self.builder, match_int, haystack_int, "offset");
    const offset_i32 = c.LLVMBuildTrunc(self.builder, offset_i64, i32_llvm, "offset_i32");

    // Select: found ? offset : -1
    const neg_one = c.LLVMConstInt(i32_llvm, @as(c_ulonglong, @bitCast(@as(c_longlong, -1))), 0);
    const result = c.LLVMBuildSelect(self.builder, is_found, offset_i32, neg_one, "index_of_res");
    return .{ .value = result, .type_tag = .i32_type };
}

fn genStrReplace(self: *CodeGen, text: c.LLVMValueRef, find: c.LLVMValueRef, replacement: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const i8_type = c.LLVMInt8TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);
    const i8ptr_ty = c.LLVMPointerType(i8_type, 0);
    const one_i64 = c.LLVMConstInt(i64_llvm, 1, 0);

    const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    const memcpy_fn_type = c.LLVMGlobalGetValueType(self.memcpy_fn);
    const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);

    // Find match
    const strstr_fn_type = c.LLVMGlobalGetValueType(self.strstr_fn);
    var strstr_args = [_]c.LLVMValueRef{ text, find };
    const match_ptr = c.LLVMBuildCall2(self.builder, strstr_fn_type, self.strstr_fn, &strstr_args, 2, "repl_match");

    // If no match, return original string
    const null_ptr = c.LLVMConstNull(i8ptr_ty);
    const is_found = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, match_ptr, null_ptr, "repl_found");

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
    const do_replace_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "do_replace");
    const done_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "repl_done");
    _ = c.LLVMBuildCondBr(self.builder, is_found, do_replace_bb, done_bb);

    // do_replace: build new string
    c.LLVMPositionBuilderAtEnd(self.builder, do_replace_bb);

    // Lengths
    var text_len_args = [_]c.LLVMValueRef{text};
    const text_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &text_len_args, 1, "text_len");
    var find_len_args = [_]c.LLVMValueRef{find};
    const find_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &find_len_args, 1, "find_len");
    var repl_len_args = [_]c.LLVMValueRef{replacement};
    const repl_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &repl_len_args, 1, "repl_len");

    // prefix_len = match_ptr - text
    const text_int = c.LLVMBuildPtrToInt(self.builder, text, i64_llvm, "text_int");
    const match_int = c.LLVMBuildPtrToInt(self.builder, match_ptr, i64_llvm, "match_int");
    const prefix_len = c.LLVMBuildSub(self.builder, match_int, text_int, "prefix_len");

    // suffix_len = text_len - prefix_len - find_len
    const prefix_plus_find = c.LLVMBuildAdd(self.builder, prefix_len, find_len, "pf");
    const suffix_len = c.LLVMBuildSub(self.builder, text_len, prefix_plus_find, "suffix_len");

    // new_len = prefix_len + repl_len + suffix_len
    const pr = c.LLVMBuildAdd(self.builder, prefix_len, repl_len, "pr");
    const new_len = c.LLVMBuildAdd(self.builder, pr, suffix_len, "new_len");
    const buf_size = c.LLVMBuildAdd(self.builder, new_len, one_i64, "buf_size");

    // malloc
    var malloc_args = [_]c.LLVMValueRef{buf_size};
    const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "repl_buf");

    // memcpy prefix
    var prefix_args = [_]c.LLVMValueRef{ buf, text, prefix_len };
    _ = c.LLVMBuildCall2(self.builder, memcpy_fn_type, self.memcpy_fn, &prefix_args, 3, "");

    // memcpy replacement
    var gep_repl_off = [_]c.LLVMValueRef{prefix_len};
    const repl_dst = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_repl_off, 1, "repl_dst");
    var repl_args = [_]c.LLVMValueRef{ repl_dst, replacement, repl_len };
    _ = c.LLVMBuildCall2(self.builder, memcpy_fn_type, self.memcpy_fn, &repl_args, 3, "");

    // memcpy suffix
    const suffix_off = c.LLVMBuildAdd(self.builder, prefix_len, repl_len, "suffix_off");
    var gep_suf_off = [_]c.LLVMValueRef{suffix_off};
    const suffix_dst = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_suf_off, 1, "suffix_dst");
    var gep_suf_src = [_]c.LLVMValueRef{prefix_plus_find};
    const suffix_src = c.LLVMBuildGEP2(self.builder, i8_type, text, &gep_suf_src, 1, "suffix_src");
    var suffix_args = [_]c.LLVMValueRef{ suffix_dst, suffix_src, suffix_len };
    _ = c.LLVMBuildCall2(self.builder, memcpy_fn_type, self.memcpy_fn, &suffix_args, 3, "");

    // Null-terminate
    var gep_null = [_]c.LLVMValueRef{new_len};
    const null_pos = c.LLVMBuildGEP2(self.builder, i8_type, buf, &gep_null, 1, "null_pos");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(i8_type, 0, 0), null_pos);

    _ = c.LLVMBuildBr(self.builder, done_bb);

    // done: phi between original text and replaced text
    c.LLVMPositionBuilderAtEnd(self.builder, done_bb);
    const entry_bb = c.LLVMGetPreviousBasicBlock(do_replace_bb);
    const result_phi = c.LLVMBuildPhi(self.builder, i8ptr_ty, "repl_result");
    c.LLVMAddIncoming(result_phi, @constCast(&[_]c.LLVMValueRef{ text, buf }), @constCast(&[_]c.LLVMBasicBlockRef{ entry_bb, do_replace_bb }), 2);

    return .{ .value = result_phi, .type_tag = .string_type };
}

fn genStrSplit(self: *CodeGen, text: c.LLVMValueRef, delimiter: c.LLVMValueRef) CodeGen.GenError!CodeGen.ExprResult {
    const i8_type = c.LLVMInt8TypeInContext(self.context);
    const i32_llvm = c.LLVMInt32TypeInContext(self.context);
    const i64_llvm = c.LLVMInt64TypeInContext(self.context);
    const i8ptr_ty = c.LLVMPointerType(i8_type, 0);
    const zero_i32 = c.LLVMConstInt(i32_llvm, 0, 0);
    const one_i32 = c.LLVMConstInt(i32_llvm, 1, 0);
    const one_i64 = c.LLVMConstInt(i64_llvm, 1, 0);

    const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
    const strstr_fn_type = c.LLVMGlobalGetValueType(self.strstr_fn);
    const memcpy_fn_type = c.LLVMGlobalGetValueType(self.memcpy_fn);
    const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);

    // Get delimiter length
    var delim_len_args = [_]c.LLVMValueRef{delimiter};
    const delim_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &delim_len_args, 1, "delim_len");

    const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));

    // Phase 1: Count segments by counting delimiter occurrences
    const count_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "split_count");
    const count_body = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "split_count_body");
    const count_done = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "split_count_done");

    _ = c.LLVMBuildBr(self.builder, count_header);
    c.LLVMPositionBuilderAtEnd(self.builder, count_header);
    const pos_phi = c.LLVMBuildPhi(self.builder, i8ptr_ty, "pos");
    const cnt_phi = c.LLVMBuildPhi(self.builder, i32_llvm, "cnt");

    // Find next delimiter
    var find_args = [_]c.LLVMValueRef{ pos_phi, delimiter };
    const found = c.LLVMBuildCall2(self.builder, strstr_fn_type, self.strstr_fn, &find_args, 2, "found");
    const null_ptr = c.LLVMConstNull(i8ptr_ty);
    const has_more = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, found, null_ptr, "has_more");
    _ = c.LLVMBuildCondBr(self.builder, has_more, count_body, count_done);

    c.LLVMPositionBuilderAtEnd(self.builder, count_body);
    const cnt_next = c.LLVMBuildAdd(self.builder, cnt_phi, one_i32, "cnt_next");
    // Advance past delimiter
    const delim_len_as_gep = c.LLVMBuildTrunc(self.builder, delim_len, i32_llvm, "dl_i32");
    _ = delim_len_as_gep;
    var gep_advance = [_]c.LLVMValueRef{delim_len};
    const next_pos = c.LLVMBuildGEP2(self.builder, i8_type, found, &gep_advance, 1, "next_pos");
    _ = c.LLVMBuildBr(self.builder, count_header);

    // Wire count phis
    const pre_count_bb = c.LLVMGetPreviousBasicBlock(count_header);
    c.LLVMAddIncoming(pos_phi, @constCast(&[_]c.LLVMValueRef{ text, next_pos }), @constCast(&[_]c.LLVMBasicBlockRef{ pre_count_bb, count_body }), 2);
    c.LLVMAddIncoming(cnt_phi, @constCast(&[_]c.LLVMValueRef{ zero_i32, cnt_next }), @constCast(&[_]c.LLVMBasicBlockRef{ pre_count_bb, count_body }), 2);

    c.LLVMPositionBuilderAtEnd(self.builder, count_done);
    // segments = count + 1
    const num_segments = c.LLVMBuildAdd(self.builder, cnt_phi, one_i32, "num_segments");

    // Phase 2: Allocate array of string pointers
    const ptr_size = c.LLVMConstInt(i64_llvm, 8, 0); // sizeof(i8*)
    const num_seg_i64 = c.LLVMBuildZExt(self.builder, num_segments, i64_llvm, "ns_i64");
    const arr_size = c.LLVMBuildMul(self.builder, num_seg_i64, ptr_size, "arr_size");
    var arr_malloc_args = [_]c.LLVMValueRef{arr_size};
    const arr_buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &arr_malloc_args, 1, "split_arr");
    const typed_arr = c.LLVMBuildBitCast(self.builder, arr_buf, c.LLVMPointerType(i8ptr_ty, 0), "typed_arr");

    // Phase 3: Extract segments
    const extract_header = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "split_extract");
    const extract_found = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "split_found");
    const extract_last = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "split_last");
    const extract_done = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "split_done");

    _ = c.LLVMBuildBr(self.builder, extract_header);
    c.LLVMPositionBuilderAtEnd(self.builder, extract_header);
    const cur_phi = c.LLVMBuildPhi(self.builder, i8ptr_ty, "cur");
    const idx_phi = c.LLVMBuildPhi(self.builder, i32_llvm, "idx");

    // Find next delimiter
    var find_args2 = [_]c.LLVMValueRef{ cur_phi, delimiter };
    const found2 = c.LLVMBuildCall2(self.builder, strstr_fn_type, self.strstr_fn, &find_args2, 2, "found2");
    const has_delim = c.LLVMBuildICmp(self.builder, c.LLVMIntNE, found2, null_ptr, "has_delim");
    _ = c.LLVMBuildCondBr(self.builder, has_delim, extract_found, extract_last);

    // extract_found: segment = cur..found
    c.LLVMPositionBuilderAtEnd(self.builder, extract_found);
    const cur_int = c.LLVMBuildPtrToInt(self.builder, cur_phi, i64_llvm, "cur_int");
    const found_int = c.LLVMBuildPtrToInt(self.builder, found2, i64_llvm, "found_int");
    const seg_len = c.LLVMBuildSub(self.builder, found_int, cur_int, "seg_len");
    const seg_buf_size = c.LLVMBuildAdd(self.builder, seg_len, one_i64, "seg_buf_sz");
    var seg_malloc_args = [_]c.LLVMValueRef{seg_buf_size};
    const seg_buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &seg_malloc_args, 1, "seg_buf");
    var seg_cpy_args = [_]c.LLVMValueRef{ seg_buf, cur_phi, seg_len };
    _ = c.LLVMBuildCall2(self.builder, memcpy_fn_type, self.memcpy_fn, &seg_cpy_args, 3, "");
    // Null-terminate
    var gep_seg_null = [_]c.LLVMValueRef{seg_len};
    const seg_null = c.LLVMBuildGEP2(self.builder, i8_type, seg_buf, &gep_seg_null, 1, "seg_null");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(i8_type, 0, 0), seg_null);
    // Store in array
    var gep_arr_idx = [_]c.LLVMValueRef{idx_phi};
    const arr_slot = c.LLVMBuildGEP2(self.builder, i8ptr_ty, typed_arr, &gep_arr_idx, 1, "arr_slot");
    _ = c.LLVMBuildStore(self.builder, seg_buf, arr_slot);
    // Advance
    var gep_past_delim = [_]c.LLVMValueRef{delim_len};
    const next_cur = c.LLVMBuildGEP2(self.builder, i8_type, found2, &gep_past_delim, 1, "next_cur");
    const idx_next = c.LLVMBuildAdd(self.builder, idx_phi, one_i32, "idx_next");
    _ = c.LLVMBuildBr(self.builder, extract_header);

    // Wire extract phis
    c.LLVMAddIncoming(cur_phi, @constCast(&[_]c.LLVMValueRef{ text, next_cur }), @constCast(&[_]c.LLVMBasicBlockRef{ count_done, extract_found }), 2);
    c.LLVMAddIncoming(idx_phi, @constCast(&[_]c.LLVMValueRef{ zero_i32, idx_next }), @constCast(&[_]c.LLVMBasicBlockRef{ count_done, extract_found }), 2);

    // extract_last: remaining string is last segment
    c.LLVMPositionBuilderAtEnd(self.builder, extract_last);
    var last_len_args = [_]c.LLVMValueRef{cur_phi};
    const last_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &last_len_args, 1, "last_len");
    const last_buf_size = c.LLVMBuildAdd(self.builder, last_len, one_i64, "last_buf_sz");
    var last_malloc_args = [_]c.LLVMValueRef{last_buf_size};
    const last_buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &last_malloc_args, 1, "last_buf");
    var last_cpy_args = [_]c.LLVMValueRef{ last_buf, cur_phi, last_len };
    _ = c.LLVMBuildCall2(self.builder, memcpy_fn_type, self.memcpy_fn, &last_cpy_args, 3, "");
    var gep_last_null = [_]c.LLVMValueRef{last_len};
    const last_null = c.LLVMBuildGEP2(self.builder, i8_type, last_buf, &gep_last_null, 1, "last_null");
    _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(i8_type, 0, 0), last_null);
    var gep_last_slot = [_]c.LLVMValueRef{idx_phi};
    const last_slot = c.LLVMBuildGEP2(self.builder, i8ptr_ty, typed_arr, &gep_last_slot, 1, "last_slot");
    _ = c.LLVMBuildStore(self.builder, last_buf, last_slot);
    _ = c.LLVMBuildBr(self.builder, extract_done);

    // Build slice struct
    c.LLVMPositionBuilderAtEnd(self.builder, extract_done);
    const slice_llvm = self.sliceLLVMType();
    const slice_alloca = c.LLVMBuildAlloca(self.builder, slice_llvm, "split_slice");
    const sl_ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 0, "split_ptr_f");
    _ = c.LLVMBuildStore(self.builder, arr_buf, sl_ptr_field);
    const sl_len_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 1, "split_len_f");
    _ = c.LLVMBuildStore(self.builder, num_segments, sl_len_field);
    const slice_val = c.LLVMBuildLoad2(self.builder, slice_llvm, slice_alloca, "split_result");

    const elem_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
    elem_ptr.* = .string_type;
    return .{ .value = slice_val, .type_tag = .{ .slice_type = .{ .element_type = elem_ptr } } };
}

pub fn genIoCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    const void_val = c.LLVMGetUndef(c.LLVMVoidTypeInContext(self.context));
    if (std.mem.eql(u8, func, "println")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "io.println() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        // Use printf with newline format string (same as log)
        const printf_fn_type = c.LLVMGlobalGetValueType(self.printf_fn);
        const fmt = switch (arg.type_tag) {
            .i32_type => self.fmt_int,
            .i64_type => self.fmt_int,
            .f64_type => self.fmt_float,
            .string_type => self.fmt_str,
            .char_type => self.fmt_char,
            .bool_type => return blk: {
                // Same pattern as genLogCall for bools
                const true_str = self.fmt_bool_true;
                const false_str = self.fmt_bool_false;
                const fmt_sel = c.LLVMBuildSelect(self.builder, arg.value, true_str, false_str, "bool_fmt");
                var print_args = [_]c.LLVMValueRef{fmt_sel};
                _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 1, "");
                break :blk .{ .value = void_val, .type_tag = .void_type };
            },
            else => { self.diagnostics.report(.@"error", 0, "io.println: unsupported type", .{}); return error.CodeGenError; },
        };
        var print_args = [_]c.LLVMValueRef{ fmt, arg.value };
        _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 2, "");
        return .{ .value = void_val, .type_tag = .void_type };
    } else if (std.mem.eql(u8, func, "print")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "io.print() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        // Use printf with no-newline format string
        const printf_fn_type = c.LLVMGlobalGetValueType(self.printf_fn);
        const fmt = switch (arg.type_tag) {
            .i32_type => self.fmt_int_raw,
            .i64_type => self.fmt_int_raw,
            .f64_type => self.fmt_float_raw,
            .string_type => self.fmt_str_raw,
            .char_type => self.fmt_char_raw,
            .bool_type => return blk: {
                const true_str = c.LLVMBuildGlobalStringPtr(self.builder, "true", "true_no_nl");
                const false_str = c.LLVMBuildGlobalStringPtr(self.builder, "false", "false_no_nl");
                const fmt_sel = c.LLVMBuildSelect(self.builder, arg.value, true_str, false_str, "bool_fmt");
                var print_args = [_]c.LLVMValueRef{fmt_sel};
                _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 1, "");
                break :blk .{ .value = void_val, .type_tag = .void_type };
            },
            else => { self.diagnostics.report(.@"error", 0, "io.print: unsupported type", .{}); return error.CodeGenError; },
        };
        var print_args = [_]c.LLVMValueRef{ fmt, arg.value };
        _ = c.LLVMBuildCall2(self.builder, printf_fn_type, self.printf_fn, &print_args, 2, "");
        return .{ .value = void_val, .type_tag = .void_type };
    } else if (std.mem.eql(u8, func, "read_line")) {
        if (args.items.len != 0) { self.diagnostics.report(.@"error", 0, "io.read_line() expects 0 arguments", .{}); return error.CodeGenError; }
        // malloc(4096)
        const buf_size = c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 4096, 0);
        const malloc_fn_type = c.LLVMGlobalGetValueType(self.malloc_fn);
        var malloc_args = [_]c.LLVMValueRef{buf_size};
        const buf = c.LLVMBuildCall2(self.builder, malloc_fn_type, self.malloc_fn, &malloc_args, 1, "read_buf");
        // Store empty string first (in case fgets fails)
        _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(c.LLVMInt8TypeInContext(self.context), 0, 0), buf);
        // Declare fgets: i8* fgets(i8*, i32, i8*)
        const i8ptr_type = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
        const fgets_params = [_]c.LLVMTypeRef{ i8ptr_type, c.LLVMInt32TypeInContext(self.context), i8ptr_type };
        const fgets_type = c.LLVMFunctionType(i8ptr_type, @constCast(&fgets_params), 3, 0);
        var fgets_fn = c.LLVMGetNamedFunction(self.module, "fgets");
        if (fgets_fn == null) {
            fgets_fn = c.LLVMAddFunction(self.module, "fgets", fgets_type);
        }
        // Get stdin - platform-specific (macOS: __stdinp)
        var stdin_global = c.LLVMGetNamedGlobal(self.module, "__stdinp");
        if (stdin_global == null) {
            stdin_global = c.LLVMAddGlobal(self.module, i8ptr_type, "__stdinp");
            c.LLVMSetExternallyInitialized(stdin_global, 1);
        }
        const stdin_val = c.LLVMBuildLoad2(self.builder, i8ptr_type, stdin_global, "stdin");
        var fgets_args = [_]c.LLVMValueRef{ buf, c.LLVMConstInt(c.LLVMInt32TypeInContext(self.context), 4096, 0), stdin_val };
        _ = c.LLVMBuildCall2(self.builder, fgets_type, fgets_fn, &fgets_args, 3, "");
        // Strip trailing newline
        const strlen_fn_type = c.LLVMGlobalGetValueType(self.strlen_fn);
        var strlen_args = [_]c.LLVMValueRef{buf};
        const read_len = c.LLVMBuildCall2(self.builder, strlen_fn_type, self.strlen_fn, &strlen_args, 1, "read_len");
        const has_content = c.LLVMBuildICmp(self.builder, c.LLVMIntUGT, read_len, c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 0, 0), "has_content");
        const current_fn = c.LLVMGetBasicBlockParent(c.LLVMGetInsertBlock(self.builder));
        const strip_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "strip_nl");
        const done_bb = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "read_done");
        _ = c.LLVMBuildCondBr(self.builder, has_content, strip_bb, done_bb);
        c.LLVMPositionBuilderAtEnd(self.builder, strip_bb);
        const last_idx = c.LLVMBuildSub(self.builder, read_len, c.LLVMConstInt(c.LLVMInt64TypeInContext(self.context), 1, 0), "last_idx");
        var gep_last = [_]c.LLVMValueRef{last_idx};
        const last_ptr = c.LLVMBuildGEP2(self.builder, c.LLVMInt8TypeInContext(self.context), buf, &gep_last, 1, "last_ptr");
        const last_ch = c.LLVMBuildLoad2(self.builder, c.LLVMInt8TypeInContext(self.context), last_ptr, "last_ch");
        const is_nl = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, last_ch, c.LLVMConstInt(c.LLVMInt8TypeInContext(self.context), 10, 0), "is_nl");
        const strip_bb2 = c.LLVMAppendBasicBlockInContext(self.context, current_fn, "do_strip");
        _ = c.LLVMBuildCondBr(self.builder, is_nl, strip_bb2, done_bb);
        c.LLVMPositionBuilderAtEnd(self.builder, strip_bb2);
        _ = c.LLVMBuildStore(self.builder, c.LLVMConstInt(c.LLVMInt8TypeInContext(self.context), 0, 0), last_ptr);
        _ = c.LLVMBuildBr(self.builder, done_bb);
        c.LLVMPositionBuilderAtEnd(self.builder, done_bb);
        return .{ .value = buf, .type_tag = .string_type };
    }
    self.diagnostics.report(.@"error", 0, "unknown io function '{s}'", .{func});
    return error.CodeGenError;
}

pub fn genOsCall(self: *CodeGen, func: []const u8, args: *const std.ArrayList(Ast.ExprIndex)) CodeGen.GenError!CodeGen.ExprResult {
    if (std.mem.eql(u8, func, "getenv")) {
        if (args.items.len != 1) { self.diagnostics.report(.@"error", 0, "os.getenv() expects 1 argument", .{}); return error.CodeGenError; }
        const arg = try codegen_exprs.genExpr(self, args.items[0]);
        const fn_type = c.LLVMGlobalGetValueType(self.getenv_fn);
        var call_args = [_]c.LLVMValueRef{arg.value};
        const result = c.LLVMBuildCall2(self.builder, fn_type, self.getenv_fn, &call_args, 1, "getenv_res");
        // If null, return empty string
        const null_ptr = c.LLVMConstNull(c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0));
        const is_null = c.LLVMBuildICmp(self.builder, c.LLVMIntEQ, result, null_ptr, "is_null");
        const empty_str = c.LLVMBuildGlobalStringPtr(self.builder, "", "empty_str");
        const final_val = c.LLVMBuildSelect(self.builder, is_null, empty_str, result, "env_val");
        return .{ .value = final_val, .type_tag = .string_type };
    } else if (std.mem.eql(u8, func, "clock")) {
        if (args.items.len != 0) { self.diagnostics.report(.@"error", 0, "os.clock() expects 0 arguments", .{}); return error.CodeGenError; }
        const fn_type = c.LLVMGlobalGetValueType(self.clock_fn);
        const clock_val = c.LLVMBuildCall2(self.builder, fn_type, self.clock_fn, null, 0, "clock_res");
        // Convert to f64 and divide by CLOCKS_PER_SEC (1000000 on POSIX)
        const f64_val = c.LLVMBuildSIToFP(self.builder, clock_val, c.LLVMDoubleTypeInContext(self.context), "clock_f64");
        const divisor = c.LLVMConstReal(c.LLVMDoubleTypeInContext(self.context), 1000000.0);
        const seconds = c.LLVMBuildFDiv(self.builder, f64_val, divisor, "seconds");
        return .{ .value = seconds, .type_tag = .f64_type };
    } else if (std.mem.eql(u8, func, "args")) {
        if (args.items.len != 0) { self.diagnostics.report(.@"error", 0, "os.args() expects 0 arguments", .{}); return error.CodeGenError; }
        const i8ptr_ty = c.LLVMPointerType(c.LLVMInt8TypeInContext(self.context), 0);
        const i8ptr_ptr_ty = c.LLVMPointerType(i8ptr_ty, 0);
        // Load argc and argv from globals
        const argc = c.LLVMBuildLoad2(self.builder, c.LLVMInt32TypeInContext(self.context), self.argc_global, "argc");
        const argv = c.LLVMBuildLoad2(self.builder, i8ptr_ptr_ty, self.argv_global, "argv");
        // Build slice struct { ptr, len }
        const slice_llvm = self.sliceLLVMType();
        const slice_alloca = c.LLVMBuildAlloca(self.builder, slice_llvm, "args_slice");
        const ptr_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 0, "args_ptr_f");
        _ = c.LLVMBuildStore(self.builder, c.LLVMBuildBitCast(self.builder, argv, i8ptr_ty, "argv_cast"), ptr_field);
        const len_field = c.LLVMBuildStructGEP2(self.builder, slice_llvm, slice_alloca, 1, "args_len_f");
        _ = c.LLVMBuildStore(self.builder, argc, len_field);
        const slice_val = c.LLVMBuildLoad2(self.builder, slice_llvm, slice_alloca, "args_val");
        const elem_ptr = self.allocator.create(CodeGen.TypeTag) catch return error.CodeGenError;
        elem_ptr.* = .string_type;
        return .{ .value = slice_val, .type_tag = .{ .slice_type = .{ .element_type = elem_ptr } } };
    }
    self.diagnostics.report(.@"error", 0, "unknown os function '{s}'", .{func});
    return error.CodeGenError;
}
