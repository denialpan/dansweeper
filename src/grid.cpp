#include "headers/grid.h"

Grid::Grid(int w, int h) : width(w), height(h) {
    cells = new Cell*[height];
    for (int y = 0; y < height; ++y) {
        cells[y] = new Cell[width];
        for (int x = 0; x < width; ++x) {
            cells[y][x].content = CELL_EMPTY;
            cells[y][x].renderTile = TILE_BLANK;
        }
    }
}

Grid::~Grid() {
    for (int y = 0; y < height; ++y) {
        delete[] cells[y];
    }
    delete[] cells;
}
