// Copyright (C) 2023  ilobilo

#include <conflict/conflict.hpp>
#include <fmt/color.h>
#include <chess.hpp>

uint64_t flags = 0;
const auto parser = conflict::parser
{
    conflict::option { { 'h', "help", "Show help" }, flags, (1 << 0) },
    conflict::option { { 'v', "version", "Show version" }, flags, (1 << 1) }
};

bool parse_flags()
{
    if (flags & (1 << 0))
    {
        fmt::print("Options:\n");
        parser.print_help();
        fmt::print("How to play:\n"
                   "    Enter from and to corordinates\n"
                   "    Enter 'resign' to exit\n"
                   "Example:\n"
                   "    b2b4 or g1f3\n");
        return true;
    }
    else if (flags & (1 << 1))
    {
        fmt::print("chess v0.1\n");
        return true;
    }

    return false;
}

auto main(int argc, char **argv) -> int
{
    parser.apply_defaults();
    conflict::default_report(parser.parse(argc - 1, argv + 1));

    if (parse_flags())
        return EXIT_SUCCESS;

    chess::init(chess::colours::white);
    chess::init(chess::colours::black);

    chess::clear();

    fmt::print("Last move: none, whites: {}, blacks: {}\n\n", 0, 0);
    chess::draw();

    while (true)
    {
        fmt::print("> ");
        std::string val;
        std::cin >> val;

        if (val == "resign")
        {
            chess::resign();
            break;
        }

        chess::clear();
        if (chess::move(val) == false)
            fmt::print(fmt::fg(fmt::color::red), "Could not move the piece! {}\n\n", val);
        else
            fmt::print("Last move: {}, whites: {}, blacks: {}\n\n", val, chess::scores[size_t(chess::colours::white)], chess::scores[size_t(chess::colours::black)]);
        chess::draw();
    }

    return EXIT_SUCCESS;
}