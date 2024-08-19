// Copyright (C) 2024  ilobilo

#pragma once

#include <functional>
#include <optional>
#include <array>

#include <chess/piece.hpp>

namespace chess
{
    struct player
    {
        std::array<std::optional<std::reference_wrapper<piece>>, 16> pieces;
        std::size_t points;
        pos king_pos;

        constexpr player() : pieces { }, points { 0 }, king_pos { } { }
    };

    class board
    {
        private:
        std::array<piece, 64> buffer;
        player white, black;

        piece::colour current_turn;

        struct move_entry
        {
            move mv;
            piece from;
            piece to;
        };
        std::optional<move_entry> last_move;

        constexpr auto rev(auto y) const { return 7 - y; }
        constexpr auto rev(piece::colour col) const
        {
            switch (col)
            {
                case piece::colour::white:
                    return piece::colour::black;
                case piece::colour::black:
                    return piece::colour::white;
                default:
                    return piece::colour::none;
            };
        }

        constexpr auto &get_player(piece::colour col)
        {
            switch (col)
            {
                case piece::colour::white:
                    return white;
                case piece::colour::black:
                    return black;
                default:
                    std::unreachable();
            };
        }

        public:
        std::vector<pos> checked_squares;

        constexpr piece &at(std::size_t x, std::size_t y) { return buffer[y * 8 + x]; }
        constexpr piece &at(pos p) { return buffer[p.second * 8 + p.first]; }
        constexpr piece &operator[](std::size_t x, std::size_t y) { return at(x, y); }
        constexpr auto &data() { return buffer; }

        constexpr piece::colour get_current_turn() const { return current_turn; }

        constexpr board() :
            buffer { }, white { }, black { },
            current_turn { piece::colour::white },
            last_move { }, checked_squares { }
        {
            std::size_t i = 0;
            auto add = [&](auto x, auto y, auto tp)
            {
                white.pieces[i] = (at(x, rev(y)) = piece { tp, piece::colour::white });
                black.pieces[i++] = (at(x, y) = piece { tp, piece::colour::black });
            };

            for (std::size_t x = 0; x < 8; x++)
                add(x, 1, piece::type::pawn);

            add(0, 0, piece::type::rook);
            add(7, 0, piece::type::rook);

            add(1, 0, piece::type::knight);
            add(6, 0, piece::type::knight);

            add(2, 0, piece::type::bishop);
            add(5, 0, piece::type::bishop);

            add(3, 0, piece::type::queen);
            add(4, 0, piece::type::king);

            white.king_pos = { 4, rev(0) };
            black.king_pos = { 4, rev(7) };
        }

        std::pair<bool, bool> is_move_legal(move &mv, bool once = true, bool checking = false);
        void move_piece(move mv);

        static std::vector<pos> gen_checks(piece::colour fcol, board &brd, bool once = true);

        static constexpr pos index2pos(std::size_t index) { return { index % 8, index / 8 }; }
    };
} // namespace chess