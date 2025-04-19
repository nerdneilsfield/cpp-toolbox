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

-- Benchmark build option
option("benchmark")
set_default(false)
set_showmenu(true)
set_category("Development")
set_description("Build benchmarks")
option_end()
