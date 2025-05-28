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

enum GridState
{
    REVEALED,
    BOMB,
    FLAG,
};

enum CellContent
{
    CELL_EMPTY,
    CELL_BOMB,
};

enum CellRender
{
    TILE_HIDDEN,
    TILE_REVEALED,
    TILE_FLAG,
    TILE_BOMB
};

struct Cell
{
    CellContent content;
    CellRender render;
    int bombsAround;
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
    bool revealed[GRID_ROWS][GRID_COLS] = {false};

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
                    Rectangle dest = {x * GRID_CELL_SIZE, y * GRID_CELL_SIZE, GRID_CELL_SIZE, GRID_CELL_SIZE};

                    CellRender type = revealed[y][x] ? TILE_REVEALED : TILE_HIDDEN;
                    DrawTexturePro(tileAtlas, TILE_RECTS[type], dest, (Vector2){0, 0}, 0, WHITE);
                    // Mouse interaction
                    bool clicked = CheckCollisionPointRec(GetMousePosition(), dest) &&
                                   IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
                    if (clicked)
                    {
                        revealed[y][x] = true;
                        // Print to console
                        cout << "Clicked cell (" << x << ", " << y << ") at "
                             << std::fixed << std::setprecision(3)
                             << (GetTime() - startTime) << " seconds" << std::endl;
                    }
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
