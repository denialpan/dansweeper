
#include "headers/solver/solvercontroller.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "headers/grid.h"
#include "headers/solver/algorithms/brute_force_dern_style.h"

SolverController::SolverController() {};
SolverController::~SolverController() {
    stop();
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

void SolverController::start(Grid* grid, SolverType type) {
    stop();

    if (currentSolver != nullptr) {
        delete currentSolver;
        currentSolver = nullptr;
    }

    this->grid = grid;
    this->currentType = type;

    switch (type) {
        case SolverType::BRUTE_FORCE_DERN_STYLE:
            currentSolver = new BruteForceDernStyle(grid);
            break;
        default:
            currentSolver = nullptr;
            break;
    }

    running = true;
    solverThread = std::thread(&SolverController::solverThreadLoop, this);
}

void SolverController::step() {
    if (currentSolver && !currentSolver->isFinished()) {
        stepRequested = true;
        stepCV.notify_one();
    }
}

void SolverController::solverThreadLoop() {
    while (running && currentSolver && !currentSolver->isFinished()) {
        std::unique_lock<std::mutex> lock(stepMutex);
        stepCV.wait(lock, [&]() {
            return stepRequested || !running;
        });

        if (!running) {
            break;
        }

        stepRequested = false;
        currentSolver->step();
    }
}

void SolverController::stop() {
    running = false;
    stepRequested = false;
    stepCV.notify_all();

    if (solverThread.joinable()) {
        solverThread.join();
    }

    if (currentSolver) {
        delete currentSolver;
        currentSolver = nullptr;
    }
}

bool SolverController::isFinished() const {
    return currentSolver ? currentSolver->isFinished() : true;
}

std::vector<std::pair<SolverType, std::string>> SolverController::getPairSolverString() {
    std::vector<std::pair<SolverType, std::string>> output = {};

    output.push_back({SolverType::BRUTE_FORCE_DERN_STYLE, "brute force dern style"});
    output.push_back({SolverType::BRUTE_FORCE_DERN_STYLE, "sus"});

    return output;
}