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
                    "./io/**.cpp",
                    "./my_catch2_main.cpp"}

target("cpp-toolbox-tests")
        set_kind("binary")
        add_files(test_files)
        add_deps("cpp-toolbox_static")
        add_packages("catch2")
        add_rules("generate_export_header")
        if not is_plat("macosx") then
          set_pcxxheader("../src/impl/cpp-toolbox/pch.hpp")
        end
        set_policy("build.optimization.lto", true)
        if is_plat("windows") then
                add_cxflags("/utf-8")
        end

        local script_dir = os.scriptdir()
        local test_data_dir_path = script_dir .. "/data"

        print(test_data_dir_path)

        set_configdir("$(buildir)/generated")
        add_configfiles("./test_data_dir.xmake.hpp.in", {
                variables = {
                        TEST_DATA_DIR = test_data_dir_path
                },
                filename = "test_data_dir.hpp"
        })
        add_includedirs("$(buildir)/generated")
