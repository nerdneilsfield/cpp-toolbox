set_project("cpp-toolbox")
set_version("0.1.0")
set_description("A tools box library for accelerating cpp programe developing.")
set_xmakever("2.7.0")
set_languages("c++17")

-- Set common compilation flags
add_rules("mode.debug", "mode.release", "mode.coverage", "mode.profile", "mode.asan", "mode.tsan", "mode.lsan",
    "mode.ubsan")
set_policy("build.ccache", true)

includes("xmake/options.lua")
includes("xmake/flags.lua")
includes("xmake/common.lua")


-- Include and link directories
add_includedirs("src/include")

-- Dependencies: concurrentqueue and catch2
add_requires("concurrentqueue")

-- Define source files
local source_files = {"src/impl/cpp-toolbox/base/**.cpp",
                      "src/impl/cpp-toolbox/container/**.cpp",
                      "src/impl/cpp-toolbox/types/**.cpp",
                      -- "src/impl/cpp-toolbox/concurrent/**.cpp",
                      "src/impl/cpp-toolbox/utils/**.cpp",
                      "src/impl/cpp-toolbox/file/**.cpp", 
                      "src/impl/cpp-toolbox/logger/**.cpp",
                      "src/impl/cpp-toolbox/io/**.cpp"}

-- Define the shared library target
target("cpp-toolbox")
        add_rules("generate_export_header")
        add_files(source_files)
        add_headerfiles("src/include/(**.hpp)")
        add_packages("concurrentqueue")
        set_kind("shared")
        if not is_plat("macosx") then
          set_pcxxheader("src/impl/cpp-toolbox/pch.hpp")
        end
        add_defines("CPP_TOOLBOX_EXPORTS")
        set_policy("build.optimization.lto", true)


        -- Add thread library
        add_syslinks(is_plat("windows") and "kernel32" or "pthread")

        -- Set symbol visibility on non-Windows platforms
        if not is_plat("windows") then
          add_cxflags("-fvisibility=hidden")
        else
          add_cxflags("/utf-8")
        end

-- Define the static library target
target("cpp-toolbox_static")
        add_rules("generate_export_header")
        add_files(source_files)
        add_headerfiles("src/include/(**.hpp)")
        add_packages("concurrentqueue")
        set_kind("static")
        if not is_plat("macosx") then
          set_pcxxheader("src/impl/cpp-toolbox/pch.hpp")
        end
        add_defines("CPP_TOOLBOX_STATIC_DEFINE")
        set_policy("build.optimization.lto", true)


        -- Add thread library
        add_syslinks(is_plat("windows") and "kernel32" or "pthread")

        -- Set symbol visibility on non-Windows platforms
        if not is_plat("windows") then
          add_cxflags("-fvisibility=hidden")
        else
          add_cxflags("/utf-8")
        end

-- Define tests if enabled
if has_config("tests") or has_config("developer") then
    includes("test")
end

-- Build examples if enabled
if has_config("examples") or has_config("developer") then
    includes("example")
end

-- Define tests if enabled
if has_config("benchmark") or has_config("developer") then
    includes("benchmark")
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
