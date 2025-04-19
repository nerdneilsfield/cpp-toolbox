add_requires("catch2")

-- Define benchmark files
local benchmark_files = {"./**.cpp"}

target("cpp-toolbox-benchmark")
        set_kind("binary")
        add_files(benchmark_files)
        add_deps("cpp-toolbox_static")
        add_packages("catch2")
        add_defines("CATCH_CONFIG_MAIN")
        add_rules("generate_export_header")
        if not is_plat("macosx") then
          set_pcxxheader("../src/impl/cpp-toolbox/pch.hpp")
        end
        set_policy("build.optimization.lto", true)
        if is_plat("windows") then
                add_cxflags("/utf-8")
        end