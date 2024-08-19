// Copyright (C) 2024  ilobilo

#include <chess/board.hpp>
#include <centurion.hpp>
#include <ranges>

namespace chess
{
    std::pair<bool, bool> board::is_move_legal(move &mv, bool once, bool checking)
    {
        if (mv.is_valid() == false)
            return { false, false };

        auto &from = at(mv.from);
        auto &to = at(mv.to);

        auto fcol = from.get_colour();
        auto ftype = from.get_type();

        auto tcol = to.get_colour();

        if (fcol == piece::colour::none)
            return { false, false };

        if (!checking && fcol == tcol)
            return { false, false };

        if (ftype == piece::type::king && once)
        {
            if (std::ranges::contains(checked_squares, mv.to))
                return { false, false };

            auto copy = *this;
            copy.at(mv.to) = copy.at(mv.from);
            copy.at(mv.from) = piece { };

            auto checks = board::gen_checks(rev(fcol), copy, false);
            if (std::ranges::contains(checks, mv.to))
                return { false, false };
        }
        else if (ftype == piece::type::pawn)
        {
            auto [fx, fy] = mv.from;
            auto [tx, ty] = mv.to;

            if (tx == fx && tcol != piece::colour::none)
                return { false, false };

            if (fcol == piece::colour::white)
            {
                fy = rev(fy);
                ty = rev(ty);
            }

            bool first_move = from.first_move; // (fy == 1);

            if (!first_move && ty != (fy + 1))
                return { false, false };

            if (first_move && (ty > 3 || (ty == 3 && fx != tx)))
                return { false, false };

            if (ty == fy + 1 && tx != fx)
            {
                bool can_en_passant = false;

                if (last_move.has_value())
                {
                    auto [lmfx, lmfy] = last_move->mv.from;
                    auto [lmtx, lmty] = last_move->mv.to;

                    if (fcol == piece::colour::white)
                    {
                        lmfy = rev(lmfy);
                        lmty = rev(lmty);
                    }

                    can_en_passant =
                        last_move->from.get_type() == piece::type::pawn &&
                        std::abs(lmfy - lmty) == 2 && lmfx == lmtx &&
                        lmfx == tx && lmty == fy;
                }

                if (tcol == fcol)
                    return { false, false };

                if (tcol == piece::colour::none)
                {
                    if (can_en_passant)
                        mv.spec = special::enpassant;
                    else
                        return { false, false };
                }
            }

            if (ty == 7)
                mv.spec = special::promotion;
        }

        if (ftype != piece::type::king && once)
        {
            auto copy = *this;
            copy.at(mv.to) = copy.at(mv.from);
            copy.at(mv.from) = piece { };

            auto checks = board::gen_checks(rev(fcol), copy, false);
            if (std::ranges::contains(checks, get_player(fcol).king_pos))
                return { false, false };
        }

        return { true, (tcol != piece::colour::none) };
    }

    std::vector<pos> board::gen_checks(piece::colour fcol, board &brd, bool once)
    {
        std::vector<pos> checked_squares { };
        for (auto [index, piece] : std::views::enumerate(brd.data()))
        {
            if (piece.get_colour() != fcol)
                continue;

            auto pos = board::index2pos(index);
            for (auto genfn : piece.possible_moves(fcol))
            {
                auto [mv, repeatable] = genfn(pos);
                if (auto [legal, took] = brd.is_move_legal(mv, once, true); legal)
                {
                    if (repeatable)
                    {
                        do {
                            checked_squares.push_back(mv.to);

                            auto [nmv, _] = genfn(mv.to);
                            mv.to = nmv.to;

                            if (took == true)
                                break;

                            std::tie(legal, took) = brd.is_move_legal(mv, once, true);
                        }
                        while (legal);
                    }
                    else checked_squares.push_back(mv.to);
                }
            }
        }
        return checked_squares;
    }

    void board::move_piece(move mv)
    {
        // assume is_move_legal has been called
        last_move = { mv, at(mv.from), at(mv.to) };

        auto &fpiece = at(mv.from);
        auto ftype = fpiece.get_type();

        if (ftype == piece::type::pawn)
        {
            if (mv.spec == special::enpassant)
            {
                auto captured_pos = mv.to;
                auto one = (fpiece.get_colour() == piece::colour::white) ? 1 : -1;
                captured_pos.second += one;

                if (auto &tpiece = at(captured_pos); tpiece.get_type() == piece::type::pawn)
                    tpiece = piece { };
            }
            else if (mv.spec == special::promotion)
            {
                cen::message_box mb { "Promotion", "Please choose a piece to promote your pawn to" };

                mb.set_type(cen::message_box_type::information);
                mb.set_button_order(cen::message_box_button_order::left_to_right);

                mb.add_button(static_cast<int>(piece::type::knight), "Knight");
                mb.add_button(static_cast<int>(piece::type::bishop), "Bishop");
                mb.add_button(static_cast<int>(piece::type::rook), "Rook");
                mb.add_button(static_cast<int>(piece::type::queen), "Queen");

                auto button = mb.show();
                at(mv.from).set_type(static_cast<piece::type>(button.value_or(static_cast<int>(piece::type::queen))));
            }
        }
        else if (ftype == piece::type::king)
            get_player(fpiece.get_colour()).king_pos = mv.to;

        fpiece.first_move = false;

        at(mv.to) = fpiece;
        fpiece = piece { };

        current_turn = rev(current_turn);
    }
} // namespace chess