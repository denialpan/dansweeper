#include <iostream>
#include <iomanip>
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "headers/raygui.h"

using namespace std;

enum GameState
{
    STATE_MENU,
    STATE_GAME
};

enum GridState
{
    REVEALED,
    BOMB,
    FLAG,
};

enum TileType
{
    TILE_HIDDEN,
    TILE_REVEALED,
    TILE_FLAG,
    TILE_BOMB
};

int main()
{
    InitWindow(800, 600, "dansweeper");
    SetTargetFPS(120);
    GuiLoadStyle("styles/default/style_default.rgs");

    GameState state = STATE_MENU;

    const int TEXTUREMAP_TILE_SIZE = 16;

    const int GRID_CELL_SIZE = 64;
    const int GRID_ROWS = 9;
    const int GRID_COLS = 9;
    bool revealed[GRID_ROWS][GRID_COLS] = {false};

    double startTime = GetTime();

    const Rectangle TILE_RECTS[] = {
        {1 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},          // TILE_HIDDEN
        {0 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},          // TILE_REVEALED
        {0, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // TILE_FLAG
        {TEXTUREMAP_TILE_SIZE, 0, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE}, // TILE_BOMB
    };

    // Load texture atlas
    Texture2D tileAtlas = LoadTexture("resources/texturemap.png");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        if (state == STATE_MENU)
        {
            if (GuiButton((Rectangle){300, 250, 200, 50}, "START GAME"))
            {
                state = STATE_GAME;
                startTime = GetTime();
            }
        }
        else if (state == STATE_GAME)
        {
            for (int y = 0; y < GRID_ROWS; y++)
            {
                for (int x = 0; x < GRID_COLS; x++)
                {
                    Rectangle dest = {x * GRID_CELL_SIZE, y * GRID_CELL_SIZE, GRID_CELL_SIZE, GRID_CELL_SIZE};

                    TileType type = revealed[y][x] ? TILE_REVEALED : TILE_HIDDEN;
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
