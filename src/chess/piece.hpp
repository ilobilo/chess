// Copyright (C) 2024  ilobilo

#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <ranges>

namespace chess
{
    using pos = std::pair<std::int8_t, std::int8_t>;

    enum class special
    {
        enpassant,
        promotion,
        castles,
        none
    };

    struct move
    {
        pos from;
        pos to;
        special spec = special::none;

        constexpr bool is_valid()
        {
            auto is_one_valid = [](auto p) {
                return p.first >= 0 && p.first < 8 && p.second >= 0 && p.second < 8;
            };
            return is_one_valid(from) && is_one_valid(to);
        }
    };

    class piece
    {
        public:
        enum class type
        {
            bishop,
            king,
            knight,
            pawn,
            queen,
            rook,
            knook,
            none
        };
        enum class colour
        {
            white,
            black,
            none
        };

        private:
        type tp;
        colour col;

        using func_ret = std::pair<move, bool>;
        using func = func_ret (*)(pos);

        inline static constexpr func moves[]
        {
            // bishop
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second + 1 } }, true }; },
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second - 1 } }, true }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second + 1 } }, true }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second - 1 } }, true }; },

            // rook
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second } }, true }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second } }, true }; },
            [](pos from) -> func_ret { return { { from, { from.first, from.second + 1 } }, true }; },
            [](pos from) -> func_ret { return { { from, { from.first, from.second - 1 } }, true }; },

            // knight
            [](pos from) -> func_ret { return { { from, { from.first + 2, from.second - 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first + 2, from.second + 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first - 2, from.second - 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first - 2, from.second + 1 } }, false }; },

            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second + 2 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second + 2 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second - 2 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second - 2 } }, false }; },

            // king
            [](pos from) -> func_ret { return { { from, { from.first, from.second + 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first, from.second - 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second - 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second - 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second } }, false }; },

            // pawn
            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second + 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second + 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first, from.second + 1 } }, true }; },

            [](pos from) -> func_ret { return { { from, { from.first + 1, from.second - 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first - 1, from.second - 1 } }, false }; },
            [](pos from) -> func_ret { return { { from, { from.first, from.second - 1 } }, true }; }
        };

        static func at(std::size_t i) { return moves[i]; }

        public:
        bool first_move;

        constexpr piece() : tp { type::none }, col { colour::none }, first_move { true } { }
        constexpr piece(type tp, colour col) : tp { tp }, col { col }, first_move { true } { }

        auto possible_moves(colour col = colour::white) const
        {
            switch (tp)
            {
                case type::bishop:
                    return std::views::iota(0uz, 4uz) | std::views::transform(at);
                case type::king:
                    return std::views::iota(16uz, 24uz) | std::views::transform(at);
                case type::knight:
                    return std::views::iota(8uz, 16uz) | std::views::transform(at);
                case type::pawn:
                    if (col == colour::black)
                        return std::views::iota(22uz, 25uz) | std::views::transform(at);
                    else if (col == colour::white)
                        return std::views::iota(25uz, 28uz) | std::views::transform(at);
                    std::unreachable();
                case type::queen:
                    return std::views::iota(0uz, 8uz) | std::views::transform(at);
                case type::rook:
                    return std::views::iota(4uz, 8uz) | std::views::transform(at);
                default:
                    std::unreachable();
                    break;
            }
        }

        constexpr auto get_type() const { return tp; }
        constexpr void set_type(type tp) { this->tp = tp; }

        constexpr auto get_colour() const { return col; }
    };
} // namespace chess