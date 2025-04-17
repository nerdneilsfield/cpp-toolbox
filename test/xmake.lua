add_requires("catch2")

-- Define test files
local test_files = {"./base/**.cpp", 
                    "./container/**.cpp",
                    "./concurrent/**.cpp",
                    "./file/**.cpp",
                    "./logger/**.cpp", 
                    "./utils/**.cpp", 
                    "./types/**.cpp", 
                    "./functional/**.cpp",
                    "./my_catch2_main.cpp"}

target("cpp-toolbox-tests")
        set_kind("binary")
        add_files(test_files)
        add_deps("cpp-toolbox_static")
        add_packages("catch2")
        add_rules("generate_export_header")
        set_pcxxheader("../src/impl/cpp-toolbox/pch.hpp")
        set_policy("build.optimization.lto", true)
        if is_plat("windows") then
                add_cxflags("/utf-8")
        end