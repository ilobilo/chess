// Copyright (C) 2023  ilobilo

#include <fmt/ranges.h>
#include <fmt/color.h>
#include <functional>
#include <chess.hpp>
#include <ranges>

namespace chess
{
    static colours curr = colours::white;
    static colours next = colours::black;
    static bool generating = false;

    constexpr inline bool in_range(int8_t x, int8_t y)
    {
        return x >= 0 && y >= 0 && x < 8 && y < 8;
    }

    constexpr inline size_t colour2index(colours colour)
    {
        switch (colour)
        {
            case colours::black:
                return 0;
            case colours::white:
                return 1;
            default:
                __builtin_unreachable();
        }
    }

    constexpr inline auto colour2str(colours colour)
    {
        switch (colour)
        {
            case colours::black:
                return "Black";
            case colours::white:
                return "White";
            default:
                __builtin_unreachable();
        }
    }

    constexpr inline colours revcolour(colours colour)
    {
        switch (colour)
        {
            case colours::black:
                return colours::white;
            case colours::white:
                return colours::black;
            default:
                __builtin_unreachable();
        }
    }

    inline bool is_king(piece *piece)
    {
        return piece != nullptr && (piece == kings[0] || piece == kings[1]);
    }

    inline bool is_in(auto &container, auto value)
    {
        return std::ranges::find(container.begin(), container.end(), value) != container.end();
    }

    inline bool remove_from(auto &container, auto pred)
    {
        return container.erase(std::remove_if(container.begin(), container.end(), pred), container.end()) != container.end();
    }

    inline bool king_checked(colours colour)
    {
        auto index = colour2index(colour);
        auto king = kings[index];
        return is_in(checks[index], std::pair(king->x, king->y));
    }

    bool generate_checks(colours colour)
    {
        generating = true;

        auto index = colour2index(colour);
        checks[index].clear();

        for (const auto &piece : pieces[colour2index(revcolour(colour))])
        {
            auto moves = piece->possible_moves();
            checks[index].insert(checks[index].end(), moves.begin(), moves.end());
        }

        return checks[index].empty() == (generating = false);
    }

    enum flags { none, should_kill, shouldn_kill };
    bool can_go(auto &container, colours colour, int8_t x, int8_t y, flags flags = none)
    requires requires(int8_t x, int8_t y) { container.emplace_back(x, y); }
    {
        if (in_range(x, y))
        {
            auto piece = board[x][y];
            bool kills = (piece != nullptr);
            if ((kills && piece->colour == colour) || (!kills && generating == false && flags == should_kill) || (kills && flags == shouldn_kill))
                return false;

            container.emplace_back(x, y);
            return (generating == true && is_king(board[x][y])) ? true : (kills == false);
        }
        return false;
    }

    bool kill_or_move(piece *piece, int8_t x, int8_t y)
    {
        auto old = board[x][y];
        if (is_king(old) == true)
            return false;

        if (old != nullptr && old->colour != piece->colour)
        {
            scores[colour2index(piece->colour)] += old->value;
            remove_from(pieces[colour2index(old->colour)], [&](auto sptr) { return sptr == old; });
        }

        board[x][y] = piece;
        board[piece->x][piece->y] = nullptr;

        piece->x = x;
        piece->y = y;

        return true;
    }

    inline void long_possible_moves(auto &array, auto &container, piece *piece)
    {
        for (const auto &func : array)
        {
            int8_t to_x = piece->x;
            int8_t to_y = piece->y;

            while (true)
            {
                std::tie(to_x, to_y) = func(to_x, to_y);
                if (can_go(container, piece->colour, to_x, to_y))
                    continue;
                break;
            }
        }
    }

