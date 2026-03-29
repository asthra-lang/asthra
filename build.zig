const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // LLVM paths — configurable via -Dllvm-prefix or environment, with macOS Homebrew default
    const default_llvm_prefix = "/opt/homebrew/opt/llvm@20";
    const llvm_prefix = b.option([]const u8, "llvm-prefix", "LLVM installation prefix") orelse default_llvm_prefix;
    const llvm_include: std.Build.LazyPath = .{ .cwd_relative = b.fmt("{s}/include", .{llvm_prefix}) };
    const llvm_lib: std.Build.LazyPath = .{ .cwd_relative = b.fmt("{s}/lib", .{llvm_prefix}) };

    // Detect LLVM library name (LLVM-20, LLVM-18, etc.)
    const llvm_lib_name = b.option([]const u8, "llvm-lib", "LLVM library name (e.g. LLVM-20)") orelse "LLVM-20";

    // Main executable
    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    exe_mod.addIncludePath(llvm_include);
    exe_mod.addLibraryPath(llvm_lib);
    exe_mod.linkSystemLibrary(llvm_lib_name, .{});

    const exe = b.addExecutable(.{
        .name = "asthra",
        .root_module = exe_mod,
    });

    b.installArtifact(exe);

    // Run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |run_args| {
        run_cmd.addArgs(run_args);
    }
    const run_step = b.step("run", "Run the compiler");
    run_step.dependOn(&run_cmd.step);

    // Tests
    const test_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    test_mod.addIncludePath(llvm_include);
    test_mod.addLibraryPath(llvm_lib);
    test_mod.linkSystemLibrary(llvm_lib_name, .{});

    const unit_tests = b.addTest(.{
        .root_module = test_mod,
    });

    const run_unit_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_unit_tests.step);

    // Package management tests (no LLVM dependency)
    const pkg_tests = b.addTest(.{
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/package.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });
    const run_pkg_tests = b.addRunArtifact(pkg_tests);
    test_step.dependOn(&run_pkg_tests.step);
}
