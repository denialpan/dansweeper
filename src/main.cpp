#include <iomanip>
#include <iostream>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "headers/grid.h"
#include "headers/raygui.h"

using namespace std;

enum GameState
{
    MENU,
    GAME,
};

const int TEXTUREMAP_TILE_SIZE = 16;
bool showMessageBox = false;

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "dansweeper");
    SetTargetFPS(120);
    GuiLoadStyle("styles/genesis/style_genesis.rgs");

    Font customFont = LoadFontEx("resources/ProggyClean.ttf", 13, 0, 250);
    SetTextureFilter(customFont.texture, TEXTURE_FILTER_POINT);

    GameState state = MENU;

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

    GridSettings gs = {
        1,
        1,
        1,
        "wasd",
    };

    Grid *gameGrid = new Grid(gs);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        int contentWidth = 240;
        int contentHeight = 250; // Adjust based on number of elements
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        int originX = (screenWidth - contentWidth) / 2;
        int originY = (screenHeight - contentHeight) / 2;

        if (state == MENU)
        {

            static int activeTab = 0; // 0 = Manual, 1 = Seed
            static int gridWidth = 9;
            static int gridHeight = 9;
            static int numMines = 10;

            static char seedText[17] = ""; // 16-char seed + null terminator
            static bool confirmPressed = false;

            static bool useSeed = false; // false = manual, true = seed

            // Title
            GuiLabel((Rectangle){originX, originY, 240, 20}, "dansweeper");

            // Toggle button
            if (GuiButton((Rectangle){originX, originY + 30, 240, 30},
                          useSeed ? "Custom" : "Seed"))
            {
                useSeed = !useSeed;
            }

            // Manual config
            if (!useSeed)
            {
                float tempGridWidth = (float)gridWidth;
                float tempGridHeight = (float)gridHeight;
                float tempNumMines = (float)numMines;

                int maxMines = gridWidth * gridHeight - 1;

                GuiSliderBar((Rectangle){originX, originY + 80, 240, 20}, "Width", TextFormat("%d", gridWidth), &tempGridWidth, 5.0f, 30.0f);
                GuiSliderBar((Rectangle){originX, originY + 110, 240, 20}, "Height", TextFormat("%d", gridHeight), &tempGridHeight, 5.0f, 30.0f);
                GuiSliderBar((Rectangle){originX, originY + 140, 240, 20}, "Mines", TextFormat("%d", numMines), &tempNumMines, 1.0f, (float)maxMines);

                gridWidth = (int)(tempGridWidth + 0.5f);
                gridHeight = (int)(tempGridHeight + 0.5f);
                numMines = (int)(tempNumMines + 0.5f);
            }
            else
            {
                GuiLabel((Rectangle){originX, originY + 80, 240, 20}, "Seed Input:");
                GuiTextBox((Rectangle){originX, originY + 100, 240, 30}, seedText, 17, true);
            }

            // Confirm button
            if (GuiButton((Rectangle){originX, originY + 190, 240, 30}, "Confirm"))
            {
                if (useSeed)
                {
                    // Use seedText
                }
                else
                {
                    // Use gridWidth, gridHeight, numMines
                }
                state = GAME;
            }
        }
        else if (state == GAME)
        {
        }

        int fps = GetFPS();
        const char *fpsText = TextFormat("%d FPS", fps);
        int fpsTextWidth = MeasureText(fpsText, 20);
        DrawTextEx(customFont, fpsText, (Vector2){170, 0}, 13, 0, LIME);
        DrawTextEx(customFont, "CONFIGURATION", (Vector2){0, 0}, 13, 0, LIME);

        EndDrawing();
    }

    UnloadTexture(tileAtlas);
    CloseWindow();
    return 0;
}
