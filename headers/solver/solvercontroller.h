#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

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

    void start(Grid* grid, SolverType type);
    void step();
    bool isFinished() const;
    void stop();
    static std::vector<std::pair<SolverType, std::string>> getPairSolverString();

   private:
    SolverStepInterface* currentSolver = nullptr;
    SolverType currentType = SolverType::NONE;
    Grid* grid = nullptr;

    std::thread solverThread;
    std::atomic<bool> running = false;
    std::atomic<bool> stepRequested = false;
    std::mutex stepMutex;
    std::condition_variable stepCV;

    std::condition_variable stepDoneCV;
    bool stepDone = false;

    void solverThreadLoop();
};