#pragma once

#include "headers/grid.h"
#include "headers/solver/solverinterface.h"

enum class SolverType {
    NONE,
    BRUTE_FORCE_DERN_STYLE,
};

class SolverController {
   public:
    SolverController();
    ~SolverController();

    void start(Grid* grid, SolverType type, int startX, int startY);
    void step();
    bool isFinished() const;

   private:
    SolverStepInterface* currentSolver = nullptr;
    SolverType currentType = SolverType::NONE;
    Grid* grid = nullptr;
};