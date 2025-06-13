#include "headers/grid.h"

#include <algorithm>
#include <array>
#include <ctime>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
// extremely messy grid seed generation handling

static const char b64_table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/";

// grid initialization
Grid::Grid(GridMetadata& metadata, const std::string& seed16, bool useSeed) {
    // generate 16 character seed from manual input
    if (!useSeed) {
        this->width = metadata.width;
        this->height = metadata.height;
        this->numMine = metadata.numMine;
        this->firstClick = true;

        std::random_device rd;
        std::mt19937_64 gen(rd());
        this->prngSeed = gen();

        this->cells.resize(this->height, std::vector<Cell>(this->width));
    } else {
        this->width = decoded.width;
        this->height = decoded.height;
        this->numMine = decoded.numMine;
        this->prngSeed = decoded.prngSeed;
        this->safeX = decoded.safeX;
        this->safeY = decoded.safeY;
        this->firstClick = false;
        this->seed32 = seed16;
        this->cells.resize(this->height, std::vector<Cell>(this->width));
        generateBoard();
        std::cout << "Decoded width: " << +width << "\n";
        std::cout << "Decoded height: " << +height << "\n";
    }
}

void Grid::generateBoard() {
    int totalCells = width * height;
    int safeIndex = safeY * width + safeX;

    // Safety check: numMine must be totalCells - 1 or fewer
    if (numMine > totalCells - 1) {
        throw std::runtime_error("Too many mines! At least one cell must be safe.");
    }

    std::vector<int> positions;
    positions.reserve(totalCells - 1);

    // Step 1: Add all cell indices, except the safe cell
    for (int i = 0; i < totalCells; ++i) {
        if (i != safeIndex) positions.push_back(i);
    }

    // Step 2: Shuffle positions
    std::mt19937 rng(static_cast<unsigned int>(prngSeed));
    std::shuffle(positions.begin(), positions.end(), rng);

    // Step 3: Mark mine cells
    for (int i = 0; i < numMine; ++i) {
        int pos = positions[i];
        int y = pos / width;
        int x = pos % width;
        cells[y][x].content = CELL_MINE;
        cells[y][x].renderTile = TILE_MINE_REVEALED;
    }

    int emptyCount = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (cells[y][x].content == CELL_EMPTY) {
                emptyCount++;
                std::cout << "Empty at: " << x << ", " << y << "\n";
            }
        }
    }
    std::cout << "Total empty: " << emptyCount << "\n";
}
std::string Grid::getSeed16() const {
    return this->seed32;
}