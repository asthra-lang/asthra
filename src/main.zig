const std = @import("std");
const Lexer = @import("lexer.zig").Lexer;
const Ast = @import("ast.zig").Ast;
const Parser = @import("parser.zig").Parser;
const CodeGen = @import("codegen.zig").CodeGen;
const Diagnostics = @import("diagnostics.zig").Diagnostics;

fn writeAll(comptime fmt: []const u8, args: anytype) void {
    const file = std.fs.File.stderr();
    const w = file.deprecatedWriter();
    w.print(fmt, args) catch {};
}

fn writeOut(comptime fmt: []const u8, args: anytype) void {
    const file = std.fs.File.stdout();
    const w = file.deprecatedWriter();
    w.print(fmt, args) catch {};
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // Parse CLI args
    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if (args.len < 2) {
        writeAll("Usage: asthra <source.ast> [-o output] [--emit-ir]\n", .{});
        std.process.exit(1);
    }

    const source_path = args[1];
    var output_path: []const u8 = "output";
    var emit_ir = false;

    var i: usize = 2;
    while (i < args.len) : (i += 1) {
        if (std.mem.eql(u8, args[i], "-o") and i + 1 < args.len) {
            i += 1;
            output_path = args[i];
        } else if (std.mem.eql(u8, args[i], "--emit-ir")) {
            emit_ir = true;
        }
    }

    // Read source file
    const source = std.fs.cwd().readFileAlloc(allocator, source_path, 1024 * 1024) catch |err| {
        writeAll("error: cannot read '{s}': {}\n", .{ source_path, err });
        std.process.exit(1);
    };
    defer allocator.free(source);

    // Initialize diagnostics
    var diagnostics = Diagnostics.init(allocator, source, source_path);
    defer diagnostics.deinit();

    // Parse — use arena for AST allocations (freed all at once)
    var arena = std.heap.ArenaAllocator.init(allocator);
    defer arena.deinit();
    const ast_allocator = arena.allocator();

    var ast = Ast.init(ast_allocator);
    var parser = Parser.init(source, &ast, &diagnostics);
    parser.parse() catch {};

    if (diagnostics.hasErrors()) {
        diagnostics.printAll();
        std.process.exit(1);
    }

    // Codegen
    var codegen = CodeGen.init(allocator, "asthra_module", &diagnostics, &ast);
    defer codegen.deinit();

    codegen.generate() catch {
        diagnostics.printAll();
        std.process.exit(1);
    };

    if (emit_ir) {
        codegen.printIR();
    }

    // Emit object file
    const obj_path = try std.fmt.allocPrint(allocator, "{s}.o", .{output_path});
    defer allocator.free(obj_path);

    codegen.emitObjectFile(obj_path) catch {
        diagnostics.printAll();
        std.process.exit(1);
    };

    // Link with cc
    const result = std.process.Child.run(.{
        .allocator = allocator,
        .argv = &.{ "cc", obj_path, "-o", output_path },
    }) catch |err| {
        writeAll("error: linking failed: {}\n", .{err});
        std.process.exit(1);
    };
    defer allocator.free(result.stdout);
    defer allocator.free(result.stderr);

    if (result.term.Exited != 0) {
        writeAll("error: linker failed:\n{s}\n", .{result.stderr});
        std.process.exit(1);
    }

    // Clean up object file
    std.fs.cwd().deleteFile(obj_path) catch {};

    writeOut("Compiled successfully: {s}\n", .{output_path});
}
