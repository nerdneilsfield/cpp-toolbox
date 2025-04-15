set_project("cpp-toolbox")
set_version("0.1.0")
set_description("A tools box library for accelerating cpp programe developing.")
set_xmakever("2.7.0")
set_languages("c++17")

-- Developer mode option
option("developer")
set_default(false)
set_showmenu(true)
set_category("Development")
set_description("Enable developer mode")
option_end()

-- Documentation build option
option("docs")
set_default(false)
set_showmenu(true)
set_category("Development")
set_description("Build documentation")
option_end()

-- Tests build option
option("tests")
set_default(false)
set_showmenu(true)
set_category("Development")
set_description("Build tests")
option_end()

-- Examples build option
option("examples")
set_default(false)
set_showmenu(true)
set_category("Development")
set_description("Build examples")
option_end()

-- Set common compilation flags
add_rules("mode.debug", "mode.release", "mode.coverage", "mode.profile", "mode.asan", "mode.tsan", "mode.lsan",
    "mode.ubsan")
set_policy("build.ccache", true)

-- Platform specific configurations
if is_plat("windows") then
    add_cxflags("/sdl", "/guard:cf", "/utf-8", "/diagnostics:caret", "/w14165", "/w44242", "/w44254", "/w44263",
        "/w34265", "/w34287", "/w44296", "/w44365", "/w44388", "/w44464", "/w14545", "/w14546", "/w14547", "/w14549",
        "/w14555", "/w34619", "/w34640", "/w24826", "/w14905", "/w14906", "/w14928", "/w45038", "/W4", "/permissive-",
        "/volatile:iso", "/Zc:inline", "/Zc:preprocessor", "/Zc:enumTypes", "/Zc:lambda", "/Zc:__cplusplus",
        "/Zc:externConstexpr", "/Zc:throwingNew", "/EHsc")
    add_ldflags("/machine:x64", "/guard:cf")
    add_defines("NOMINMAX", "_CRT_SECURE_NO_WARNINGS")
elseif is_plat("linux", "macosx") then
    add_cxflags("-fstack-protector-strong", "-Wall", "-Wextra", "-Wpedantic", "-Wconversion", "-Wsign-conversion",
        "-Wcast-qual", "-Wformat=2", "-Wundef", "-Werror=float-equal", "-Wshadow", "-Wcast-align", "-Wunused",
        "-Wnull-dereference", "-Wdouble-promotion", "-Wimplicit-fallthrough", "-Wextra-semi", "-Woverloaded-virtual",
        "-Wnon-virtual-dtor", "-Wold-style-cast")
    if is_plat("linux") then
        add_cxflags("-U_FORTIFY_SOURCE", "-D_FORTIFY_SOURCE=3", "-D_GLIBCXX_ASSERTIONS=1", "-fcf-protection=full",
            "-fstack-clash-protection")
        add_ldflags("-Wl,--allow-shlib-undefined,--as-needed,-z,noexecstack,-z,relro,-z,now,-z,nodlopen")
    end
end

-- Create a rule to generate the export header
rule("generate_export_header")
on_config(function(target)
    -- Create the output directory
    local outputdir = path.join("$(buildir)", "exports", "cpp-toolbox")
    os.mkdir(outputdir)

    -- Export header content
    local content = [[
#ifndef CPP_TOOLBOX_EXPORT_H
#define CPP_TOOLBOX_EXPORT_H

#ifdef CPP_TOOLBOX_STATIC_DEFINE
#  define CPP_TOOLBOX_EXPORT
#  define CPP_TOOLBOX_NO_EXPORT
#else
#  ifdef _MSC_VER
#    ifdef CPP_TOOLBOX_EXPORTS
#      define CPP_TOOLBOX_EXPORT __declspec(dllexport)
#    else
#      define CPP_TOOLBOX_EXPORT __declspec(dllimport)
#    endif
#    define CPP_TOOLBOX_NO_EXPORT
#  else
#    ifdef CPP_TOOLBOX_EXPORTS
#      define CPP_TOOLBOX_EXPORT __attribute__((visibility("default")))
#    else
#      define CPP_TOOLBOX_EXPORT
#    endif
#    define CPP_TOOLBOX_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

/* This needs to suppress only for MSVC */
#if defined(_MSC_VER) && !defined(__ICL)
#  define CPP_TOOLBOX_NEW_SUPPRESS_C4251 _Pragma("warning(suppress:4251)")
#else
#  define CPP_TOOLBOX_NEW_SUPPRESS_C4251
#endif

#endif
]]

    -- Write to output file
    local output_file = path.join(outputdir, "cpp-toolbox_export.hpp")
    io.writefile(output_file, content)

    -- Add the build directory to include paths
    target:add("includedirs", "$(buildir)/exports")
end)
rule_end()

-- Include and link directories
add_includedirs("src/include")

