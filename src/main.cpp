#include <algorithm>
#include <format>
#include <iomanip>
#include <iostream>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "headers/globals.h"
#include "headers/grid.h"
#include "headers/inputcontroller.h"
#include "headers/raygui.h"
#include "headers/render.h"

using namespace std;

WindowState windowState = WindowState::MENU;

const char* WindowStateToString(WindowState state) {
    switch (state) {
        case WindowState::MENU:
            return "MENU";
        case WindowState::GAME:
            return "GAME";
        case WindowState::PAUSE:
            return "PAUSE";
        case WindowState::SETTINGS:
            return "SETTINGS";
        default:
            return "UNKNOWN";
    }
}

// void DrawHoveredTileLabel() {
//     inputController::HoveredTile hvt = inputController::GetHoveredTile();

//     if (hvt.valid) {
//         char buffer[32];
//         snprintf(buffer, sizeof(buffer), "(%d, %d)", hvt.x, hvt.y);

//         Vector2 textPos = {10, 10};  // Adjust as needed
//         DrawTextEx(GetFontDefault(), buffer, textPos, 20, 1, RAYWHITE);
//     }
// }

// helper sanitize text input
bool isValidBase64Char(char c) {
    return (isalnum(c) || c == '+' || c == '/' || c == '=');
}

