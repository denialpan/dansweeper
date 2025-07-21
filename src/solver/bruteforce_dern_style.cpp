#include <algorithm>
#include <format>
#include <iostream>
#include <queue>
#include <utility>

#include "headers/grid.h"
#include "headers/solver/algorithms/brute_force_dern_style.h"

BFSFlagCSPSolver::BFSFlagCSPSolver(Grid* grid, int startX, int startY)
    : grid(grid) {
    if (!grid || grid->cells[startY][startX].content == CELL_MINE) {
        finished = true;
        return;
    }

    // hacky cheat way to reset queue
    std::set<std::pair<int, int>> resetSet;
    std::swap(revealedNumberTiles, resetSet);

    grid->reveal(startX, startY);
}

void BFSFlagCSPSolver::step() {
    if (grid->gameState != GameState::ONGOING) {
        finished = true;
        return;
    }

    if (finished == false) {
        // get all revealed NUMBER tiles
        for (int i = 0; i < grid->height; i++) {
            for (int j = 0; j < grid->width; j++) {
                Cell& firstPass = grid->cells[i][j];
                if (firstPass.revealed && firstPass.adjacentMines > 0 && revealedNumberTiles.find({j, i}) == revealedNumberTiles.end()) {
                    revealedNumberTiles.insert({j, i});
                    std::cout << "revealed number tile: " << j << ", " << i << "\n";
                }
            }
        }

        for (std::pair<int, int> revealedNumberTile : revealedNumberTiles) {
            auto [x, y] = revealedNumberTile;
            std::vector<std::pair<int, int>> neighbors = getNeighbors(x, y);
            Cell& cellRevealedProperties = grid->cells[y][x];

            int unrevealedNeighbors = 0;
            int flaggedNeighbors = 0;

            for (std::pair<int, int> neighbor : neighbors) {
                auto [neighborX, neighborY] = neighbor;
                std::cout << "checking revealed number tile: " << neighborX << ", " << neighborY << "\n";
                Cell& cellNeighborProperties = grid->cells[neighborY][neighborX];

                if (cellNeighborProperties.revealed == true) {
                    std::cout << "skipped " << neighborX << ", " << neighborY << "\n";
                    continue;
                }

                if (cellNeighborProperties.flagged == true) {
                    std::cout << "flagged " << neighborX << ", " << neighborY << "\n";
                    flaggedNeighbors++;
                }

                if (cellNeighborProperties.revealed == false) {
                    std::cout << "this tile is unrevealed " << neighborX << ", " << neighborY << "\n";
                    unrevealedNeighbors++;
                }
            }

            std::cout << std::format("flagged: {}, revealed: {} \n", flaggedNeighbors, unrevealedNeighbors);
            std::cout << std::format("revealed adjacency: {} \n", cellRevealedProperties.adjacentMines);

            if (flaggedNeighbors == cellRevealedProperties.adjacentMines) {
                for (std::pair<int, int> neighbor : neighbors) {
                    auto [chordX, chordY] = neighbor;
                    grid->chord(chordX, chordY);
                }
            }

            if (unrevealedNeighbors == cellRevealedProperties.adjacentMines) {
                for (std::pair<int, int> neighbor : neighbors) {
                    auto [flagX, flagY] = neighbor;
                    Cell& cellNeighborProperties = grid->cells[flagY][flagX];
                    if (cellNeighborProperties.flagged == false && cellNeighborProperties.revealed == false) {
                        grid->flag(flagX, flagY);
                    }
                }
            }
        }

    } else {
        std::cout << "ended game";
    }
}

bool BFSFlagCSPSolver::isFinished() const {
    return finished;
}

std::vector<std::pair<int, int>> BFSFlagCSPSolver::getNeighbors(int x, int y) {
    std::vector<std::pair<int, int>> neighbors;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            if (dx != 0 || dy != 0)

                // if out of bounds neighbor
                if (!(x + dx < 0 || x + dx >= grid->width || y + dy < 0 || y + dy >= grid->height))
                    neighbors.emplace_back(x + dx, y + dy);

    return neighbors;
}
