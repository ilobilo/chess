// Copyright (C) 2024  ilobilo

#include <centurion.hpp>

#include <utility>
#include <array>
#include <optional>
#include <cstddef>

#include <chess/chess.hpp>

namespace chess
{
    inline constexpr auto window_title = "Chess";
    inline constexpr cen::iarea window_size { 600, 600 };
    inline constexpr std::size_t window_min_size = 400;

    inline constexpr std::size_t margin = 20;

    inline constexpr auto window_flags = cen::window::hidden | cen::window::resizable;
    inline constexpr auto locked_aspect_ratio = window_size.as_f().width / window_size.as_f().height;

    inline constexpr auto colour_border = cen::colors::black;
    inline constexpr auto colour_white = cen::colors::wheat;
    inline constexpr auto colour_black = cen::colors::dark_slate_grey;
    inline constexpr auto colour_circle = cen::color { 0x3C, 0xB3, 0x71, 120 };

    app::app() :
        window { window_title, window_size, window_flags },
        renderer { window.make_renderer() }, dispatcher { },
        mouse_pos { }, mouse_left_at { std::nullopt },
        selected_piece { std::nullopt }, was_on_piece { false },

        font_file { std::apply(SDL_RWFromConstMem, chess::get_font_data()) },
        knook_file { std::apply(SDL_RWFromConstMem, chess::get_knook_data()) },

        font { font_file, 16 },

        //! there's a typo in renderer.hpp:185 IMG_LoadTexture_RW -> IMG_LoadTextureRW
        //! so we use cen::surface with make_texture calls
        knook_texture { renderer.make_texture(cen::surface { knook_file }) },

        piece_files {
            [&]<std::size_t... I>(std::index_sequence<I...>) -> std::array<cen::file, 12> {
                return { cen::file { std::apply(SDL_RWFromConstMem, piece_datas[I]) } ... };
            } (std::make_index_sequence<piece_datas.size()>())
        },

        piece_textures {
            [&]<std::size_t... I>(std::index_sequence<I...>) -> std::array<cen::texture, 12> {
                return { renderer.make_texture(cen::surface { piece_files[I] }) ... };
            } (std::make_index_sequence<piece_datas.size()>())
        },
        brd { }, is_running { false }, game_over { false }
    {
        window.set_min_size(cen::iarea { window_min_size, static_cast<std::size_t>(window_min_size / locked_aspect_ratio) });

        dispatcher.bind<cen::window_event>().to<&app::on_window_event>(this);
        dispatcher.bind<cen::quit_event>().to<&app::on_quit_event>(this);
        dispatcher.bind<cen::mouse_motion_event>().to<&app::on_mouse_motion_event>(this);
        dispatcher.bind<cen::mouse_button_event>().to<&app::on_mouse_button_event>(this);
    }

    void app::run()
    {
        is_running = true;
        window.show();

        while (is_running)
        {
            dispatcher.poll();

            renderer.clear_with(cen::colors::sandy_brown);
            draw_board();

            renderer.present();
        }

        window.hide();
    }

    std::size_t app::get_board_size()
    {
        return std::min(window.width(), window.height()) - (2 * margin);
    }

