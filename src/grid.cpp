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

#include "headers/globals.h"
#include "raylib.h"

// extremely messy grid seed generation handling
#include "headers/utils/gridutils.h"

// grid initialization
Grid::Grid(GridMetadata& metadata, const std::string& seed32, bool useSeed) {
    this->useSeed = useSeed;
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
        this->firstClick = true;

        this->cells.resize(this->height, std::vector<Cell>(this->width));
        Grid::generateBoard();
    }
}

void Grid::generateBoard() {
    // Reset all cells
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            cells[y][x].content = CELL_EMPTY;
            cells[y][x].renderTile = TILE_BLANK;
            cells[y][x].revealed = false;
            cells[y][x].flagged = false;
            cells[y][x].adjacentMines = 0;
        }

    // Generate list of all valid cells excluding the safe cell
    std::vector<std::pair<int, int>> validCells;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            if (!(x == safeX && y == safeY))
                validCells.emplace_back(x, y);

    std::mt19937_64 gen(prngSeed);
    std::shuffle(validCells.begin(), validCells.end(), gen);

    for (int i = 0; i < numMine && i < static_cast<int>(validCells.size()); ++i) {
        int x = validCells[i].first;
        int y = validCells[i].second;
        cells[y][x].content = CELL_MINE;
    }

    // Compute adjacent mine counts
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (cells[y][x].content == CELL_MINE)
                continue;

            int count = 0;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
                        cells[ny][nx].content == CELL_MINE)
                        ++count;
                }

            cells[y][x].adjacentMines = count;
        }
    }
}

void Grid::reveal(int startX, int startY) {
    // big first click edge case check condition
    // effects how the board is generated

    if (this->firstClick) {
        if (!this->useSeed) {
            // generate prngseed on click
            auto now = std::chrono::high_resolution_clock::now();
            auto timeNs = now.time_since_epoch().count();

            std::ostringstream saltStream;
            saltStream << std::hex << timeNs;

            std::string salt = saltStream.str();

            std::string fullKey = std::to_string(this->width) + "x" + std::to_string(this->height) +
                                  ":" + std::to_string(this->numMine) +
                                  ":" + std::to_string(this->safeX) + "," + std::to_string(this->safeY) +
                                  ":" + salt;

            std::hash<std::string> hasher;
            this->prngSeed = hasher(fullKey);
            this->safeX = startX;
            this->safeY = startY;
            this->seed32 = gridutils::createSeedFromManualInput(this->width, this->height, this->numMine, this->safeX, this->safeY, this->prngSeed);
            this->generateBoard();
        }
        this->firstClick = false;
        this->startTime = GetTime();
        this->timerRunning = true;
    }

    // bfs implementation of floodfill
    // not related to solver of determining next tile to reveal
    if (startX < 0 || startX >= width || startY < 0 || startY >= height)
        return;

    Cell& firstCell = cells[startY][startX];

    if (firstCell.revealed || firstCell.flagged) {
        return void();
    }

    if (firstCell.content == CELL_MINE) {
        gameState = GameState::LOST;
        int remainingMines = 0;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Cell& cell = cells[y][x];
                if (cell.content == CELL_MINE) {
                    if (cell.flagged) {
                        continue;
                    } else if (!cell.flagged) {
                        remainingMines++;
                    }

                    if (!cell.revealed) {
                        cell.revealed = true;
                        cell.renderTile = TILE_MINE_REVEALED;
                    }
                } else if (cell.flagged) {
                    cell.renderTile = TILE_MINE_WRONG;
                }
            }
        }
        cells[startY][startX].renderTile = TILE_MINE_HIT;

        this->endStats.bombsLeft = remainingMines;
        this->endStats.timeElapsed = this->timeElapsed;
        this->endStats.height = this->height;
        this->endStats.width = this->width;
        this->endStats.seed32 = this->seed32;

        return;
    }

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

        if (cell.adjacentMines == 0) {
            cell.renderTile = TILE_REVEALED;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    if (dx != 0 || dy != 0)
                        toReveal.push({x + dx, y + dy});
        } else {
            cell.renderTile = static_cast<TileId>(TILE_1 + (cell.adjacentMines - 1));
        }
    }

    this->endStats.numRevealed++;

    if (checkWinCondition()) {
        gameState = GameState::WON;
        this->endStats.bombsLeft = 0;
        this->endStats.timeElapsed = this->timeElapsed;
        this->endStats.height = this->height;
        this->endStats.width = this->width;
        this->endStats.seed32 = this->seed32;
    }
}

void Grid::chord(int x, int y) {
    if (!cells[y][x].revealed || cells[y][x].adjacentMines == 0)
        return;

    int flagCount = 0;

    // Count flags around
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = x + dx;
            int ny = y + dy;
            if (dx == 0 && dy == 0) continue;
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (cells[ny][nx].flagged)
                    flagCount++;
            }
        }
    }

    if (flagCount == cells[y][x].adjacentMines) {
        // Reveal surrounding cells that are not flagged
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int nx = x + dx;
                int ny = y + dy;
                if (dx == 0 && dy == 0) continue;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    Cell& neighbor = cells[ny][nx];
                    if (!neighbor.flagged && !neighbor.revealed) {
                        reveal(nx, ny);
                    }
                }
            }
        }
    }
}

void Grid::flag(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;

    if (cells[y][x].revealed == false) {
        cells[y][x].flagged = (cells[y][x].flagged == true) ? false : true;
        cells[y][x].renderTile = (cells[y][x].flagged == true) ? TILE_FLAG : TILE_BLANK;
        this->endStats.numFlagged++;
    }
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

void Grid::updateTimer() {
    if (!this->timerRunning || this->gameState != GameState::ONGOING) {
        return;
    }

    switch (windowState) {
        case WindowState::GAME: {
            timeElapsed = static_cast<float>(GetTime() - startTime);
            break;
        }

        case WindowState::PAUSE: {
            startTime = static_cast<float>(GetTime() - timeElapsed);
            break;
        }

        default:
            break;
    }
}

std::string Grid::getSeed32() const {
    return this->seed32;
}