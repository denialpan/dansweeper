#include <algorithm>
#include <format>
#include <iomanip>
#include <iostream>
#include <random>

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include <sys/stat.h>

#include "headers/globals.h"
#include "headers/grid.h"
#include "headers/inputcontroller.h"
#include "headers/raygui.h"
#include "headers/render.h"
#include "headers/solver/solvercontroller.h"

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

enum class MenuMode {
    MANUAL,
    SEED,
    SOLVER,
};

enum class SolverMainThreadState {
    IDLE,
    RUNNING,
    CREATE_BOARD,
    SOLVE_BOARD,
    ALL_FINISHED
};

// Function to generate a random string of a given length using Base64 characters
std::string generateRandomBase64String(int length) {
    // The Base64 character set
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    // Use a random device to seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Create a distribution for the indices of the base64_chars string
    std::uniform_int_distribution<> distrib(0, base64_chars.size() - 1);

    std::string random_string;
    random_string.reserve(length);  // Reserve memory for efficiency

    // Generate random characters and append to the string
    for (int i = 0; i < length; ++i) {
        random_string += base64_chars[distrib(gen)];
    }

    return random_string;
}

// helper sanitize text input
bool isValidBase64Char(char c) {
    return (isalnum(c) || c == '+' || c == '/' || c == '=');
}

void resetGrid(Grid*& grid, InputController*& ipc) {
    delete grid;
    grid = nullptr;
    delete ipc;
    ipc = nullptr;
}

void EnsureDirectoryExists(const char* path) {
    mkdir(path);
}

void SaveCsvPathToIni(const char* path, const char* filename = "resources/config.ini") {
    EnsureDirectoryExists("resources");

    FILE* file = fopen(filename, "w");
    if (file) {
        fprintf(file, "[Solver]\ncsv_path=%s\n", path);
        fclose(file);
    }
}

