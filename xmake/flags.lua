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