    void app::draw_board()
    {
        auto board_size = get_board_size();
        auto square_size = board_size / 8.f;

        renderer.set_color(colour_border);
        renderer.draw_rect(
            cen::irect {
                cen::ipoint {
                    static_cast<cen::ipoint::value_type>(margin - 1),
                    static_cast<cen::ipoint::value_type>(margin - 1)
                },
                cen::iarea {
                    static_cast<cen::iarea::value_type>(board_size + 2),
                    static_cast<cen::iarea::value_type>(board_size + 2)
                }
            }
        );

        std::optional<std::pair<piece, cen::frect>> last_render { std::nullopt };
        bool on_piece = false;

        std::size_t i = 0;
        for (std::size_t y = 0; y < 8; y++)
        {
            for (std::size_t x = 0; x < 8; x++)
            {
                renderer.set_color(((y + i++) % 2) ? colour_black : colour_white);

                cen::fpoint pos {
                    static_cast<cen::fpoint::value_type>(margin + (x * square_size)),
                    static_cast<cen::fpoint::value_type>(margin + (y * square_size)),
                };
                renderer.fill_rect(
                    cen::frect {
                        pos, cen::farea {
                            static_cast<cen::farea::value_type>(square_size),
                            static_cast<cen::farea::value_type>(square_size)
                        }
                    }
                );

                if (auto piece = brd[x, y]; piece.get_type() != piece::type::none)
                {
                    bool selected_this = false;

                    if (!game_over && brd.get_current_turn() == piece.get_colour())
                    {
                        if (selected_piece == std::make_pair(x, y))
                            selected_this = true;

                        if (mouse_left_at)
                        {
                            auto [mx, my] = mouse_left_at.value().get();
                            auto ex = pos.x() + square_size;
                            auto ey = pos.y() + square_size;

                            if ((mx >= pos.x() && my >= pos.y()) && (mx <= ex && my <= ey))
                            {
                                selected_piece = { x, y };
                                selected_this = true;
                                was_on_piece = on_piece = true;
                            }
                        }
                    }

                    cen::farea area {
                        static_cast<cen::farea::value_type>(square_size),
                        static_cast<cen::farea::value_type>(square_size)
                    };

                    if (selected_this)
                    {
                        if (last_render != std::nullopt)
                        {
                            auto lpiece = last_render->first;
                            renderer.render(get_piece_texture(lpiece.get_colour(), lpiece.get_type()), last_render->second);
                        }

                        if (was_on_piece)
                            last_render = { piece, { cen::fpoint { mouse_pos.x() - (square_size / 2), mouse_pos.y() - (square_size / 2) }, area } };
                        else
                            last_render = { piece, { pos, area } };
                    }
                    else
                    {
                        renderer.render(
                            get_piece_texture(piece.get_colour(), piece.get_type()),
                            cen::frect { pos, area }
                        );
                    }
                }
            }
        }

        {
            auto font_size = square_size / 5;
            font.set_size(static_cast<int>(font_size));

            for (std::size_t y = 8; y > 0; y--)
            {
                renderer.render(
                    renderer.make_texture(
                        font.render_blended(
                            std::to_string(y).c_str(),
                            cen::colors::black
                        )
                    ),
                    cen::fpoint {
                        margin + (font_size / 8),
                        margin + ((8 - y) * square_size) + (font_size / 8)
                    }
                );
            }

            for (char x = 'A'; x <= 'H'; x++)
            {
                char str[2] { x, '\0' };

                renderer.render(
                    renderer.make_texture(
                        font.render_blended(
                            str,
                            cen::colors::black
                        )
                    ),
                    cen::fpoint {
                        margin + (((x - 'A') + 1) * square_size) - font_size * 0.8f,
                        margin + (square_size * 8) - font_size - (font_size / 8)
                    }
                );
            }
        }

        bool deselect = (mouse_left_at && !on_piece);
        bool one_legal = false;
        {
            bool drop = !mouse_left_at && was_on_piece;

            if (!game_over)
            {
                for (auto [index, piece] : std::views::enumerate(brd.data()))
                {
                    auto col = piece.get_colour();
                    if (col != brd.get_current_turn())
                        continue;

                    for (auto genfn : piece.possible_moves(col))
                    {
                        auto pos = board::index2pos(index);
                        auto [mv, repeatable] = genfn(pos);

                        auto iterate_legal_move = [&brd = this->brd, &one_legal](auto &mv, auto repeatable, auto &&genfn, auto &&func)
                        {
                            if (auto [legal, took] = brd.is_move_legal(mv); legal)
                            {
                                one_legal = true;
                                if (repeatable)
                                {
                                    do {
                                        if (func())
                                            return true;

                                        auto [nmv, _] = genfn(mv.to);
                                        mv.to = nmv.to;

                                        if (took == true)
                                            break;

                                        std::tie(legal, took) = brd.is_move_legal(mv);
                                    }
                                    while (legal);
                                }
                                else if (func())
                                    return true;
                            }
                            return false;
                        };

                        auto possible_move = [&]
                        {
                            auto sx = margin + (mv.to.first * square_size);
                            auto sy = margin + (mv.to.second * square_size);
                            auto ex = sx + square_size;
                            auto ey = sy + square_size;

                            if (deselect || drop)
                            {
                                auto [mx, my] = deselect ? mouse_left_at.value().get() : mouse_pos.get();
                                if ((mx >= sx && my >= sy) && (mx <= ex && my <= ey))
                                {
                                    brd.move_piece(mv);
                                    return true;
                                }
                                if (deselect)
                                    return false;
                            }

                            renderer.set_blend_mode(cen::blend_mode::blend);
                            renderer.set_color(colour_circle);
                            draw_circle(
                                sx + (square_size / 2),
                                sy + (square_size / 2),
                                square_size / 10
                            );

                            return false;
                        };

                        if (pos != selected_piece)
                        {
                            if (!one_legal)
                                iterate_legal_move(mv, repeatable, genfn, [] { return false; });
                            continue;
                        }

                        if (iterate_legal_move(mv, repeatable, genfn, possible_move))
                        {
                            brd.checked_squares = board::gen_checks(col, brd);
                            break;
                        }
                    }
                }
            }

            if (selected_piece && drop)
                was_on_piece = false;

            if (last_render)
            {
                auto piece = last_render->first;
                auto rect = last_render->second;
                renderer.render(get_piece_texture(piece.get_colour(), piece.get_type()), rect);
            }
        }

        if (!game_over && one_legal == false)
        {
            cen::message_box::show("Game Over", "No more legal moves!", cen::message_box_type::information);
            game_over = true;
        }

        if (deselect)
            selected_piece = std::nullopt;
    }

