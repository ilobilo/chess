-- Copyright (C) 2024  ilobilo

set_project("chess")
set_version("v0.1")
set_license("GPL-3.0")

add_rules("mode.release", "mode.debug")
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" })

set_policy("run.autobuild", true)

add_requires("centurion")

target("chess")
    set_kind("binary")

    add_packages("centurion")

    add_files("src/**.cpp")

    add_includedirs("src")

    set_languages("c++23")

    set_warnings("all", "error")
    set_optimize("fastest")

    on_config(function (target)
        target:add("defines", "DATA_FONT=\"" .. path.join(os.projectdir(), "data/FiraCode-Regular.ttf") .. "\"")
        target:add("defines", "DATA_KNOOK=\"" .. path.join(os.projectdir(), "data/knook.png") .. "\"")
        target:add("defines", "DATA_PIECES=\"" .. path.join(os.projectdir(), "data/pieces") .. "\"")
    end)