// Copyright (C) 2024  ilobilo

#include <utility>
#include <array>
#include <cstddef>

#define CONCAT_IMPL(A, B) A ## B
#define CONCAT(A, B) CONCAT_IMPL(A, B)

#define IMPORT_BIN(file, sym)                          \
    asm (                                              \
        ".section .rodata\n"                           \
        ".balign 4\n"                                  \
        ".global " #sym "\n"                           \
        #sym ":\n"                                     \
        ".incbin \"" file "\"\n"                       \
        ".global " #sym "_size\n"                      \
        ".set " #sym "_size, . - " #sym "\n"           \
        ".balign 4\n"                                  \
        ".section \".text\"\n"                         \
    );                                                 \
    extern "C" const char sym[], CONCAT(sym, _size)[];

#define IMPORT_PIECE_IMPL(file, sym) IMPORT_BIN(file, sym)
#define IMPORT_PIECE(cpiece) IMPORT_PIECE_IMPL(DATA_PIECES "/" #cpiece ".png", CONCAT(piece_, cpiece))

#define IMPORT_PIECES_IMPL(piece) IMPORT_PIECE(piece)
#define IMPORT_PIECES(colour)             \
    IMPORT_PIECES_IMPL(CONCAT(colour, b)) \
    IMPORT_PIECES_IMPL(CONCAT(colour, k)) \
    IMPORT_PIECES_IMPL(CONCAT(colour, n)) \
    IMPORT_PIECES_IMPL(CONCAT(colour, p)) \
    IMPORT_PIECES_IMPL(CONCAT(colour, q)) \
    IMPORT_PIECES_IMPL(CONCAT(colour, r))

#define ADD_PIECE_IMPL(sym) { reinterpret_cast<const void *>(sym), reinterpret_cast<std::size_t>(CONCAT(sym, _size)) },
#define ADD_PIECE(cpiece) ADD_PIECE_IMPL(CONCAT(piece_, cpiece))

#define ADD_PIECES(colour)       \
    ADD_PIECE(CONCAT(colour, b)) \
    ADD_PIECE(CONCAT(colour, k)) \
    ADD_PIECE(CONCAT(colour, n)) \
    ADD_PIECE(CONCAT(colour, p)) \
    ADD_PIECE(CONCAT(colour, q)) \
    ADD_PIECE(CONCAT(colour, r))

IMPORT_PIECES(w)
IMPORT_PIECES(b)

IMPORT_BIN(DATA_FONT, font_data)
IMPORT_BIN(DATA_KNOOK, knook_data)

IMPORT_BIN(DATA_MOVE, move_data)
IMPORT_BIN(DATA_CAPTURE, capture_data)

namespace chess
{
    std::array<std::pair<const void *, std::size_t>, 12> piece_datas
    {
        { ADD_PIECES(w) ADD_PIECES(b) }
    };

    std::pair<const void *, std::size_t> get_knook_data()
    {
        return { reinterpret_cast<const void *>(knook_data), reinterpret_cast<std::size_t>(knook_data_size) };
    }

    std::pair<const void *, std::size_t> get_font_data()
    {
        return { reinterpret_cast<const void *>(font_data), reinterpret_cast<std::size_t>(font_data_size) };
    }

    std::pair<const void *, std::size_t> get_move_data()
    {
        return { reinterpret_cast<const void *>(move_data), reinterpret_cast<std::size_t>(move_data_size) };
    }

    std::pair<const void *, std::size_t> get_capture_data()
    {
        return { reinterpret_cast<const void *>(capture_data), reinterpret_cast<std::size_t>(capture_data_size) };
    }
} // namespace chess