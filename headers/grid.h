// headers/grid.h
#pragma once
#include "headers/tile.h"

enum CellContent {
    CELL_EMPTY,
    CELL_BOMB
};

struct Cell {
    CellContent content;
    TileId renderTile;
};

struct Grid {
    int width, height;
    Cell** cells;

    Grid(int w, int h);
    ~Grid();
};
