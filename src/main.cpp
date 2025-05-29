#include <iomanip>
#include <iostream>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "headers/grid.h"
#include "headers/raygui.h"

using namespace std;

enum GameState {
    MENU,
    GAME,
};

const int TEXTUREMAP_TILE_SIZE = 16;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "dansweeper");
    SetTargetFPS(120);
    GuiLoadStyle("styles/default/style_default.rgs");

    GameState state = MENU;

    const Rectangle TILE_RECTS[] = {
        {0 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 1
        {1 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 2
        {2 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 3
        {3 * 16, 0 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 4
        {0 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 5
        {1 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 6
        {2 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 7
        {3 * 16, 1 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // 8
        {0 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // blank revealed
        {1 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // blank unrevealed
        {2 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // flag
        {3 * 16, 2 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // bomb wrong
        {0 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // ? revealed
        {1 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // ? unrevealed
        {2 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // bomb normal
        {3 * 16, 3 * 16, TEXTUREMAP_TILE_SIZE, TEXTUREMAP_TILE_SIZE},  // bomb hit
    };

    // Load texture atlas
    Texture2D tileAtlas = LoadTexture("resources/texturemap.png");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (state == MENU) {
            static int radioActive = 0;  // 0 = Custom config, 1 = Seed
            static bool useCustomConfig = true;
            static int inputWidth = 9;
            static int inputHeight = 9;
            static int inputBombs = 10;
            static char seedBuffer[16] = "Xy9z7Aa2";
            static bool configConfirmed = false;

            DrawText("Configuration", 330, 140, 20, RAYWHITE);

            // Radio toggle between Custom and Seed
            GuiToggleGroup((Rectangle){300, 170, 200, 24}, "Custom;Seed", &radioActive);
            useCustomConfig = (radioActive == 0);

            if (useCustomConfig) {
                // Width
                float widthTemp = static_cast<float>(inputWidth);
                GuiSliderBar((Rectangle){380, 210, 150, 20}, NULL, NULL, &widthTemp, 5, 50);
                inputWidth = static_cast<int>(widthTemp + 0.5f);

                // Height
                float heightTemp = static_cast<float>(inputHeight);
                GuiSliderBar((Rectangle){380, 240, 150, 20}, NULL, NULL, &heightTemp, 5, 50);
                inputHeight = static_cast<int>(heightTemp + 0.5f);

                // Bombs
                float bombsTemp = static_cast<float>(inputBombs);
                GuiSliderBar((Rectangle){380, 270, 150, 20}, NULL, NULL, &bombsTemp, 1, 999);
                inputBombs = static_cast<int>(bombsTemp + 0.5f);

                DrawText(TextFormat("%d", inputWidth), 540, 210, 20, RAYWHITE);
                DrawText(TextFormat("%d", inputHeight), 540, 240, 20, RAYWHITE);
                DrawText(TextFormat("%d", inputBombs), 540, 270, 20, RAYWHITE);
            } else {
                static bool seedEdit = false;

                Rectangle seedBox = {380, 210, 140, 20};
                if (GuiTextBox(seedBox, seedBuffer, 16, seedEdit)) {
                    // If textbox is active and Enter is pressed, exit edit mode
                    seedEdit = false;
                }
                if (CheckCollisionPointRec(GetMousePosition(), seedBox) &&
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    seedEdit = true;
                }
            }

            if (GuiButton((Rectangle){330, 320, 140, 40}, "CONFIRM")) {
                configConfirmed = true;
            }

        } else if (state == GAME) {
        }

        int fps = GetFPS();
        const char *fpsText = TextFormat("%d FPS", fps);
        int fpsTextWidth = MeasureText(fpsText, 20);
        DrawText(fpsText, GetScreenWidth() - fpsTextWidth - 10, 10, 20, LIME);

        EndDrawing();
    }

    UnloadTexture(tileAtlas);
    CloseWindow();
    return 0;
}
