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
    Grid(GridMetadata& metadata, const std::string& seed32, bool useSeed);

    void generateBoard();

    int width;
    int height;
    int numMine;
    int prngSeed = 0;
    bool firstClick = false;
    int safeX = -1;
    int safeY = -1;
    std::string seed32;
    std::vector<std::vector<Cell>> cells;
    std::string getSeed32() const;
};

GridMetadata decodeSeed(const std::string& seed);
std::vector<uint8_t> decodeBase64Bytes(const std::string& encoded);
std::string encodeBase64(const std::vector<uint8_t>& data);
std::string createBase64Seed(uint16_t width, uint16_t height, uint32_t numMines, uint16_t safeX, uint16_t safeY, uint64_t prngSeed);
uint64_t hashBoardParamsWithSalt(uint16_t width, uint16_t height, uint32_t numMines, uint16_t safeX, uint16_t safeY, const std::string& salt);
std::string createSeedFromManualInput(uint16_t width, uint16_t height, uint32_t numMines, uint16_t safeX, uint16_t safeY);