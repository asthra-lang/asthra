const std = @import("std");

/// Get the cache directory path for a package at a specific commit.
/// Returns: ~/.asthra/cache/{git_url}/{commit_prefix}/
pub fn getCachePath(allocator: std.mem.Allocator, git_url: []const u8, commit: []const u8) ![]const u8 {
    const home = std.posix.getenv("HOME") orelse "/tmp";
    return std.fmt.allocPrint(allocator, "{s}/.asthra/cache/{s}/{s}", .{ home, git_url, commit });
}

/// Check if a package is already cached.
pub fn isCached(allocator: std.mem.Allocator, git_url: []const u8, commit: []const u8) !bool {
    const cache_path = try getCachePath(allocator, git_url, commit);
    defer allocator.free(cache_path);

    const entry_path = try std.fmt.allocPrint(allocator, "{s}/src/lib.ast", .{cache_path});
    defer allocator.free(entry_path);

    std.fs.cwd().access(entry_path, .{}) catch return false;
    return true;
}

/// Resolve a git tag to a commit SHA using `git ls-remote`.
pub fn resolveTagToCommit(allocator: std.mem.Allocator, git_url: []const u8, tag: []const u8) ![]const u8 {
    const https_url = try std.fmt.allocPrint(allocator, "https://{s}", .{git_url});
    defer allocator.free(https_url);

    const ref = try std.fmt.allocPrint(allocator, "refs/tags/{s}", .{tag});
    defer allocator.free(ref);

    const result = std.process.Child.run(.{
        .allocator = allocator,
        .argv = &.{ "git", "ls-remote", https_url, ref },
    }) catch |err| {
        std.debug.print("error: git ls-remote failed: {}\n", .{err});
        return error.GitError;
    };
    defer allocator.free(result.stdout);
    defer allocator.free(result.stderr);

    if (result.term.Exited != 0) {
        std.debug.print("error: git ls-remote failed for {s}\n{s}\n", .{ git_url, result.stderr });
        return error.GitError;
    }

    // Output format: "<sha>\trefs/tags/v1.0.0\n"
    if (std.mem.indexOf(u8, result.stdout, "\t")) |tab_pos| {
        if (tab_pos >= 7) {
            return allocator.dupe(u8, result.stdout[0..tab_pos]);
        }
    }

    std.debug.print("error: tag '{s}' not found in {s}\n", .{ tag, git_url });
    return error.TagNotFound;
}

/// Fetch a package into the global cache.
/// Clones the repo and checks out the specified commit.
pub fn fetchPackage(allocator: std.mem.Allocator, git_url: []const u8, commit: []const u8) ![]const u8 {
    const cache_path = try getCachePath(allocator, git_url, commit);

    // Check if already cached
    const entry_check = try std.fmt.allocPrint(allocator, "{s}/src/lib.ast", .{cache_path});
    defer allocator.free(entry_check);
    if (std.fs.cwd().access(entry_check, .{})) |_| {
        return cache_path;
    } else |_| {}

    // Create cache directory
    std.fs.cwd().makePath(cache_path) catch {};

    const https_url = try std.fmt.allocPrint(allocator, "https://{s}", .{git_url});
    defer allocator.free(https_url);

    // Clone
    const clone_result = std.process.Child.run(.{
        .allocator = allocator,
        .argv = &.{ "git", "clone", "--depth=1", https_url, cache_path },
    }) catch |err| {
        std.debug.print("error: git clone failed: {}\n", .{err});
        return error.GitError;
    };
    defer allocator.free(clone_result.stdout);
    defer allocator.free(clone_result.stderr);

    if (clone_result.term.Exited != 0) {
        std.debug.print("error: git clone failed:\n{s}\n", .{clone_result.stderr});
        return error.GitError;
    }

    // Fetch the specific commit (may not be in shallow clone)
    const fetch_result = std.process.Child.run(.{
        .allocator = allocator,
        .argv = &.{ "git", "-C", cache_path, "fetch", "--depth=1", "origin", commit },
    }) catch |err| {
        std.debug.print("error: git fetch failed: {}\n", .{err});
        return error.GitError;
    };
    defer allocator.free(fetch_result.stdout);
    defer allocator.free(fetch_result.stderr);

    // Checkout the commit
    const checkout_result = std.process.Child.run(.{
        .allocator = allocator,
        .argv = &.{ "git", "-C", cache_path, "checkout", commit },
    }) catch |err| {
        std.debug.print("error: git checkout failed: {}\n", .{err});
        return error.GitError;
    };
    defer allocator.free(checkout_result.stdout);
    defer allocator.free(checkout_result.stderr);

    if (checkout_result.term.Exited != 0) {
        std.debug.print("error: git checkout failed:\n{s}\n", .{checkout_result.stderr});
        return error.GitError;
    }

    return cache_path;
}

pub const FetcherError = error{
    GitError,
    TagNotFound,
    OutOfMemory,
};
