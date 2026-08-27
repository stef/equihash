const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const static_lib = b.addLibrary(.{
        .name = "libequihash",
        .linkage = .static,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
            .link_libcpp = true,
        }),
    });

    const libsodium_package = b.dependency("libsodium", .{
        .target = target,
        .optimize = optimize,
        .@"test" = false, // `test` is a keyword in zig
        .static = true,
        .shared = false
    });
    static_lib.root_module.linkLibrary(libsodium_package.artifact("sodium"));
    static_lib.root_module.addIncludePath(libsodium_package.path("include"));

    b.installArtifact(static_lib);

    const flags = &.{
        "-fvisibility=hidden",
        "-fPIC",
        "-fwrapv",
        "-std=c++17",
    };

    static_lib.root_module.addCSourceFile(.{ .file = b.path("equihash.cc"), .flags = flags, });
    static_lib.installHeader(b.path("equihash.h"), "equihash/equihash.h");
    static_lib.installHeader(b.path("equihash.hpp"), "equihash/equihash.hpp");
}
