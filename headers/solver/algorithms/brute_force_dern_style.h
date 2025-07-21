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

    std::set<std::pair<int, int>> revealedNumberTiles;

    std::vector<std::pair<int, int>> getNeighbors(int x, int y);
};