int main() {
    // resizable vsync window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(500, 400, "dansweeper");
    SetWindowMinSize(500, 400);
    SetExitKey(KEY_NULL);
    SetTargetFPS(0);

    GuiLoadStyle("styles/genesis/style_genesis.rgs");
    Font customFont = LoadFontEx("resources/ProggyClean.ttf", 13, 0, 250);
    SetTextureFilter(customFont.texture, TEXTURE_FILTER_POINT);

    Grid* currentGrid = nullptr;
    GridMetadata metadata;

    render::LoadAssets();

    // initial menu settings
    static int gridWidth = 9;
    static int gridHeight = 9;
    static int numMine = 10;

    static bool debug = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (windowState == WindowState::MENU) {
            int contentWidth = 250;
            int contentHeight = 290;  // Adjust based on number of elements
            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();

            int originX = (screenWidth - contentWidth) / 2;
            int originY = (screenHeight - contentHeight) / 2;

            static char seedText[33] = "";  // 32-char seed + null terminator

            static bool useSeed = false;  // false = manual, true = seed

            // Title
            GuiLabel((Rectangle){originX, originY, 250, 20}, "dansweeper");

            // Toggle button
            if (GuiButton((Rectangle){originX, originY + 30, 250, 30},
                          useSeed ? "Custom" : "Seed")) {
                useSeed = !useSeed;
            }

            // Manual config
            if (!useSeed) {
                float tempGridWidth = (float)gridWidth;
                float tempGridHeight = (float)gridHeight;
                float tempNumMines = (float)numMine;

                static char widthInput[8] = "9";
                static char heightInput[8] = "9";
                static char minesInput[8] = "10";
                static bool widthEdit = false;
                static bool heightEdit = false;
                static bool minesEdit = false;

                int maxMines = gridWidth * gridHeight - 1;

                const int rowHeight = 30;
                auto GetRowY = [&](int row) {
                    return originY + row * rowHeight;
                };

                int row = 3;  // Skip 0 = title, 1 = seed toggle

                // --- Difficulty Presets on Row 2 ---
                int buttonWidth = (contentWidth - 14) / 4;
                int buttonY = GetRowY(row++);

                if (GuiButton({(float)originX + 0 * (buttonWidth + 5), (float)buttonY, (float)buttonWidth, 20}, "Easy")) {
                    tempGridWidth = 9;
                    tempGridHeight = 9;
                    tempNumMines = 10;
                    snprintf(widthInput, sizeof(widthInput), "%d", 9);
                    snprintf(heightInput, sizeof(heightInput), "%d", 9);
                    snprintf(minesInput, sizeof(minesInput), "%d", 10);
                }
                if (GuiButton({(float)originX + 1 * (buttonWidth + 5), (float)buttonY, (float)buttonWidth, 20}, "Medium")) {
                    tempGridWidth = 16;
                    tempGridHeight = 16;
                    tempNumMines = 40;
                    snprintf(widthInput, sizeof(widthInput), "%d", 16);
                    snprintf(heightInput, sizeof(heightInput), "%d", 16);
                    snprintf(minesInput, sizeof(minesInput), "%d", 40);
                }
                if (GuiButton({(float)originX + 2 * (buttonWidth + 5), (float)buttonY, (float)buttonWidth, 20}, "Hard")) {
                    tempGridWidth = 30;
                    tempGridHeight = 16;
                    tempNumMines = 99;
                    snprintf(widthInput, sizeof(widthInput), "%d", 30);
                    snprintf(heightInput, sizeof(heightInput), "%d", 16);
                    snprintf(minesInput, sizeof(minesInput), "%d", 99);
                }
                if (GuiButton({(float)originX + 3 * (buttonWidth + 5), (float)buttonY, (float)buttonWidth - 1, 20}, "Extreme")) {
                    tempGridWidth = 50;
                    tempGridHeight = 50;
                    tempNumMines = 300;
                    snprintf(widthInput, sizeof(widthInput), "%d", 50);
                    snprintf(heightInput, sizeof(heightInput), "%d", 50);
                    snprintf(minesInput, sizeof(minesInput), "%d", 300);
                }

                // --- Width on Row 3 ---
                GuiSliderBar({(float)originX, (float)GetRowY(row), 180, 20}, "Width", NULL, &tempGridWidth, 5.0f, 250.0f);
                if (!widthEdit) snprintf(widthInput, sizeof(widthInput), "%d", (int)tempGridWidth);
                if (GuiTextBox({(float)originX + 185, (float)GetRowY(row), 65, 20}, widthInput, sizeof(widthInput), widthEdit))
                    widthEdit = !widthEdit;
                if (!widthEdit) {
                    int value = atoi(widthInput);
                    if (value >= 5 && value <= 10000) tempGridWidth = (float)value;
                }
                row++;

                // --- Height on Row 4 ---
                GuiSliderBar({(float)originX, (float)GetRowY(row), 180, 20}, "Height", NULL, &tempGridHeight, 5.0f, 250.0f);
                if (!heightEdit) snprintf(heightInput, sizeof(heightInput), "%d", (int)tempGridHeight);
                if (GuiTextBox({(float)originX + 185, (float)GetRowY(row), 65, 20}, heightInput, sizeof(heightInput), heightEdit))
                    heightEdit = !heightEdit;
                if (!heightEdit) {
                    int value = atoi(heightInput);
                    if (value >= 5 && value <= 10000) tempGridHeight = (float)value;
                }
                row++;

                // --- Mines on Row 5 ---
                GuiSliderBar({(float)originX, (float)GetRowY(row), 180, 20}, "Mines", NULL, &tempNumMines, 1.0f, (float)maxMines);
                if (!minesEdit) snprintf(minesInput, sizeof(minesInput), "%d", (int)tempNumMines);
                if (GuiTextBox({(float)originX + 185, (float)GetRowY(row), 65, 20}, minesInput, sizeof(minesInput), minesEdit))
                    minesEdit = !minesEdit;
                if (!minesEdit) {
                    int value = atoi(minesInput);
                    if (value >= 1 && value <= maxMines) tempNumMines = (float)value;
                }
                row++;

                // Apply final values
                gridWidth = (int)(tempGridWidth + 0.5f);
                gridHeight = (int)(tempGridHeight + 0.5f);
                numMine = (int)(tempNumMines + 0.5f);

            } else {
                GuiLabel((Rectangle){originX, originY + 80, 250, 20}, "Seed Input:");
                GuiTextBox((Rectangle){originX, originY + 100, 250, 30}, seedText, 33, true);

                // Filter out spaces manually
                int len = strlen(seedText);
                int writeIndex = 0;

                for (int i = 0; i < len; i++) {
                    if (isValidBase64Char(seedText[i])) {
                        seedText[writeIndex++] = seedText[i];
                    }
                }
                seedText[writeIndex] = '\0';  // Null-terminate
            }

            // Confirm button
            if (GuiButton((Rectangle){originX, originY + 220, 250, 30}, "Confirm")) {
                metadata.width = gridWidth;
                metadata.height = gridHeight;
                metadata.numMine = numMine;

                currentGrid = new Grid(metadata, std::string(seedText), useSeed);
                render::CenterCameraOnMap(currentGrid);
                windowState = WindowState::GAME;
            }
        } else if (windowState == WindowState::GAME || windowState == WindowState::PAUSE) {
            if (windowState != WindowState::PAUSE) {
                InputController* manualSolve = new InputController(currentGrid);
                render::DrawBoard(currentGrid);
                manualSolve->handleManualInput();
                DrawTextEx(GetFontDefault(), currentGrid->getSeed32().c_str(), {10, 50}, 20, 1, RAYWHITE);
            } else {
                const int screenWidth = GetScreenWidth();
                const int screenHeight = GetScreenHeight();

                Rectangle panel = {screenWidth / 2 - 100, screenHeight / 2 - 100, 200, 220};

                DrawRectangleRec(panel, DARKGRAY);
                DrawRectangleLinesEx(panel, 2, LIGHTGRAY);
                DrawText("Paused", panel.x + 60, panel.y + 20, 20, RAYWHITE);

                if (GuiButton({panel.x + 30, panel.y + 60, 140, 30}, "Resume")) {
                    windowState = WindowState::GAME;
                }

                // if (GuiButton({panel.x + 30, panel.y + 100, 140, 30}, "Settings")) {
                //     windowState = WindowState::SETTINGS;
                // }

                if (GuiButton({panel.x + 30, panel.y + 140, 140, 30}, "Quit to Menu")) {
                    windowState = WindowState::MENU;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                windowState = (windowState == WindowState::PAUSE) ? WindowState::GAME : WindowState::PAUSE;
            }
        };

        if (IsKeyPressed(KEY_F3)) {
            debug = (debug) ? false : true;
        };

        if (debug) {
            int fps = GetFPS();
            std::string fpsText = std::to_string(fps) + " FPS";
            DrawTextEx(customFont, fpsText.c_str(), {10, 10}, 13, 1, WHITE);
            DrawTextEx(customFont, std::format("window state: {}", std::string(WindowStateToString(windowState))).c_str(), {10, 25}, 13, 1, WHITE);

            if (currentGrid) {
                DrawTextEx(customFont, "grid: exists", {10, 40}, 13, 1, WHITE);
            } else {
                DrawTextEx(customFont, std::format("grid: {}", NULL).c_str(), {10, 40}, 13, 1, WHITE);
            }
        }
        EndDrawing();
    }

    render::UnloadAssets();
    CloseWindow();
    return 0;
}
