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
       // For MSVC when consuming the DLL, don't mark templates as dllimport
#      define CPP_TOOLBOX_EXPORT
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
