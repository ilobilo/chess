// Copyright (C) 2024  ilobilo

#include <centurion.hpp>
#include <chess/chess.hpp>

int main(int argc, char* argv[])
{
    const cen::sdl sdl;
    const cen::img img;
    const cen::ttf ttf;

    chess::app app { };
    app.run();

    return 0;
}