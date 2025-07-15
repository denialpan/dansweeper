
#include "headers/solver/solvercontroller.h"

#include "headers/grid.h"
#include "headers/solver/algorithms/BFS_with_flag_csp.h"

SolverController::SolverController() {};
SolverController::~SolverController() {
    delete this->currentSolver;
}

/*
solver philosophy:
the solver should not be able to interact with the inner workings of the grid, i.e. modify how the board is generated and the state of the tiles or of the game

rules by the solver:
- can access game state, to determine solver status
- can access tile information, such as mine count, revealed tiles, i.e. information that a player can see
while tile information for player is visually displayed as rendered tiles, the rendering of tiles is determined solely by cell state, so it is lines of this solver philosophy
- methods to interact with the board as a "player" are allowed, such calling reveal() to reveal a tile on the grid, flag(), chord(), etc
- information cannot be gained by methods that the game has, such as the if the reveal() function returned a vector of tiles that were revealed
- all information must be retrieved by the solver itself
 */

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