void LoadCsvPathFromIni(char* buffer, int bufferSize, const char* filename = "resources/config.ini") {
    FILE* file = fopen(filename, "r");
    if (!file) {
        // File doesn't exist — create it with default path
        SaveCsvPathToIni(buffer, filename);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "csv_path=%255[^\n]", buffer) == 1) break;
    }

    fclose(file);
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

    static MenuMode menuMode = MenuMode::MANUAL;
    Grid* currentGrid = nullptr;
    InputController* inputMethodology = nullptr;
    SolverController* solverMethodology = nullptr;
    GridMetadata gridMetadata;

    render::LoadAssets();

    static char csvFolderPath[256] = "results";
    static bool csvPathEdit = false;
    LoadCsvPathFromIni(csvFolderPath, sizeof(csvFolderPath));

    // initial manual settings
    static int gridWidth = 9;
    static int gridHeight = 9;
    static int numMine = 10;

    // initial solver settings
    static char boardPresetIndexFinal = 0;
    static SolverType solverPreset = SolverType::NONE;
    static int numBoardsToSolve = 10;
    static bool useSolver = false;
    static SolverMainThreadState solverMainThreadState = SolverMainThreadState::IDLE;
    static int boardsSolved = 0;
    static bool debug = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (windowState == WindowState::MENU) {
            // if game exists, reset
            if (currentGrid && inputMethodology) {
                resetGrid(currentGrid, inputMethodology);
            }

            int contentWidth = 250;
            int contentHeight = 290;
            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();

            int originX = (screenWidth - contentWidth) / 2;
            int originY = (screenHeight - contentHeight) / 2;

            static char seedText[33] = "";  // 32-char seed + null terminator

            static bool useSeed = false;

            const char* modeLabel = "";
            switch (menuMode) {
                case MenuMode::MANUAL:
                    modeLabel = "Manual";
                    break;
                case MenuMode::SEED:
                    modeLabel = "Seed";
                    break;
                case MenuMode::SOLVER:
                    modeLabel = "Solver";
                    break;
            }

            GuiLabel((Rectangle){originX, originY, 250, 20}, "dansweeper");

            // cycle toggle between 3
            if (GuiButton((Rectangle){originX, originY + 30, 250, 30}, modeLabel)) {
                menuMode = static_cast<MenuMode>(((static_cast<int>(menuMode) + 1)) % 3);
            }

            switch (menuMode) {
                case MenuMode::MANUAL: {
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

                    useSeed = false;
                    break;
                }

                case MenuMode::SEED: {
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
                    useSeed = true;
                    break;
                }

                case MenuMode::SOLVER: {
                    int solverRow = 3;
                    const int rowHeight = 30;
                    auto GetRowY = [&](int r) { return originY + r * rowHeight; };

                    static int boardPresetIndex = 0;
                    static const char* boardPresets = "Easy;Medium;Hard;Expert;Random";

                    static int solverTypeIndex = 0;

                    std::vector<std::pair<SolverType, std::string>> solverPairs = SolverController::getPairSolverString();
                    std::string solverComboText;
                    for (size_t i = 0; i < solverPairs.size(); ++i) {
                        solverComboText += solverPairs[i].second;
                        if (i + 1 < solverPairs.size()) solverComboText += ";";
                    }

                    static char boardsToSolve[8] = "9";
                    static bool boardsToSolveEdit = false;

                    float tempNumBoardsToSolve = (float)numBoardsToSolve;

                    static char csvPath[256] = "results.csv";
                    static bool csvEditMode = false;

                    // --- Board Preset ---
                    GuiLabel({(float)originX, (float)GetRowY(solverRow), 120, 20}, "Board Preset:");
                    GuiComboBox({(float)originX + 130, (float)GetRowY(solverRow), 120, 20}, boardPresets, &boardPresetIndex);
                    solverRow++;

                    // --- Solver Type ---
                    GuiLabel({(float)originX, (float)GetRowY(solverRow), 120, 20}, "Solver Type:");
                    GuiComboBox({(float)originX + 100, (float)GetRowY(solverRow), 150, 20}, solverComboText.c_str(), &solverTypeIndex);
                    solverRow++;

                    // --- Number of Boards to Solve ---
                    GuiLabel({(float)originX, (float)GetRowY(solverRow), 140, 20}, "# Boards to Solve:");
                    if (!boardsToSolveEdit) snprintf(boardsToSolve, sizeof(boardsToSolve), "%d", (int)tempNumBoardsToSolve);
                    if (GuiTextBox({(float)originX + 185, (float)GetRowY(solverRow), 65, 20}, boardsToSolve, sizeof(boardsToSolve), boardsToSolveEdit))
                        boardsToSolveEdit = !boardsToSolveEdit;
                    if (!boardsToSolveEdit) {
                        int value = atoi(boardsToSolve);
                        if (value >= 1 && value <= 1000) tempNumBoardsToSolve = (float)value;
                    }

                    solverRow++;

                    // --- CSV Save Path ---
                    GuiLabel({(float)originX, (float)GetRowY(solverRow), 120, 20}, "Save .csv:");
                    if (GuiTextBox({(float)originX + 70, (float)GetRowY(solverRow), 180, 20}, csvFolderPath, sizeof(csvFolderPath), csvPathEdit)) {
                        csvPathEdit = !csvPathEdit;
                    }
                    solverRow++;

                    // apply values
                    boardPresetIndexFinal = boardPresetIndex;
                    solverPreset = solverPairs[solverTypeIndex].first;
                    numBoardsToSolve = (int)(tempNumBoardsToSolve + 0.5f);

                    break;
                }

                default: {
                    GuiLabel((Rectangle){originX, originY + 80, 250, 20}, "how did you get here huh");
                    break;
                }
            }

            // Confirm button
            if (GuiButton((Rectangle){originX, originY + 220, 250, 30}, "Confirm")) {
                switch (menuMode) {
                    case MenuMode::MANUAL: {
                        gridMetadata.width = gridWidth;
                        gridMetadata.height = gridHeight;
                        gridMetadata.numMine = numMine;
                        currentGrid = new Grid(gridMetadata, "", useSeed);
                        render::CenterCameraOnMap(currentGrid);

                        break;
                    }

                    case MenuMode::SEED: {
                        gridMetadata = {};
                        currentGrid = new Grid(gridMetadata, std::string(seedText), useSeed);
                        render::CenterCameraOnMap(currentGrid);

                        break;
                    }

                    case MenuMode::SOLVER: {
                        SaveCsvPathToIni(csvFolderPath);
                        useSolver = true;
                        break;
                    }
                }

                windowState = WindowState::GAME;
            }

        } else if (windowState == WindowState::GAME && useSolver) {
            switch (solverMainThreadState) {
                case SolverMainThreadState::IDLE: {
                    boardsSolved = 0;
                    solverMainThreadState = SolverMainThreadState::RUNNING;
                    break;
                }

                case SolverMainThreadState::RUNNING: {
                    if (boardsSolved < numBoardsToSolve) {
                        solverMainThreadState = SolverMainThreadState::CREATE_BOARD;
                    } else {
                        solverMainThreadState = SolverMainThreadState::ALL_FINISHED;
                    }
                    break;
                }

                case SolverMainThreadState::CREATE_BOARD: {
                    GridMetadata solverMetadata;
                    std::string seed = "";
                    bool useSeed = false;
                    switch (boardPresetIndexFinal) {
                        case 0: {
                            solverMetadata.height = 9;
                            solverMetadata.width = 9;
                            solverMetadata.numMine = 10;
                            break;
                        }
                        case 1: {
                            solverMetadata.height = 16;
                            solverMetadata.width = 16;
                            solverMetadata.numMine = 40;
                            break;
                        }
                        case 2: {
                            solverMetadata.height = 16;
                            solverMetadata.width = 30;
                            solverMetadata.numMine = 99;
                            break;
                        }
                        case 3: {
                            solverMetadata.height = 50;
                            solverMetadata.width = 50;
                            solverMetadata.numMine = 300;
                            break;
                        }

                        case 4: {
                            seed = generateRandomBase64String(16);
                            useSeed = true;
                        }
                    }

                    currentGrid = new Grid(solverMetadata, seed, useSeed);
                    render::CenterCameraOnMap(currentGrid);
                    solverMethodology = new SolverController();
                    solverMethodology->start(currentGrid, SolverType::BRUTE_FORCE_DERN_STYLE);
                    solverMainThreadState = SolverMainThreadState::SOLVE_BOARD;
                    break;
                }
                case SolverMainThreadState::SOLVE_BOARD: {
                    DrawTextEx(customFont, std::format("board #{}", boardsSolved + 1).c_str(), {10, 10}, 39, 1, WHITE);
                    if (solverMethodology->isFinished()) {
                        boardsSolved++;
                        solverMainThreadState = SolverMainThreadState::RUNNING;
                    } else {
                        solverMethodology->step();
                    }
                    break;
                }
                case SolverMainThreadState::ALL_FINISHED: {
                    std::cout << std::format("boards solved: {}\n", boardsSolved);
                    solverMainThreadState = SolverMainThreadState::IDLE;
                    solverMethodology->stop();
                    delete solverMethodology;
                    solverMethodology = nullptr;
                    windowState = WindowState::MENU;
                    useSolver = false;
                    break;
                }
            }

            render::DrawBoard(currentGrid, useSolver);

        } else if (windowState == WindowState::GAME || windowState == WindowState::PAUSE) {
            if (windowState != WindowState::PAUSE) {
                inputMethodology = new InputController(currentGrid);
                render::DrawBoard(currentGrid, useSolver);
                inputMethodology->handleManualInput();
            } else {
                // pause implemented this way to prevent "pause board examination"
                const int screenWidth = GetScreenWidth();
                const int screenHeight = GetScreenHeight();

                Rectangle windowRect = {
                    screenWidth / 2.0f - 150,
                    screenHeight / 2.0f - 150,
                    300,
                    250};

                bool closeClicked = GuiWindowBox(windowRect, "Paused");
                if (closeClicked) {
                    windowState = WindowState::GAME;
                }

                // Button layout
                float btnWidth = 180;
                float btnHeight = 35;
                float btnX = windowRect.x + (windowRect.width - btnWidth) / 2;
                float spacing = 20;
                float startY = windowRect.y + 60;

                Rectangle resumeBtn = {btnX, startY, btnWidth, btnHeight};
                Rectangle quitBtn = {btnX, startY + btnHeight + spacing, btnWidth, btnHeight};

                if (GuiButton(resumeBtn, "Resume")) {
                    windowState = WindowState::GAME;
                }

                if (GuiButton(quitBtn, "Quit to Menu")) {
                    windowState = WindowState::MENU;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                windowState = (windowState == WindowState::PAUSE) ? WindowState::GAME : WindowState::PAUSE;
            }

            // if (IsKeyPressed(KEY_S)) {
            //     solverMethodology->step();
            // }

            currentGrid->updateTimer();
        };

        if (IsKeyPressed(KEY_F3)) {
            debug = (debug) ? false : true;
        };

        if (debug) {
            int fps = GetFPS();
            std::string fpsText = std::to_string(fps) + " FPS";
            DrawTextEx(customFont, fpsText.c_str(), {10, 10}, 13, 1, WHITE);
            DrawTextEx(customFont, std::format("window state: {}", std::string(WindowStateToString(windowState))).c_str(), {10, 25}, 13, 1, WHITE);

            if (currentGrid && inputMethodology) {
                DrawTextEx(customFont, "grid: exists", {10, 40}, 13, 1, WHITE);
                GridCoordinates coords = inputMethodology->handleHoverCursor(render::GetCamera());
                DrawTextEx(customFont, std::format("(x, y): {}, {}", coords.x, coords.y).c_str(), {10, 55}, 13, 1, WHITE);
                DrawTextEx(customFont, std::format("seed: {}", currentGrid->seed32).c_str(), {10, 70}, 13, 1, WHITE);
                DrawTextEx(customFont, std::format("prng: {}", currentGrid->prngSeed).c_str(), {10, 85}, 13, 1, WHITE);
                DrawTextEx(customFont, std::format("dims: {} x {}", currentGrid->width, currentGrid->height).c_str(), {10, 100}, 13, 1, WHITE);
                DrawTextEx(customFont, std::format("mine: {}", currentGrid->numMine).c_str(), {10, 115}, 13, 1, WHITE);
                DrawTextEx(customFont, std::format("safe: {}, {}", currentGrid->safeX, currentGrid->safeY).c_str(), {10, 130}, 13, 1, WHITE);
                DrawTextEx(customFont, std::format("time: {}", currentGrid->timeElapsed).c_str(), {10, 145}, 13, 1, WHITE);

                if (!(coords.x < 0 || coords.x >= currentGrid->width || coords.y < 0 || coords.y >= currentGrid->height)) {
                    Cell& cellPropertyState = currentGrid->cells[coords.y][coords.x];
                    DrawTextEx(customFont, std::format("cell property state: {}, {}", cellPropertyState.flagged, cellPropertyState.revealed).c_str(), {10, 160}, 13, 1, WHITE);
                }
            }
        }
        EndDrawing();
    }

    render::UnloadAssets();
    CloseWindow();
    return 0;
}
