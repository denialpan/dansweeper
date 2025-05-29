#include <string>

#ifndef GRID_H
#define GRID_H

enum CellRender {
    RENDER_NUMBER_1,           // 0
    RENDER_NUMBER_2,           // 1
    RENDER_NUMBER_3,           // 2
    RENDER_NUMBER_4,           // 3
    RENDER_NUMBER_5,           // 4
    RENDER_NUMBER_6,           // 5
    RENDER_NUMBER_7,           // 6
    RENDER_NUMBER_8,           // 7
    RENDER_BLANK,              // 8
    RENDER_BLANK_REVEALED,     // 9
    RENDER_FLAG,               // 10
    RENDER_BOMB_WRONG,         // 11 (flagged incorrectly)
    RENDER_QUESTION_REVEALED,  // 12
    RENDER_QUESTION,           // 13
    RENDER_BOMB,               // 14
    RENDER_BOMB_HIT,           // 15
};

enum CellContent {
    CELL_EMPTY,
    CELL_BOMB,
};

struct Cell {
    CellContent content;
    CellRender cellRender;
    bool revealed;
    bool flagged;
    int bombsAround;
};

struct GridSettings {
    int width;
    int height;
    int numBomb;
    std::string seed;
};

class Grid {
   public:
    Grid(const GridSettings& settings);
    ~Grid();  // destructor declaration

    void Generate(int firstClickX, int firstClickY);
    Cell** GetCells();
    int GetWidth() const;
    int GetHeight() const;

   private:
    Cell** cells;
    GridSettings config;
    bool generated;

    void Allocate();
    void Free();
    void PlaceBombs(int safeX, int safeY, unsigned int rngSeed);
    void CalculateBombNumbers();
    unsigned int HashSeedWithClick(int x, int y) const;
};

// TODO: config mine and grid dimensions later
extern int gridRow;
extern int gridCol;
extern int numBomb;
extern Cell** grid;

void GenerateGrid();

#endif
