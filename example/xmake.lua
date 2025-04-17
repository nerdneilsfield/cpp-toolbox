-- Find all .cpp files in example directory (including subdirectories)
local example_files = os.files("./**.cpp")

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
                set_pcxxheader("../src/impl/cpp-toolbox/pch.hpp")
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