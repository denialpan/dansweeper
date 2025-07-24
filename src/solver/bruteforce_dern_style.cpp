#include <algorithm>
#include <ctime>
#include <format>
#include <iostream>
#include <queue>
#include <random>
#include <utility>

#include "headers/grid.h"
#include "headers/solver/algorithms/brute_force_dern_style.h"

BruteForceDernStyle::BruteForceDernStyle(Grid* grid)
    : grid(grid) {
    // hacky cheat way to reset queue
    std::set<std::pair<int, int>> resetSet;
    std::swap(revealedNumberTiles, resetSet);

    grid->reveal(grid->width / 2, grid->height / 2);
}

void BruteForceDernStyle::step() {
    if (grid->gameState != GameState::ONGOING) {
        finished = true;
        return;
    }

    if (finished == false) {
        bool chordOrFlagged = false;

        // heuristic values if needed
        float heuristicRatio = 1.0f;
        std::pair<int, int> heuristicPair = {-1, -1};

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
            int bothFlaggedAndUnrevealed = 0;

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
                    std::cout << "this tile is unrevealed and not flagged " << neighborX << ", " << neighborY << "\n";
                    unrevealedNeighbors++;
                }
            }

            std::cout << std::format("flagged: {}, revealed: {} \n", flaggedNeighbors, unrevealedNeighbors);
            std::cout << std::format("revealed adjacency: {} \n", cellRevealedProperties.adjacentMines);

            if (flaggedNeighbors == cellRevealedProperties.adjacentMines) {
                // unnecessary chord again if all thats around is flagged == unrevealed
                if (unrevealedNeighbors != flaggedNeighbors) {
                    grid->chord(x, y);
                    chordOrFlagged = true;
                    std::cout << "chorded \n";
                }
            }

            if (cellRevealedProperties.flagged && !cellRevealedProperties.revealed) {
                bothFlaggedAndUnrevealed++;
            }

            if (unrevealedNeighbors == cellRevealedProperties.adjacentMines) {
                for (std::pair<int, int> neighbor : neighbors) {
                    auto [flagX, flagY] = neighbor;
                    Cell& cellNeighborProperties = grid->cells[flagY][flagX];
                    if (cellNeighborProperties.flagged == false && cellNeighborProperties.revealed == false) {
                        grid->flag(flagX, flagY);
                        chordOrFlagged = true;
                    }
                }
            }

            if (!chordOrFlagged) {
                float possibleMine = cellRevealedProperties.adjacentMines - bothFlaggedAndUnrevealed;
                if (possibleMine > 0 && unrevealedNeighbors > 0 && possibleMine / unrevealedNeighbors < heuristicRatio) {
                    heuristicRatio = possibleMine / unrevealedNeighbors;

                    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
                    std::uniform_int_distribution<size_t> dist(0, neighbors.size() - 1);

                    // randomly choose so terribly
                    while (true) {
                        heuristicPair = neighbors[dist(rng)];
                        auto [heuristicX, heuristicY] = heuristicPair;
                        Cell& cellHeuristicProperties = grid->cells[heuristicY][heuristicX];

                        if (cellHeuristicProperties.revealed == false && cellHeuristicProperties.flagged == false) {
                            heuristicPair = {heuristicX, heuristicY};
                            break;
                        }
                    }
                }
            }
        }

        // fallback heuristic
        // choose best tile given lowest ratio of possible mines left / unrevealed neighbors
        if (!chordOrFlagged) {
            auto [x, y] = heuristicPair;
            grid->reveal(x, y);
            std::cout << std::format("guess reveal with a ratio of {}: {}, {}", heuristicRatio, x, y);
        }

    } else {
        std::cout << "ended game";
    }
}

bool BruteForceDernStyle::isFinished() const {
    return finished;
}

std::vector<std::pair<int, int>> BruteForceDernStyle::getNeighbors(int x, int y) {
    std::vector<std::pair<int, int>> neighbors;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            if (dx != 0 || dy != 0)

                // if out of bounds neighbor
                if (!(x + dx < 0 || x + dx >= grid->width || y + dy < 0 || y + dy >= grid->height))
                    neighbors.emplace_back(x + dx, y + dy);

    return neighbors;
}
