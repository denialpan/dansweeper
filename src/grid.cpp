#include "headers/grid.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// extremely messy grid seed generation handling
#include "headers/utils/gridutils.h"

static const char b64_table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/";

// grid initialization
Grid::Grid(GridMetadata& metadata, const std::string& seed32, bool useSeed) {
    // generate 16 character seed from manual input
    if (!useSeed) {
        this->width = metadata.width;
        this->height = metadata.height;
        this->numMine = metadata.numMine;
        this->firstClick = true;

        this->cells.resize(this->height, std::vector<Cell>(this->width));

    } else {
        GridMetadata decodedMetadata = gridutils::decodeSeed(seed32);
        this->width = decodedMetadata.width;
        this->height = decodedMetadata.height;
        this->numMine = decodedMetadata.numMine;
        this->prngSeed = decodedMetadata.prngSeed;
        this->safeX = decodedMetadata.safeX;
        this->safeY = decodedMetadata.safeY;
        this->seed32 = seed32;

        this->cells.resize(this->height, std::vector<Cell>(this->width));
        Grid::generateBoard();
    }
}

void Grid::generateBoard() {
    // Reset all cells
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            cells[y][x].content = CELL_EMPTY;
            cells[y][x].renderTile = TILE_BLANK;
        }
    }

    // Generate list of all possible cell indices excluding the safe cell
    std::vector<std::pair<int, int>> validCells;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (x == safeX && y == safeY) continue;
            validCells.emplace_back(x, y);
        }
    }

    // Shuffle valid cells using the deterministic prngSeed
    std::mt19937_64 gen(prngSeed);
    std::shuffle(validCells.begin(), validCells.end(), gen);

    // Place mines in the first `numMine` shuffled cells
    for (int i = 0; i < numMine && i < static_cast<int>(validCells.size()); ++i) {
        int x = validCells[i].first;
        int y = validCells[i].second;
        cells[y][x].content = CELL_MINE;
        cells[y][x].renderTile = TILE_MINE_REVEALED;
    }
}

void Grid::reveal(int startX, int startY) {
    // BFS implementation to floodfill
    // this is separate from determining next tile to click

    // out of bounds somehow
    if (startX < 0 || startX >= width || startY < 0 || startY >= height) {
        return;
    }

    // clicked on mine
    if (cells[startY][startX].content == CELL_MINE) {
        gameState = GameState::LOST;
        return;
    };

    std::queue<std::pair<int, int>> toReveal;
    toReveal.push({startX, startY});

    while (!toReveal.empty()) {
        auto [x, y] = toReveal.front();
        toReveal.pop();

        if (x < 0 || x >= width || y < 0 || y >= height)
            continue;

        Cell& cell = cells[y][x];
        if (cell.revealed || cell.flagged)
            continue;

        cell.revealed = true;

        int count = countAdjacentMines(x, y);
        if (count == 0) {
            cell.renderTile = TILE_REVEALED;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx != 0 || dy != 0)
                        toReveal.push({x + dx, y + dy});
                }
            }
        } else {
            cell.renderTile = static_cast<TileId>(TILE_1 + (count - 1));
        }
    }

    if (checkWinCondition()) {
        gameState = GameState::WON;
    }
}

void Grid::flag(int x, int y) {
    if (cells[y][x].revealed == false) {
        cells[y][x].flagged = (cells[y][x].flagged == true) ? false : true;
        cells[y][x].renderTile = (cells[y][x].flagged == true) ? TILE_FLAG : TILE_BLANK;
    }
}

int Grid::countAdjacentMines(int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (cells[ny][nx].content == CELL_MINE)
                    ++count;
            }
        }
    }
    return count;
}

bool Grid::checkWinCondition() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Cell& cell = cells[y][x];
            if (cell.content == CELL_EMPTY &&
                (cell.renderTile == TILE_BLANK || cell.renderTile == TILE_FLAG || cell.renderTile == TILE_QUESTION)) {
                return false;  // still unrevealed non-mine cell
            }
        }
    }
    return true;
}

std::string Grid::getSeed32() const {
    return this->seed32;
}