    static std::function<std::pair<int8_t, int8_t> (int8_t x, int8_t y)> rook_funcs[]
    {
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x + 1, y }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x - 1, y }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x, y + 1 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x, y - 1 }; }
    };

    static std::function<std::pair<int8_t, int8_t> (int8_t x, int8_t y)> bishop_funcs[]
    {
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x + 1, y + 1 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x + 1, y - 1 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x - 1, y + 1 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x - 1, y - 1 }; }
    };

    static std::function<std::pair<int8_t, int8_t> (int8_t x, int8_t y)> knight_funcs[]
    {
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x + 1, y + 2 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x + 1, y - 2 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x - 1, y + 2 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { x - 1, y - 2 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { y + 1, x + 2 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { y + 1, x - 2 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { y - 1, x + 2 }; },
        [](int8_t x, int8_t y) -> std::pair<int8_t, int8_t> { return { y - 1, x - 2 }; }
    };

    bool piece::move(int8_t to_x, int8_t to_y)
    {
        auto moves = this->possible_moves();
        if (is_in(moves, std::pair(to_x, to_y)) == false)
            return false;

        if (kill_or_move(this, to_x, to_y) == false)
            return false;

        return this->internal_move(to_x, to_y);
    }

    bool king::internal_move(int8_t to_x, int8_t to_y) { return this->has_moved = true; }
    std::vector<std::pair<int8_t, int8_t>> king::possible_moves()
    {
        auto index = colour2index(this->colour);

        std::vector<std::pair<int8_t, int8_t>> moves;
        auto in = [&](auto &funcs)
        {
            for (const auto &func : funcs)
            {
                auto [to_x, to_y] = func(this->x, this->y);
                if (generating == false && is_in(checks[index], std::pair(to_x, to_y)))
                    continue;

                can_go(moves, this->colour, to_x, to_y);
            }
        };

        in(bishop_funcs);
        in(rook_funcs);

        return moves;
    }

    bool queen::internal_move(int8_t to_x, int8_t to_y) { return true; };
    std::vector<std::pair<int8_t, int8_t>> queen::possible_moves()
    {
        std::vector<std::pair<int8_t, int8_t>> moves;
        long_possible_moves(bishop_funcs, moves, this);
        long_possible_moves(rook_funcs, moves, this);
        return moves;
    }

    bool rook::internal_move(int8_t to_x, int8_t to_y) { return this->has_moved = true; }
    std::vector<std::pair<int8_t, int8_t>> rook::possible_moves()
    {
        std::vector<std::pair<int8_t, int8_t>> moves;
        long_possible_moves(rook_funcs, moves, this);
        return moves;
    }

    std::vector<std::pair<int8_t, int8_t>> bishop::possible_moves()
    {
        std::vector<std::pair<int8_t, int8_t>> moves;
        long_possible_moves(bishop_funcs, moves, this);
        return moves;
    }

    std::vector<std::pair<int8_t, int8_t>> knight::possible_moves()
    {
        std::vector<std::pair<int8_t, int8_t>> moves;
        for (const auto &func : knight_funcs)
        {
            auto [to_x, to_y] = func(this->x, this->y);
            can_go(moves, this->colour, to_x, to_y);
        }
        return moves;
    }

    bool pawn::internal_move(int8_t to_x, int8_t to_y)
    {
        if (this->has_moved == false)
            this->has_moved = true;

        if (to_y == 7)
        {
            auto index = colour2index(this->colour);
            remove_from(pieces[index], [&](auto sptr) { return sptr == this; });
            pieces[index].emplace_back(new queen(this->colour, to_x, true));
        }

        return true;
    };

    std::vector<std::pair<int8_t, int8_t>> pawn::possible_moves()
    {
        std::vector<std::pair<int8_t, int8_t>> moves;

        auto add = [&](int8_t pos, int8_t val) { return this->colour == colours::black ? pos - val : pos + val; };
        auto sub = [&](int8_t pos, int8_t val) { return this->colour == colours::black ? pos + val : pos - val; };

        can_go(moves, this->colour, this->x, add(this->y, 1), shouldn_kill);

        if (this->has_moved == false)
            can_go(moves, this->colour, this->x, add(this->y, 2), shouldn_kill);

        can_go(moves, this->colour, add(this->x, 1), add(this->y, 1), should_kill);
        can_go(moves, this->colour, sub(this->x, 1), add(this->y, 1), should_kill);

        return moves;
    }

    void init(colours colour)
    {
        auto index = colour2index(colour);

        for (size_t i = 0; i < 8; i++)
            pieces[index].push_back(new pawn(colour, i));

        pieces[index].push_back(new rook(colour, 0));
        pieces[index].push_back(new rook(colour, 7));

        pieces[index].push_back(new knight(colour, 1));
        pieces[index].push_back(new knight(colour, 6));

        pieces[index].push_back(new bishop(colour, 2));
        pieces[index].push_back(new bishop(colour, 5));

        pieces[index].push_back(new queen(colour));

        pieces[index].push_back(kings[index] = new king(colour));
    }

    void clear()
    {
        fmt::print("\033[2J\033[1;1H");
    }

    void draw()
    {
        auto currbg = fmt::color::white;
        auto nextbg = fmt::color::gray;

        auto colour = [&]
        {
            auto old = currbg;
            std::swap(currbg, nextbg);
            return old;
        };

        for (int8_t y = 7; y >= 0; y--)
        {
            fmt::print("{} ", y + 1);
            for (int8_t x = 0; x < 8; x++)
            {
                auto piece = board[x][y];

                if (piece != nullptr)
                    fmt::print(fmt::bg(colour()) | fmt::fg(fmt::color::black), "{} ", piece->character);
                else
                    fmt::print(fmt::bg(colour()), "  ");
            }
            fmt::print("\n");
            std::swap(currbg, nextbg);
        }
        fmt::print("  a b c d e f g h\n");
    }

    constexpr inline auto plus(char c)
    {
        return c + (std::isupper(c) ? 'A' : 'a');
    }
    constexpr inline auto minus(char c)
    {
        return c - (std::isupper(c) ? 'A' : 'a');
    }

    constexpr inline std::pair<char, int8_t> pos2name(int8_t x, int8_t y)
    {
        return { char(plus(x)), y + 1 };
    }

    constexpr inline std::pair<int8_t, int8_t> name2pos(std::string_view name)
    {
        return { minus(name[0]), name[1] - '0' - 1 };
    }

    void resign()
    {
        fmt::print("{} wins!\n", colour2str(next));
    }

    static void checkmate(colours colour)
    {
        auto index = colour2index(colour);
        if (kings[index]->possible_moves().empty())
        {
            if (king_checked(colour) == true)
            {
                resign();
                std::exit(EXIT_SUCCESS);
            }
            else if (pieces[index].size() == 1)
            {
                fmt::print("Draw!\n");
                std::exit(EXIT_SUCCESS);
            }
        }
    }

    bool move(std::string_view val)
    {
        if (val.length() != 4 || !std::isalpha(val[0]) || !std::isdigit(val[1]) || !std::isalpha(val[2]) || !std::isdigit(val[3]))
            return false;

        auto [from_x, from_y] = name2pos(val.substr(0, 2));
        auto [to_x, to_y] = name2pos(val.substr(2, 4));

        auto piece = board[from_x][from_y];
        if (piece == nullptr || piece->colour != curr)
            return false;

        generate_checks(curr);

        if (king_checked(curr) == true && piece != kings[colour2index(curr)])
            return false; // TODO: if possible, get in front of it or kill it with other piece

        checkmate(curr);

        if (piece->move(to_x, to_y) == false)
            return false;

        std::swap(curr, next);

        generate_checks(curr);
        checkmate(curr);

        return true;
    }
} // namespace chess