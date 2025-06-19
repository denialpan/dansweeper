#pragma once

const int TILE_TEXTURE_PIXEL_SIZE = 16;

enum TileId {
    TILE_1 = 0,
    TILE_2 = 1,
    TILE_3 = 2,
    TILE_4 = 3,
    TILE_5 = 4,
    TILE_6 = 5,
    TILE_7 = 6,
    TILE_8 = 7,
    TILE_REVEALED = 8,
    TILE_BLANK = 9,
    TILE_FLAG = 10,
    TILE_MINE_WRONG = 11,
    TILE_QUESTION_REVEALED = 12,
    TILE_QUESTION = 13,
    TILE_MINE_REVEALED = 14,
    TILE_MINE_HIT = 15
};

enum BorderTileId {
    BORDER_TL = 0,  // (0, 0)
    BORDER_T = 1,   // (1, 0)
    BORDER_TR = 2,  // (2, 0)

    BORDER_L = 4,  // (0, 1)
    BORDER_C = 5,  // (1, 1) optional fill
    BORDER_R = 6,  // (2, 1)

    BORDER_BL = 8,  // (0, 2)
    BORDER_B = 9,   // (1, 2)
    BORDER_BR = 10  // (2, 2)
};
