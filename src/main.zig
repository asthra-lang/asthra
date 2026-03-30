const std = @import("std");
const Lexer = @import("lexer.zig").Lexer;
const Ast = @import("ast.zig").Ast;
const Parser = @import("parser.zig").Parser;
const CodeGen = @import("codegen.zig").CodeGen;
const Diagnostics = @import("diagnostics.zig").Diagnostics;
const SemanticAnalyzer = @import("sema.zig").SemanticAnalyzer;
const commands = @import("commands.zig");
const pkg = @import("package.zig");
const fetcher = @import("fetcher.zig");

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
        writeAll("       asthra init          Create asthra.toml\n", .{});
        writeAll("       asthra add <pkg>     Add a dependency\n", .{});
        writeAll("       asthra install       Fetch all dependencies\n", .{});
        std.process.exit(1);
    }

    // Subcommand routing
    const first_arg = args[1];
    if (std.mem.eql(u8, first_arg, "init")) {
        commands.init(allocator);
        return;
    } else if (std.mem.eql(u8, first_arg, "add")) {
        commands.add(allocator, args[2..]);
        return;
    } else if (std.mem.eql(u8, first_arg, "install")) {
        commands.install(allocator);
        return;
    }

    // Legacy/default: compile a source file
    const source_path = first_arg;
    var output_path: []const u8 = "output";
    var emit_ir = false;
    var debug_enabled = false;

    var i: usize = 2;
    while (i < args.len) : (i += 1) {
        if (std.mem.eql(u8, args[i], "-o") and i + 1 < args.len) {
            i += 1;
            output_path = args[i];
        } else if (std.mem.eql(u8, args[i], "--emit-ir")) {
            emit_ir = true;
        } else if (std.mem.eql(u8, args[i], "-g") or std.mem.eql(u8, args[i], "--debug")) {
            debug_enabled = true;
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

    // Read package manifest and lockfile (if they exist)
    var manifest: ?pkg.PackageManifest = pkg.PackageManifest.parseFromFile(allocator, "asthra.toml") catch null;
    defer if (manifest) |*m| m.deinit();

    var lockfile: ?pkg.Lockfile = (pkg.Lockfile.parseFromFile(allocator, "asthra.lock") catch null) orelse null;
    defer if (lockfile) |*lf| lf.deinit();

    // Resolve imports: parse imported files into separate ASTs
    var imported_modules = std.ArrayList(ImportedModule){};
    defer imported_modules.deinit(allocator);

    resolveImports(allocator, ast_allocator, &ast, &diagnostics, source_path, &imported_modules, manifest, lockfile) catch |err| {
        writeAll("error: import resolution failed: {}\n", .{err});
        std.process.exit(1);
    };

    if (diagnostics.hasErrors()) {
        diagnostics.printAll();
        std.process.exit(1);
    }

    // Semantic analysis
    var sema = SemanticAnalyzer.init(allocator, &ast, &diagnostics);
    defer sema.deinit();
    // Register public declarations from imported modules
    for (imported_modules.items) |mod| {
        for (mod.ast.program.decls.items) |decl| {
            if (decl.visibility != .public) continue;
            switch (decl.decl) {
                .struct_decl => |s| { sema.struct_names.put(s.name, {}) catch {}; },
                .enum_decl => |e| { sema.enum_names.put(e.name, {}) catch {}; },
                .const_decl => |cn| { sema.const_names.put(cn.name, {}) catch {}; },
                .function => |f| {
                    sema.functions.put(f.name, .{ .param_count = @intCast(f.params.items.len) }) catch {};
                },
                else => {},
            }
        }
    }
    sema.analyze();

    if (diagnostics.hasErrors()) {
        diagnostics.printAll();
        std.process.exit(1);
    }

    // Codegen
    var codegen = CodeGen.init(allocator, "asthra_module", &diagnostics, &ast);
    defer codegen.deinit();
    if (debug_enabled) {
        codegen.initDebug(source, source_path);
    }

    // Generate code for imported modules first
    for (imported_modules.items) |mod| {
        codegen.generateImportedModule(mod.ast, mod.package_name) catch {
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
    const link_argv: []const []const u8 = if (debug_enabled)
        &.{ "cc", obj_path, "-o", output_path, "-lm", "-g" }
    else
        &.{ "cc", obj_path, "-o", output_path, "-lm" };
    const result = std.process.Child.run(.{
        .allocator = allocator,
        .argv = link_argv,
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
    package_name: []const u8,
    ast: *Ast,
    source: []const u8,
};

fn resolveImports(gpa: std.mem.Allocator, ast_allocator: std.mem.Allocator, ast: *Ast, diagnostics: *Diagnostics, source_path: []const u8, imported_modules: *std.ArrayList(ImportedModule), manifest: ?pkg.PackageManifest, lockfile: ?pkg.Lockfile) !void {
    // Get the directory of the source file
    const source_dir = std.fs.path.dirname(source_path) orelse ".";

    for (ast.program.imports.items) |import_decl| {
        const path = import_decl.path;

        if (std.mem.startsWith(u8, path, "./")) {
            // Relative import
            const import_path = try std.fmt.allocPrint(gpa, "{s}/{s}.ast", .{ source_dir, path });
            defer gpa.free(import_path);
            const alias = import_decl.alias orelse std.fs.path.stem(path[2..]);
            try resolveAndRegisterImport(gpa, ast_allocator, ast, diagnostics, import_path, alias, imported_modules);
        } else if (std.mem.startsWith(u8, path, "github.com/") or std.mem.startsWith(u8, path, "gitlab.com/")) {
            // Remote package import
            const mfst = manifest orelse {
                diagnostics.report(.@"error", 0, "no asthra.toml found; cannot resolve '{s}'", .{path});
                return;
            };
            _ = mfst.findByGitUrl(path) orelse {
                diagnostics.report(.@"error", 0, "'{s}' not in asthra.toml [dependencies]", .{path});
                return;
            };
            const lf = lockfile orelse {
                diagnostics.report(.@"error", 0, "no asthra.lock found; run 'asthra install'", .{});
                return;
            };
            const resolved = lf.findByGitUrl(path) orelse {
                diagnostics.report(.@"error", 0, "'{s}' not in lockfile; run 'asthra install'", .{path});
                return;
            };
            const cache_path = try fetcher.getCachePath(gpa, path, resolved.commit);
            defer gpa.free(cache_path);
            const entry_file = try std.fmt.allocPrint(gpa, "{s}/src/lib.ast", .{cache_path});
            defer gpa.free(entry_file);
            const alias = import_decl.alias orelse commands.extractRepoName(path);
            try resolveAndRegisterImport(gpa, ast_allocator, ast, diagnostics, entry_file, alias, imported_modules);
        } else if (std.mem.startsWith(u8, path, "std/")) {
            // Stdlib: handled by codegen namespace system, skip
            continue;
        } else {
            diagnostics.report(.@"error", 0, "unknown import path format: '{s}'", .{path});
        }
    }
}

fn resolveAndRegisterImport(gpa: std.mem.Allocator, ast_allocator: std.mem.Allocator, ast: *Ast, diagnostics: *Diagnostics, file_path: []const u8, alias: []const u8, imported_modules: *std.ArrayList(ImportedModule)) !void {
    const import_source = std.fs.cwd().readFileAlloc(ast_allocator, file_path, 1024 * 1024) catch |err| {
        diagnostics.report(.@"error", 0, "cannot read imported file '{s}': {}", .{ file_path, err });
        return;
    };

    var import_diagnostics = Diagnostics.init(gpa, import_source, file_path);
    defer import_diagnostics.deinit();

    const import_ast = try ast_allocator.create(Ast);
    import_ast.* = Ast.init(ast_allocator);
    var import_parser = Parser.init(import_source, import_ast, &import_diagnostics);
    import_parser.parse() catch {};

    if (import_diagnostics.hasErrors()) {
        import_diagnostics.printAll();
        diagnostics.report(.@"error", 0, "errors in imported file '{s}'", .{file_path});
        return;
    }

    const package_name = import_ast.program.package_name;
    try ast.program.import_aliases.put(alias, package_name);

    try imported_modules.append(gpa, .{
        .alias = alias,
        .package_name = package_name,
        .ast = import_ast,
        .source = import_source,
    });
}
