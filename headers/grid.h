// headers/grid.h
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "headers/tile.h"

enum CellContent {
    CELL_EMPTY,
    CELL_MINE
};

struct Cell {
    CellContent content;
    TileId renderTile;
};

// what defines a board and its properties
struct GridMetadata {
    int width;
    int height;
    int numMine;
    int prngSeed;  // prng seed
};

class Grid {
   public:
    Grid(GridMetadata& metadata, const std::string& seed16, bool useSeed);

    int width;
    int height;
    int numMine;
    int prngSeed;
    std::string seed16;
    std::vector<std::vector<Cell>> cells;
};

std::string createBase64Seed(uint8_t width, uint8_t height, uint16_t numMine);
GridMetadata decodeBase64Seed(const std::string& seed);