-- WickedEngine xmake build
-- Windows-only for now (DX12 + Vulkan backends)

set_project("WickedEngine")
set_version("0.0.0")
set_languages("c++17")
set_runtimes("MT")

-- ─────────────────────────────────────────────
-- LUA (static lib)
-- ─────────────────────────────────────────────
target("LUA")
    set_kind("static")
    set_group("ThirdParty")
    add_files("WickedEngine/LUA/*.c")
    remove_files("WickedEngine/LUA/lua.c") -- standalone interpreter
    add_includedirs("WickedEngine/LUA", {public = true})
target_end()

-- ─────────────────────────────────────────────
-- Utility (static lib)
-- ─────────────────────────────────────────────
target("Utility")
    set_kind("static")
    set_group("ThirdParty")

    add_files("WickedEngine/Utility/**.cpp")
    add_files("WickedEngine/Utility/**.c")

    -- These are #included by other files, not compiled standalone
    remove_files("WickedEngine/Utility/mikktspace.c")
    remove_files("WickedEngine/Utility/zstd/zstd.c")
    remove_files("WickedEngine/Utility/volk.c")
    remove_files("WickedEngine/Utility/D3D12MemAlloc.cpp")

    -- FAudio is Linux/SDL2 only
    remove_files("WickedEngine/Utility/FAudio/**")

    add_includedirs("WickedEngine/Utility", {public = true})
    add_includedirs("WickedEngine/Utility/include", {public = true})
target_end()

-- ─────────────────────────────────────────────
-- Jolt (static lib)
-- ─────────────────────────────────────────────
target("Jolt")
    set_kind("static")
    set_group("ThirdParty")

    add_files("WickedEngine/Jolt/**.cpp")

    add_includedirs("WickedEngine", {public = true})

    set_pcxxheader("WickedEngine/Jolt/Jolt.h")

    add_defines(
        "JPH_DEBUG_RENDERER",
        "JPH_USE_SSE4_1",
        "JPH_USE_SSE4_2",
        "JPH_USE_AVX",
        "JPH_USE_LZCNT",
        "JPH_USE_TZCNT",
        "JPH_USE_F16C",
        "JPH_USE_FMADD"
    )

    add_cxflags("/arch:AVX", "/bigobj", {force = true})
target_end()

-- ─────────────────────────────────────────────
-- WickedEngine (static lib)
-- ─────────────────────────────────────────────
target("WickedEngine")
    set_kind("static")

    add_files("WickedEngine/*.cpp")
    remove_files("WickedEngine/offlineshadercompiler.cpp")

    add_deps("Jolt", "LUA", "Utility")

    add_includedirs("WickedEngine", {public = true})

    set_pcxxheader("WickedEngine/WickedEngine.h")

    -- Public defines
    add_defines(
        "WIN32=1",
        "_HAS_EXCEPTIONS=0",
        "UNICODE",
        "_UNICODE",
        "NOMINMAX",
        "WICKED_CMAKE_BUILD",
        -- SIMD intrinsics
        "_XM_SSE4_INTRINSICS_",
        "_XM_AVX_INTRINSICS_",
        "_XM_F16C_INTRINSICS_",
        "_XM_FMA3_INTRINSICS_",
        -- Jolt SIMD (must match Jolt target)
        "JPH_DEBUG_RENDERER",
        "JPH_USE_SSE4_1",
        "JPH_USE_SSE4_2",
        "JPH_USE_AVX",
        "JPH_USE_LZCNT",
        "JPH_USE_TZCNT",
        "JPH_USE_F16C",
        "JPH_USE_FMADD",
        {public = true}
    )

    -- MSVC flags
    add_cxflags("/W3", "/MP", "/EHsc-", "/GR-", "/bigobj", "/arch:AVX", {force = true})

    -- System libraries (supplement #pragma comment(lib) in source)
    add_syslinks("d3d12", "dxgi", "d3dcompiler", "dxguid", "comdlg32", "shell32")
target_end()

-- ─────────────────────────────────────────────
-- cube-demo (executable)
-- ─────────────────────────────────────────────
target("cube-demo")
    set_kind("binary")
    add_files("cube_demo/main.cpp")
    add_deps("WickedEngine")

    -- Windows subsystem
    add_ldflags("/SUBSYSTEM:WINDOWS", {force = true})

    -- Copy runtime files after build
    after_build(function (target)
        local targetdir = target:targetdir()
        local wickeddir = path.join(os.projectdir(), "WickedEngine")

        -- Copy dxcompiler.dll
        local dxc_src = path.join(wickeddir, "dxcompiler.dll")
        if os.isfile(dxc_src) then
            os.cp(dxc_src, targetdir)
            print("Copied dxcompiler.dll")
        end

        -- Copy shaders/
        local shaders_src = path.join(wickeddir, "shaders")
        if os.isdir(shaders_src) then
            os.cp(shaders_src, path.join(targetdir, "shaders"))
            print("Copied shaders/")
        end

        -- Copy Content/
        local content_src = path.join(os.projectdir(), "Content")
        if os.isdir(content_src) then
            os.cp(content_src, path.join(targetdir, "Content"))
            print("Copied Content/")
        end
    end)
target_end()
