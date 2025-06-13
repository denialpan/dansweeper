// headers/grid.h
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "headers/tile.h"
#include "raylib.h"

enum CellContent {
    CELL_EMPTY,
    CELL_MINE
};

struct Cell {
    CellContent content = CELL_EMPTY;
    TileId renderTile = TILE_BLANK;
};

// what defines a board and its properties
struct GridMetadata {
    int width;
    int height;
    int numMine;
    int prngSeed;  // prng seed
    int safeX;     // safe first grid coords
    int safeY;
};

class Grid {
   public:
    Grid(GridMetadata& metadata, const std::string& seed16, bool useSeed);

    void generateBoard();

    int width;
    int height;
    int numMine;
    int prngSeed = 0;
    bool firstClick = false;
    int safeX = 0;
    int safeY = 0;
    std::string seed32;
    std::vector<std::vector<Cell>> cells;
    std::string getSeed16() const;
};
