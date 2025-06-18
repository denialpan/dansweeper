#ifndef INPUT_H
#define INPUT_H

#include "headers/grid.h"

enum PerformedAction {
    OKAY,
    INTERRUPTED,
    END_GAME
};

struct GridCoordinates {
    int x;
    int y;
};

class InputController {
   public:
    Grid* grid;
    InputController(Grid* grid);
    GridCoordinates gc;

    void handleManualInput();

   private:
    void clampCameraTarget(Camera2D& camera);
    void handlePanning(Camera2D& camera);
    void handleZoomCursor(Camera2D& camera);
    GridCoordinates handleHoverCursor(Camera2D& camera);

    void handleLeftClick();
    void handleRightClick();
};

#endif  // INPUT_H
