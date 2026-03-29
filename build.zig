const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const llvm_prefix = "/opt/homebrew/opt/llvm@20";
    const llvm_include: std.Build.LazyPath = .{ .cwd_relative = llvm_prefix ++ "/include" };
    const llvm_lib: std.Build.LazyPath = .{ .cwd_relative = llvm_prefix ++ "/lib" };

    // Main executable
    const exe_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    exe_mod.addIncludePath(llvm_include);
    exe_mod.addLibraryPath(llvm_lib);
    exe_mod.linkSystemLibrary("LLVM-20", .{});

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
    test_mod.linkSystemLibrary("LLVM-20", .{});

    const unit_tests = b.addTest(.{
        .root_module = test_mod,
    });

    const run_unit_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_unit_tests.step);
}
