// headers/grid.h
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "headers/tile.h"
#include "raylib.h"

enum CellContent {
    CELL_EMPTY,
    CELL_MINE
};

enum class GameState {
    ONGOING,
    WON,
    LOST
};

struct Cell {
    CellContent content = CELL_EMPTY;
    TileId renderTile = TILE_BLANK;
    bool revealed = false;
    bool flagged = false;
    int adjacentMines = 0;
};

// what defines a board and its properties
struct GridMetadata {
    int width;
    int height;
    int numMine;
    int prngSeed;  // prng seed
    int safeX;     // safe first grid coords
    int safeY;
};

struct GridEndStats {
    float timeElapsed = 0.0f;
    int numFlagged = 0.0f;
    int numRevealed = 0.0f;
    int bombsLeft;
    int width = 0.0f;
    int height = 0.0f;
    std::string seed32;
};

class Grid {
   public:
    Grid(GridMetadata& metadata, const std::string& seed32, bool useSeed);
    GameState gameState = GameState::ONGOING;

    void generateBoard();
    int countAdjacentMines(int x, int y);
    bool checkWinCondition();
    bool validateCellInBounds(int x, int y);

    // user interactions and allowed solver interactions
    // see solvercontroller.cpp for arbitrary "rules"
    void reveal(int x, int y);
    void chord(int x, int y);
    void flag(int x, int y);
    Cell getCellProperties(int x, int y);
    int getGridWidth();
    int getGridHeight();

    double startTime = 0.0f;
    float timeElapsed = 0.0f;
    bool timerRunning = false;
    void updateTimer();

    int width;
    int height;
    int numMine;
    int prngSeed = 0;
    bool firstClick = false;
    int safeX = -1;
    int safeY = -1;
    bool useSeed;
    std::string seed32;
    std::vector<std::vector<Cell>> cells;
    std::string getSeed32() const;
    GridEndStats endStats;
};