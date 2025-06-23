import os
import sys
import platform

def get_name():
    return 'sbc'

def can_build():
    if os.name != "posix":
        return False
    return True

def get_opts():
    from SCons.Variables import BoolVariable, EnumVariable

    return [
        BoolVariable("use_static_cpp", "Link libgcc and libstdc++ statically for better portability", True),
        BoolVariable("use_llvm", "Use the LLVM compiler", False),
    ]

def get_flags():
    return []

def configure(env):
    env.Prepend(CPPPATH=["#platform/sbc"])
    env.Append(CPPDEFINES=["PLATFORM_SBC", "UNIX_ENABLED", "LINUX_ENABLED"])
    env.Append(CPPDEFINES=[('_FILE_OFFSET_BITS', 64)])
    env.Append(CPPFLAGS=['-DUNIX_ENABLED'])
    env.Append(CXXFLAGS=["-std=c++17"])

    arch = env.get("arch", "x86_64")
    is_cross = False

    if platform.machine() != "aarch64" and arch == "arm64":
        is_cross = True

    print(f"==> Compiling for architecture: {arch} ({'cross-compiling' if is_cross else 'native'})")

    if arch == "arm64":
        env.Append(CPPDEFINES=["ARM64_ENABLED"])
        #env.extra_suffix = ".arm64"

        if is_cross:
            print("🔧 Setuping cross-compilation environment for ARM64 with LLVM...")
            sysroot_path = "/srv/chroot/ubuntu-arm64"
            target_triplet = "aarch64-linux-gnu"

            if env.get("use_llvm", False):
                env["CC"] = "clang"
                env["CXX"] = "clang++"
                env["LD"] = "clang++"
                env["LINK"] = "clang++"
            else:
                env["CC"] = f"{target_triplet}-gcc"
                env["CXX"] = f"{target_triplet}-g++"
                env["AR"] = f"{target_triplet}-ar"
                env["RANLIB"] = f"{target_triplet}-ranlib"
                env["STRIP"] = f"{target_triplet}-strip"
                env["AS"] = f"{target_triplet}-as"
                env["LD"] = f"{target_triplet}-ld"
                env["LINK"] = f"{target_triplet}-g++"

            target_flags = [
                f"--target={target_triplet}",
                f"--sysroot={sysroot_path}",
            ]
            env.Append(CCFLAGS=target_flags)
            env.Append(CXXFLAGS=target_flags)
            env.Append(LINKFLAGS=target_flags)

            env.Append(LINKFLAGS=[
                "-B", os.path.join(sysroot_path, "usr/lib/gcc/aarch64-linux-gnu"),
                "-L" + os.path.join(sysroot_path, "usr/lib/aarch64-linux-gnu"),
                "-L" + os.path.join(sysroot_path, "lib/aarch64-linux-gnu"),
                "-L" + os.path.join(sysroot_path, "lib"),
            ])

            crt_path = os.path.join(sysroot_path, "usr/lib/aarch64-linux-gnu")
            if os.path.isdir(crt_path):
                env.Prepend(LINKFLAGS=["-L" + crt_path])

            env.Prepend(CPPPATH=[
                os.path.join(sysroot_path, "usr/include"),
                os.path.join(sysroot_path, "usr/include/aarch64-linux-gnu")
            ])

            env.Prepend(LIBPATH=[
                os.path.join(sysroot_path, "lib/aarch64-linux-gnu"),
                os.path.join(sysroot_path, "usr/lib/aarch64-linux-gnu")
            ])

            os.environ["PKG_CONFIG_PATH"] = f"{sysroot_path}/usr/lib/aarch64-linux-gnu/pkgconfig:{sysroot_path}/usr/share/pkgconfig"
            env["PKG_CONFIG_PATH"] = os.environ["PKG_CONFIG_PATH"]

        else:
            env.Prepend(CPPPATH=["/usr/include", "/usr/include/aarch64-linux-gnu"])
            env.Prepend(LIBPATH=["/lib/aarch64-linux-gnu", "/usr/lib/aarch64-linux-gnu"])

    elif arch == "x86_64":
        env.Append(CPPDEFINES=["X86_64_ENABLED"])
       # env.extra_suffix = ".x86_64" + env.extra_suffix

    else:
        print(f"ERROR: Unsupported architecture: {arch}")
        sys.exit(1)

    sdl2_path = os.environ.get("SDL2_ARM64_PATH") if arch == "arm64" else None
    if sdl2_path:
        print(f"✅ Using custom SDL2 path: {sdl2_path}")
        env.Prepend(CPPPATH=[os.path.join(sdl2_path, "include")])
        env.Prepend(LIBPATH=[os.path.join(sdl2_path, "lib")])
        env.Append(LIBS=["SDL2"])
    else:
        if os.system("pkg-config --exists sdl2") != 0:
            print("❌ SDL2 not found. Set SDL2_ARM64_PATH or install libsdl2-dev.")
            sys.exit(1)
        else:
            print("✅ SDL2 header found.")
            env.ParseConfig("pkg-config --cflags --libs sdl2")

    env.Append(LIBS=["pthread", "dl", "z"])

    if env.get("vulkan"):
        env.Append(CPPDEFINES=["VULKAN_ENABLED", "RD_ENABLED"])
        if not env.get("builtin_glslang"):
            env.Append(LIBS=["glslang", "SPIRV"])

    if env.get("opengl3"):
        env.Append(CPPDEFINES=["GLES3_ENABLED"])

    if env.get("use_llvm", False):
        if "clang++" not in os.path.basename(env.get("CXX", "")):
            env["CC"] = "clang"
            env["CXX"] = "clang++"
            env["LD"] = "clang++"
        env.extra_suffix = ".llvm" + env.extra_suffix

    if env.get("use_static_cpp", True):
        env.Append(LINKFLAGS=["-static-libgcc", "-static-libstdc++"])

    if env.get("ZSTD_DISABLE_ASM", False):
        env.Append(CPPDEFINES=["ZSTD_DISABLE_ASM"])