    void app::draw_circle(int cx, int cy, int radius)
    {
        auto error = -radius;
        auto x = radius;
        int y = 0;

        auto draw4 = [&](auto x, auto y)
        {
            renderer.draw_line(cen::ipoint { cx - x, cy + y }, cen::ipoint { cx + x, cy + y });
            if (y != 0)
                renderer.draw_line(cen::ipoint { cx - x, cy - y }, cen::ipoint { cx + x, cy - y });
        };

        while (x >= y)
        {
            auto ly = y;

            error += y;
            y++;
            error += y;

            draw4(x, ly);

            if (error >= 0)
            {
                if (x != ly)
                    draw4(ly, x);

                error -= x;
                x--;
                error -= x;
            }
        }
    }

    void app::on_window_event(const cen::window_event &ev)
    {
        auto ev_id = ev.event_id();
        // cen::log_info("window_event %s", cen::to_string(ev_id).data());

        switch (ev_id)
        {
            case cen::window_event_id::resized:
            {
                auto width = window.width();
                auto height = window.height();
                auto aspect = static_cast<float>(width) / height;

                if (aspect > locked_aspect_ratio)
                    height = width * (1.f / locked_aspect_ratio);
                else if (aspect < locked_aspect_ratio)
                    width = height * locked_aspect_ratio;

                window.set_size({ width, height });
                break;
            }
            default:
                break;
        }
    }

    void app::on_quit_event(const cen::quit_event &)
    {
        // cen::log_info("quit_event");
        is_running = false;
    }

    void app::on_mouse_motion_event(const cen::mouse_motion_event &ev)
    {
        // cen::log_info("mouse_motion_event");
        mouse_pos = { static_cast<cen::fpoint::value_type>(ev.x()), static_cast<cen::fpoint::value_type>(ev.y()) };
    }

    void app::on_mouse_button_event(const cen::mouse_button_event &ev)
    {
        // cen::log_info("mouse_button_event");
        if (ev.button() == cen::mouse_button::left)
        {
            if (ev.released())
                mouse_left_at = std::nullopt;
            else if (ev.pressed() && !mouse_left_at)
                mouse_left_at = { static_cast<cen::fpoint::value_type>(ev.x()), static_cast<cen::fpoint::value_type>(ev.y()) };
        }
    }
} // namespace chess