-- Dependencies: concurrentqueue and catch2
add_requires("concurrentqueue")
add_requires("catch2")

-- Define source files
local source_files = {"src/impl/cpp-toolbox/base/**.cpp",
                      "src/impl/cpp-toolbox/container/**.cpp",
                      "src/impl/cpp-toolbox/types/**.cpp",
                      -- "src/impl/cpp-toolbox/concurrent/**.cpp",
                      "src/impl/cpp-toolbox/utils/**.cpp",
                      "src/impl/cpp-toolbox/file/**.cpp", 
                      "src/impl/cpp-toolbox/logger/**.cpp"}

-- Define test files
local test_files = {"test/base/**.cpp", 
                    -- "test/container/**.cpp",
                    -- "test/concurrent/**.cpp",
                    -- "test/file/**.cpp",
                    -- "test/logger/**.cpp", 
                    "test/utils/**.cpp", 
                    -- "test/types/**.cpp", 
                    -- "test/functional/**.cpp",
                    "test/my_catch2_main.cpp"}

-- Define benchmark files
local benchmark_files = {"benchmark/**.cpp"}

-- Define the shared library target
target("cpp-toolbox")
        add_rules("generate_export_header")
        add_files(source_files)
        add_headerfiles("src/include/(**.hpp)")
        add_packages("concurrentqueue")
        set_kind("shared")
        set_pcxxheader("src/impl/cpp-toolbox/pch.hpp")
        add_defines("CPP_TOOLBOX_EXPORTS")
        set_policy("build.optimization.lto", true)


        -- Add thread library
        add_syslinks(is_plat("windows") and "kernel32" or "pthread")

        -- Set symbol visibility on non-Windows platforms
        if not is_plat("windows") then
        add_cxflags("-fvisibility=hidden")
        end

-- Define the static library target
target("cpp-toolbox_static")
        add_rules("generate_export_header")
        add_files(source_files)
        add_headerfiles("src/include/(**.hpp)")
        add_packages("concurrentqueue")
        set_kind("static")
        set_pcxxheader("src/impl/cpp-toolbox/pch.hpp")
        add_defines("CPP_TOOLBOX_STATIC_DEFINE")
        set_policy("build.optimization.lto", true)


        -- Add thread library
        add_syslinks(is_plat("windows") and "kernel32" or "pthread")

        -- Set symbol visibility on non-Windows platforms
        if not is_plat("windows") then
        add_cxflags("-fvisibility=hidden")
        end

-- Define tests if enabled
if has_config("tests") or has_config("developer") then
    target("cpp-toolbox-tests")
        set_kind("binary")
        add_files(test_files)
        add_deps("cpp-toolbox_static")
        add_packages("catch2")
        add_rules("generate_export_header")
        set_pcxxheader("src/impl/cpp-toolbox/pch.hpp")
        set_policy("build.optimization.lto", true)
end

-- Build examples if enabled
if has_config("examples") or has_config("developer") then
    -- Find all .cpp files in example directory (including subdirectories)
    local example_files = os.files("example/**.cpp")

    -- If files are found
    if example_files and #example_files > 0 then
        -- Iterate through all cpp files
        for _, filepath in ipairs(example_files) do
            -- Get filename (without path and extension) as target name
            local targetname = path.filename(filepath)

            -- remove the .cpp extension
            targetname = path.basename(targetname, ".cpp")
            -- remove the last example of the path
            targetname = string.gsub(targetname, "_example$", "")
            -- Define a target for each file
            target("example_" .. targetname)
                set_kind("binary")
                add_files(filepath)
                add_deps("cpp-toolbox")
                add_rules("generate_export_header")
                set_pcxxheader("src/impl/cpp-toolbox/pch.hpp")
                set_policy("build.optimization.lto", true)
                -- Add platform specific system libraries
                if is_plat("linux", "macosx", "bsd") then
                        add_syslinks("pthread")
                elseif is_plat("windows") then
                        add_syslinks("kernel32")
                else
                        add_syslinks("pthread")
                end

            -- Print information
            print("Added example target: example_%s from %s", targetname, filepath)
        end
    end
end

-- Define tests if enabled
if has_config("benchmark") or has_config("developer") then
    target("cpp-toolbox-benchmark")
        set_kind("binary")
        add_files(benchmark_files)
        add_deps("cpp-toolbox_static")
        add_packages("catch2")
        add_defines("CATCH_CONFIG_MAIN")
        add_rules("generate_export_header")
        set_pcxxheader("src/impl/cpp-toolbox/pch.hpp")
        set_policy("build.optimization.lto", true)
end



-- Documentation with Doxygen
-- if has_config("docs") or has_config("developer") then
--     add_requires("doxygen", {
--         optional = true
--     })

--     target("docs")
--     set_kind("phony")
--     on_build(function(target)
--         os.exec("doxygen docs/Doxyfile")
--     end)
-- end
