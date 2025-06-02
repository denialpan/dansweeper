#ifndef INPUT_H
#define INPUT_H

#include "headers/grid.h"

namespace input {
struct HoveredTile {
    bool valid;
    int x;
    int y;
};
void HandleInput(Grid* grid);
HoveredTile GetHoveredTile();

}  // namespace input

#endif  // INPUT_H
