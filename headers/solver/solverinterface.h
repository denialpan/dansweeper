#pragma once

class SolverStepInterface {
   public:
    virtual ~SolverStepInterface() = default;
    virtual void step() = 0;
    virtual bool isFinished() const = 0;
};