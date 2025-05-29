#include "headers/grid.h"

#include <algorithm>
#include <functional>
#include <random>

Grid::Grid(const GridSettings& settings) : config(settings), generated(false) {
    Allocate();
}

void Grid::Allocate() {
    cells = new Cell*[config.height];
    for (int i = 0; i < config.height; i++) {
        cells[i] = new Cell[config.width];
        for (int j = 0; j < config.width; j++) {
            cells[i][j] = {CELL_EMPTY, RENDER_BLANK, 0};
        }
    }
}

void Grid::Free() {
    for (int i = 0; i < config.height; i++)
        delete[] cells[i];
    delete[] cells;
}

Grid::~Grid() { Free(); }

Cell** Grid::GetCells() { return cells; }
int Grid::GetWidth() const { return config.width; }
int Grid::GetHeight() const { return config.height; }

unsigned int Grid::HashSeedWithClick(int x, int y) const {
    std::hash<std::string> hasher;
    return static_cast<unsigned int>(
        hasher(config.seed + "_" + std::to_string(x) + "_" + std::to_string(y)));
}

void Grid::Generate(int firstClickX, int firstClickY) {
    if (generated) return;
    unsigned int seed = HashSeedWithClick(firstClickX, firstClickY);
    PlaceBombs(firstClickX, firstClickY, seed);
    CalculateBombNumbers();
    generated = true;
}

void Grid::PlaceBombs(int safeX, int safeY, unsigned int rngSeed) {
    std::vector<std::pair<int, int>> available;
    for (int y = 0; y < config.height; y++) {
        for (int x = 0; x < config.width; x++) {
            if (x == safeX && y == safeY) continue;
            available.emplace_back(x, y);
        }
    }

    std::mt19937 rng(rngSeed);
    std::shuffle(available.begin(), available.end(), rng);

    for (int i = 0; i < config.numBomb && i < available.size(); i++) {
        auto [x, y] = available[i];
        cells[y][x].content = CELL_BOMB;
    }
}

void Grid::CalculateBombNumbers() {
    for (int y = 0; y < config.height; y++) {
        for (int x = 0; x < config.width; x++) {
            if (cells[y][x].content == CELL_BOMB) continue;

            int count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < config.width && ny >= 0 && ny < config.height &&
                        cells[ny][nx].content == CELL_BOMB) {
                        count++;
                    }
                }
            }

            cells[y][x].bombsAround = count;
        }
    }
}
