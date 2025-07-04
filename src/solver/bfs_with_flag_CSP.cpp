#include "headers/solver/algorithms/BFS_with_flag_csp.h"

#include <algorithm>

#include "headers/grid.h"

BFSFlagCSPSolver::BFSFlagCSPSolver(Grid* grid, int startX, int startY)
    : grid(grid) {
    if (!grid || grid->cells[startY][startX].content == CELL_MINE) {
        finished = true;
        return;
    }

    grid->reveal(startX, startY);
    queue.push({startX, startY});
}

void BFSFlagCSPSolver::step() {
    if (grid->gameState != GameState::ONGOING) {
        finished = true;
        return;
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
