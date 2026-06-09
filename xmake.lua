add_rules("mode.debug", "mode.release")

set_languages("c++20")

target("sm_loadingscreen_override")
    set_kind("shared")
    add_files("src/*.cpp")
    add_syslinks("user32")