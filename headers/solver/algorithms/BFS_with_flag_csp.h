#pragma once
#include <queue>
#include <set>
#include <utility>
#include <vector>

#include "headers/grid.h"
#include "headers/solver/solverinterface.h"

class BFSFlagCSPSolver : public SolverStepInterface {
   public:
    BFSFlagCSPSolver(Grid* grid, int startX, int startY);

    void step() override;
    bool isFinished() const override;

   private:
    Grid* grid;
    bool finished = false;

    std::queue<std::pair<int, int>> queue;
    std::set<std::pair<int, int>> visited;
    std::set<std::pair<int, int>> revealedNumberTiles;

    void processCurrentTile(int x, int y);
    void performFallback();
    std::vector<std::pair<int, int>> getNeighbors(int x, int y);
};
