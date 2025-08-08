#pragma once
#include <headers/grid.h>

class SolverStepInterface {
   public:
    Grid* grid;
    int steps = 0;
    virtual ~SolverStepInterface() = default;
    virtual void step() = 0;
    virtual bool isFinished() const = 0;

    // "allowed" grid accesses methods
   protected:
    Cell getCellProperties(int x, int y) {
        if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
            return grid->cells[y][x];
        }
        return {};
    }

    int getWidth() {
        return grid->width;
    }

    int getHeight() {
        return grid->height;
    }
};