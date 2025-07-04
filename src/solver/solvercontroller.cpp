
#include "headers/solver/solvercontroller.h"

#include "headers/grid.h"
#include "headers/solver/algorithms/BFS_with_flag_csp.h"

SolverController::SolverController() {};
SolverController::~SolverController() {
    delete this->currentSolver;
}

void SolverController::start(Grid* grid, SolverType type, int startX, int startY) {
    if (currentSolver != nullptr) {
        delete currentSolver;
        currentSolver = nullptr;
    }

    this->grid = grid;
    this->currentType = type;

    switch (type) {
        case SolverType::BFS_WITH_FLAG_CSP:
            currentSolver = new BFSFlagCSPSolver(grid, startX, startY);
            break;
        default:
            currentSolver = nullptr;
            break;
    }
}

void SolverController::step() {
    if (currentSolver && !currentSolver->isFinished()) {
        currentSolver->step();
    }
}

bool SolverController::isFinished() const {
    return currentSolver ? currentSolver->isFinished() : true;
}
