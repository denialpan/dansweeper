#include <iostream>
#include <iomanip>
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

using namespace std;

enum GameState
{
    MENU,
    GAME,
};

enum CellContent
{
    CELL_EMPTY,
    CELL_BOMB,
};

enum CellRender
{
    RENDER_NUMBER_1,          // 0
    RENDER_NUMBER_2,          // 1
    RENDER_NUMBER_3,          // 2
    RENDER_NUMBER_4,          // 3
    RENDER_NUMBER_5,          // 4
    RENDER_NUMBER_6,          // 5
    RENDER_NUMBER_7,          // 6
    RENDER_NUMBER_8,          // 7
    RENDER_EMPTY_REVEALED,    // 8
    RENDER_EMPTY_HIDDEN,      // 9
    RENDER_FLAG,              // 10
    RENDER_BOMB_WRONG,        // 11 (flagged incorrectly)
    RENDER_QUESTION_REVEALED, // 12
    RENDER_QUESTION_HIDDEN,   // 13
    RENDER_BOMB_NORMAL,       // 14
    RENDER_BOMB_HIT           // 15
};

struct Cell
{
    CellContent content; // What’s in the cell (empty or bomb)
    CellRender render;   // What to draw
    int bombsAround;     // Used for numbered cells
    bool revealed;       // Has the player revealed this cell?
    bool flagged;        // Has the player flagged this cell?
};

const int TEXTUREMAP_TILE_SIZE = 16;

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "dansweeper");
    SetTargetFPS(120);
    GuiLoadStyle("styles/default/style_default.rgs");

    GameState state = MENU;

    const int GRID_CELL_SIZE = 64;
    const int GRID_ROWS = 9;
    const int GRID_COLS = 9;
    Cell grid[GRID_ROWS][GRID_COLS];

    for (int y = 0; y < GRID_ROWS; y++)
    {
        for (int x = 0; x < GRID_COLS; x++)
        {
            grid[y][x] = {
                .content = CELL_EMPTY,
                .render = RENDER_EMPTY_HIDDEN,
                .bombsAround = 0,
                .revealed = false,
                .flagged = false};
        }
    }

    double startTime = GetTime();

    const Rectangle TILE_RECTS[] = {
        {0 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 1
        {1 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 2
        {2 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 3
        {3 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 4
        {0 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 5
        {1 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 6
        {2 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 7
        {3 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // 8
        {0 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // blank revealed
        {1 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // blank unrevealed
        {2 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // flag
        {3 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // bomb wrong
        {0 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // ? revealed
        {1 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // ? unrevealed
        {2 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // bomb normal
        {3 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // bomb hit
    };

    // Load texture atlas
    Texture2D tileAtlas = LoadTexture("resources/texturemap.png");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        if (state == MENU)
        {
            if (GuiButton((Rectangle){300, 250, 200, 50}, "START GAME"))
            {
                state = GAME;
                startTime = GetTime();
            }
        }
        else if (state == GAME)
        {
            for (int y = 0; y < GRID_ROWS; y++)
            {
                for (int x = 0; x < GRID_COLS; x++)
                {
                    Rectangle dest = {
                        x * GRID_CELL_SIZE,
                        y * GRID_CELL_SIZE,
                        GRID_CELL_SIZE,
                        GRID_CELL_SIZE};

                    // Get mouse state
                    Vector2 mousePos = GetMousePosition();
                    bool hovered = CheckCollisionPointRec(mousePos, dest);

                    // Handle left click (reveal)
                    if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !grid[y][x].flagged && !grid[y][x].revealed)
                    {
                        grid[y][x].revealed = true;

                        // Update render type based on bomb count
                        if (grid[y][x].content == CELL_BOMB)
                        {
                            grid[y][x].render = RENDER_BOMB_HIT;
                        }
                        else if (grid[y][x].bombsAround == 0)
                        {
                            grid[y][x].render = RENDER_EMPTY_REVEALED;
                        }
                        else
                        {
                            grid[y][x].render = (CellRender)(RENDER_NUMBER_1 + grid[y][x].bombsAround - 1);
                        }

                        std::cout << "Clicked cell (" << x << ", " << y << ") at "
                                  << std::fixed << std::setprecision(3)
                                  << (GetTime() - startTime) << " seconds" << std::endl;
                    }

                    // Handle right click (flag toggle)
                    if (hovered && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !grid[y][x].revealed)
                    {
                        grid[y][x].flagged = !grid[y][x].flagged;
                        grid[y][x].render = grid[y][x].flagged ? RENDER_FLAG : RENDER_EMPTY_HIDDEN;

                        std::cout << (grid[y][x].flagged ? "Flagged" : "Unflagged")
                                  << " cell (" << x << ", " << y << ") at "
                                  << std::fixed << std::setprecision(3)
                                  << (GetTime() - startTime) << " seconds" << std::endl;
                    }

                    // Draw the cell based on its current render type
                    DrawTexturePro(tileAtlas, TILE_RECTS[grid[y][x].render], dest, (Vector2){0, 0}, 0, WHITE);
                }
            }
        }

        int fps = GetFPS();
        const char *fpsText = TextFormat("%d FPS", fps);
        int fpsTextWidth = MeasureText(fpsText, 20);
        DrawText(fpsText, GetScreenWidth() - fpsTextWidth - 10, 10, 20, LIME);

        double elapsed = GetTime() - startTime;
        const char *timeText = TextFormat("Time: %.3f", elapsed);
        int timeTextWidth = MeasureText(timeText, 20);

        DrawText(timeText, GetScreenWidth() - timeTextWidth - 10, GetScreenHeight() - 30, 20, GREEN);

        EndDrawing();
    }

    UnloadTexture(tileAtlas);
    CloseWindow();
    return 0;
}
