// Copyright (C) 2023  ilobilo

#pragma once

#include <frozen/map.h>

#include <cstdint>
#include <utility>
#include <vector>
#include <memory>

namespace chess
{
    enum class colours
    {
        black,
        white
    };

    constexpr inline auto get_y(colours colour, int8_t off)
    {
        return colour == colours::white ? 0 + off : 7 - off;
    }

    struct piece;
    inline piece *board[8][8];
    inline int8_t scores[2];

    inline constexpr auto chars = frozen::make_map<std::tuple<colours, uint8_t>, const char *>({
        { { colours::white, 0 }, "♔" },
        { { colours::white, 1 }, "♕" },
        { { colours::white, 2 }, "♖" },
        { { colours::white, 3 }, "♗" },
        { { colours::white, 4 }, "♘" },
        { { colours::white, 5 }, "♙" },
        { { colours::black, 0 }, "♚" },
        { { colours::black, 1 }, "♛" },
        { { colours::black, 2 }, "♜" },
        { { colours::black, 3 }, "♝" },
        { { colours::black, 4 }, "♞" },
        { { colours::black, 5 }, "♟︎" }
    });

    struct piece
    {
        bool killed = false;
        const int8_t value = 0;
        const char *character;
        colours colour;
        int8_t x = 0;
        int8_t y = 0;

        bool move(int8_t to_x, int8_t to_y);

        virtual bool internal_move(int8_t to_x, int8_t to_y) { return true; };
        virtual std::vector<std::pair<int8_t, int8_t>> possible_moves() = 0;

        piece(int8_t value, const char *character, colours colour, int8_t x, int8_t y) : value(value), character(character), colour(colour), x(x), y(y)
        {
            board[this->x][this->y] = this;
        }

        virtual ~piece() { board[this->x][this->y] = nullptr; }
    };

    struct king : piece
    {
        bool has_moved = false;
        bool check = false;

        bool internal_move(int8_t to_x, int8_t to_y);
        std::vector<std::pair<int8_t, int8_t>> possible_moves();

        king(colours colour) : piece(INT8_MAX, chars.at({ colour, 0 }), colour, 4, get_y(colour, 0)) { }
    };

    struct queen : piece
    {
        std::vector<std::pair<int8_t, int8_t>> possible_moves();

        queen(colours colour, int8_t x = 3, bool reverse = false) : piece(9, chars.at({ colour, 1 }), colour, x, get_y(colour, reverse ? 7 : 0)) { }
    };

    struct rook : piece
    {
        bool has_moved = false;

        bool internal_move(int8_t to_x, int8_t to_y);
        std::vector<std::pair<int8_t, int8_t>> possible_moves();

        rook(colours colour, int8_t x) : piece(5, chars.at({ colour, 2 }), colour, x, get_y(colour, 0)) { }
    };

    struct bishop : piece
    {
        std::vector<std::pair<int8_t, int8_t>> possible_moves();

        bishop(colours colour, int8_t x) : piece(3, chars.at({ colour, 3 }), colour, x, get_y(colour, 0)) { }
    };

    struct knight : piece
    {
        std::vector<std::pair<int8_t, int8_t>> possible_moves();

        knight(colours colour, int8_t x) : piece(3, chars.at({ colour, 4 }), colour, x, get_y(colour, 0)) { }
    };

    struct pawn : piece
    {
        bool has_moved = false;

        bool internal_move(int8_t to_x, int8_t to_y);
        std::vector<std::pair<int8_t, int8_t>> possible_moves();

        pawn(colours colour, int8_t x) : piece(1, chars.at({ colour, 5 }), colour, x, get_y(colour, 1)) { }
    };

    void init(colours colour);

    void clear();
    void draw();

    void resign();
    bool move(std::string_view val);
} // namespace chess