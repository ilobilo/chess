// Copyright (C) 2024  ilobilo

#pragma once

#include <centurion.hpp>

#include <optional>
#include <utility>
#include <array>
#include <cstddef>

#include <chess/board.hpp>
#include <chess/piece.hpp>

namespace chess
{
    class app
    {
        private:
        using event_dispatcher = cen::event_dispatcher<
                cen::window_event,
                cen::quit_event,
                cen::mouse_motion_event,
                cen::mouse_button_event
            >;

        cen::window window;
        cen::renderer renderer;
        event_dispatcher dispatcher;

        cen::fpoint mouse_pos;
        std::optional<cen::fpoint> mouse_left_at;
        std::optional<pos> selected_piece;
        bool was_on_piece;

        cen::file font_file;
        cen::file knook_file;

        cen::file move_file;
        cen::file capture_file;

        cen::music move_audio;
        cen::music capture_audio;

        cen::font font;

        cen::texture knook_texture;

        std::array<cen::file, 12> piece_files;
        std::array<cen::texture , 12> piece_textures;

        board brd;

        bool is_running;
        bool game_over;
        bool next_game_over;

        inline auto &get_piece_texture(piece::colour c, piece::type p)
        {
            if (p == piece::type::knook)
                return knook_texture;
            return piece_textures[std::size_t(c) * 6 + std::size_t(p)];
        }

        std::size_t get_board_size();

        void draw_board();
        void draw_circle(int cx, int cy, int radius);

        void on_window_event(const cen::window_event &);
        void on_quit_event(const cen::quit_event &);
        void on_mouse_motion_event(const cen::mouse_motion_event &);
        void on_mouse_button_event(const cen::mouse_button_event &);

        public:
        app();

        void run();
    };

    extern std::array<std::pair<const void *, std::size_t>, 12> piece_datas;

    std::pair<const void *, std::size_t> get_knook_data();
    std::pair<const void *, std::size_t> get_font_data();
    std::pair<const void *, std::size_t> get_move_data();
    std::pair<const void *, std::size_t> get_capture_data();
} // namespace chess