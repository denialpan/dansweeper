#include "headers/solver/algorithms/BFS_with_flag_csp.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

#include "headers/grid.h"

BFSFlagCSPSolver::BFSFlagCSPSolver(Grid* grid, int startX, int startY)
    : grid(grid) {
    if (!grid || grid->cells[startY][startX].content == CELL_MINE) {
        finished = true;
        return;
    }

    // hacky cheat way to empty reset queue
    std::queue<std::pair<int, int>> resetQueue;
    std::set<std::pair<int, int>> resetQueue1;

    std::swap(queue, resetQueue);
    std::swap(revealedNumberTiles, resetQueue1);
    queue.push({startX, startY});
}

void BFSFlagCSPSolver::step() {
    if (grid->gameState != GameState::ONGOING) {
        finished = true;
        return;
    }

    if (!queue.empty()) {
        auto [x, y] = queue.front();
        grid->reveal(x, y);
        queue.pop();

        // get all revealed NUMBER tiles
        for (int i = 0; i < grid->height; i++) {
            for (int j = 0; j < grid->width; j++) {
                Cell& cellProperties = grid->cells[i][j];
                if (cellProperties.revealed && cellProperties.adjacentMines > 0) {
                    revealedNumberTiles.insert({j, i});
                    std::cout << "revealed number tile: " << j << ", " << i << "\n";
                }
            }
        }
        std::cout << "revealed tile: " << x << " " << y;

        // push first occurrence of 100% safe tile
        for (std::pair<int, int> revealedNumberTile : revealedNumberTiles) {
            int flaggedNeighbors = 0;
            int unrevealedNeighbors = 0;

            auto [revealedX, revealedY] = revealedNumberTile;
            Cell& revealedNumberTileProperties = grid->cells[revealedY][revealedX];
            Cell& neighborCellProperties = grid->cells[0][0];
            std::vector<std::pair<int, int>> neighbors = getNeighbors(revealedX, revealedY);
            int neighborX = -1;
            int neighborY = -1;

            for (std::pair<int, int> neighbor : neighbors) {
                neighborX = neighbor.first;
                neighborY = neighbor.second;

                if (neighborX < 0 || neighborX > grid->width - 1 || neighborY < 0 || neighborY > grid->height - 1) {
                    continue;
                }

                if (neighborCellProperties.revealed) {
                    continue;
                }

                if (neighborCellProperties.flagged) {
                    flaggedNeighbors++;
                }

                if (!neighborCellProperties.revealed) {
                    unrevealedNeighbors++;
                }
            }

            if (unrevealedNeighbors == revealedNumberTileProperties.adjacentMines) {
                for (std::pair<int, int> neighbor : neighbors) {
                    auto [x, y] = neighbor;
                    neighborCellProperties = grid->cells[y][x];
                    if (!neighborCellProperties.flagged) {
                        grid->flag(x, y);
                    }
                }
            }

            // yeah you're going to jail for making this for loop again
            flaggedNeighbors = 0;
            unrevealedNeighbors = 0;
            for (std::pair<int, int> neighbor : neighbors) {
                neighborX = neighbor.first;
                neighborY = neighbor.second;

                if (neighborX < 0 || neighborX > grid->width - 1 || neighborY < 0 || neighborY > grid->height - 1) {
                    continue;
                }

                Cell& neighborCellProperties = grid->cells[neighborY][neighborX];

                if (neighborCellProperties.revealed) {
                    continue;
                }

                if (neighborCellProperties.flagged) {
                    flaggedNeighbors++;
                }

                if (!neighborCellProperties.revealed) {
                    unrevealedNeighbors++;
                }
            }

            if (flaggedNeighbors == revealedNumberTileProperties.adjacentMines) {
                for (std::pair<int, int> neighbor : neighbors) {
                    queue.push(neighbor);
                }
            }
        }

    } else {
        std::cout << "drew ahh empty queue";
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
                neighbors.emplace_back(x + dx, y + dy);
    return neighbors;
}
