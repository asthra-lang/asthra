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
        } else if (std.mem.eql(u8, args[i], "-g") or std.mem.eql(u8, args[i], "--debug")) {
            // Debug info emission planned for future release
            writeAll("note: -g flag recognized; DWARF debug info not yet implemented\n", .{});
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

    // Resolve imports: parse imported files into separate ASTs
    var imported_modules = std.ArrayList(ImportedModule){};
    defer imported_modules.deinit(allocator);

    resolveImports(allocator, ast_allocator, &ast, &diagnostics, source_path, &imported_modules) catch |err| {
        writeAll("error: import resolution failed: {}\n", .{err});
        std.process.exit(1);
    };

    if (diagnostics.hasErrors()) {
        diagnostics.printAll();
        std.process.exit(1);
    }

    // Codegen
    var codegen = CodeGen.init(allocator, "asthra_module", &diagnostics, &ast);
    defer codegen.deinit();

    // Generate code for imported modules first
    for (imported_modules.items) |mod| {
        codegen.generateImportedModule(mod.ast) catch {
            diagnostics.printAll();
            std.process.exit(1);
        };
    }

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

const ImportedModule = struct {
    alias: []const u8,
    ast: *Ast,
    source: []const u8,
};

fn resolveImports(gpa: std.mem.Allocator, ast_allocator: std.mem.Allocator, ast: *Ast, diagnostics: *Diagnostics, source_path: []const u8, imported_modules: *std.ArrayList(ImportedModule)) !void {
    // Get the directory of the source file
    const source_dir = std.fs.path.dirname(source_path) orelse ".";

    for (ast.program.imports.items) |import_decl| {
        const path = import_decl.path;

        // Only handle relative imports (starting with ./)
        if (!std.mem.startsWith(u8, path, "./")) {
            continue;
        }

        // Build the full file path: source_dir + relative_path + .ast
        const import_path = try std.fmt.allocPrint(gpa, "{s}/{s}.ast", .{ source_dir, path });
        defer gpa.free(import_path);

        // Read the imported file — keep source alive for the AST (arena allocated)
        const import_source = std.fs.cwd().readFileAlloc(ast_allocator, import_path, 1024 * 1024) catch |err| {
            diagnostics.report(.@"error", 0, "cannot read imported file '{s}': {}", .{ import_path, err });
            return;
        };

        // Parse the imported file
        var import_diagnostics = Diagnostics.init(gpa, import_source, import_path);
        defer import_diagnostics.deinit();

        const import_ast = try ast_allocator.create(Ast);
        import_ast.* = Ast.init(ast_allocator);
        var import_parser = Parser.init(import_source, import_ast, &import_diagnostics);
        import_parser.parse() catch {};

        if (import_diagnostics.hasErrors()) {
            import_diagnostics.printAll();
            diagnostics.report(.@"error", 0, "errors in imported file '{s}'", .{import_path});
            return;
        }

        // Register the import alias
        const alias = import_decl.alias orelse std.fs.path.stem(path[2..]); // strip "./" prefix for stem
        try ast.program.import_aliases.append(ast_allocator, alias);

        try imported_modules.append(gpa, .{
            .alias = alias,
            .ast = import_ast,
            .source = import_source,
        });
    }
}
