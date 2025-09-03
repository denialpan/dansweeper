#include "headers/render.h"

#include <iostream>
#include <mutex>
#include <vector>

#include "headers/globals.h"
#include "headers/grid.h"
#include "headers/inputcontroller.h"
#include "headers/raygui.h"
#include "raylib.h"
#include "raymath.h"

namespace render {

static Texture2D textureTileset;
static Camera2D camera;
static const Grid* activeGrid = nullptr;

// highlight tile
static std::vector<std::pair<int, int>> highlightedTiles;
static std::mutex highlightMutex;

bool showEndscreen = false;
GameState previousWindowState = GameState::ONGOING;

void QueueHighlight(int x, int y) {
    std::lock_guard<std::mutex> lock(highlightMutex);
    highlightedTiles.push_back({x, y});
}

void LoadAssets() {
    Image texturemap = LoadImage("resources/texturemap.png");
    textureTileset = LoadTextureFromImage(texturemap);
    UnloadImage(texturemap);
}

void UnloadAssets() {
    UnloadTexture(textureTileset);
}

void CenterCameraOnMap(const Grid* grid) {
    int mapWidthPixels = grid->width * TILE_TEXTURE_PIXEL_SIZE;
    int mapHeightPixels = grid->height * TILE_TEXTURE_PIXEL_SIZE;

    camera.target = {mapWidthPixels / 2.0f, mapHeightPixels / 2.0f};
    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.zoom = 1.0f;
    camera.rotation = 0.0f;
}

void DrawBoard(const Grid* grid) {
    if (!grid) {
        return;
    };
    activeGrid = grid;

    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    BeginMode2D(camera);

    // CULLING: compute visible tile bounds
    Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D(
        {(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    int startX = Clamp((int)(topLeft.x / TILE_TEXTURE_PIXEL_SIZE), 0, grid->width - 1);
    int endX = Clamp((int)(bottomRight.x / TILE_TEXTURE_PIXEL_SIZE) + 1, 0, grid->width);
    int startY = Clamp((int)(topLeft.y / TILE_TEXTURE_PIXEL_SIZE), 0, grid->height - 1);
    int endY = Clamp((int)(bottomRight.y / TILE_TEXTURE_PIXEL_SIZE) + 1, 0, grid->height);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileID = grid->cells[y][x].renderTile;
            int srcX = (tileID % TILESET_COLS) * TILE_TEXTURE_PIXEL_SIZE;
            int srcY = (tileID / TILESET_COLS) * TILE_TEXTURE_PIXEL_SIZE;

            Rectangle srcRect = {(float)srcX, (float)srcY, (float)TILE_TEXTURE_PIXEL_SIZE, (float)TILE_TEXTURE_PIXEL_SIZE};
            Vector2 pos = {(float)(x * TILE_TEXTURE_PIXEL_SIZE), (float)(y * TILE_TEXTURE_PIXEL_SIZE)};
            DrawTextureRec(textureTileset, srcRect, pos, WHITE);
        }
    };

    render::highlightTile();

    EndMode2D();

    // draw endscreen once, allow post game examination
    if ((grid->gameState == GameState::WON || grid->gameState == GameState::LOST) &&
        previousWindowState == GameState::ONGOING) {
        showEndscreen = true;
    }

    previousWindowState = grid->gameState;

    if (showEndscreen) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        int boxWidth = 350;
        int boxHeight = 280;
        int boxX = (screenWidth - boxWidth) / 2;
        int boxY = (screenHeight - boxHeight) / 2;
        Rectangle windowBounds = {(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight};

        const char* title = grid->gameState == GameState::WON ? "You Win!" : "Game Over!";
        if (GuiWindowBox(windowBounds, title)) {
            showEndscreen = false;
            return;
        }

        // Stat Labels
        int labelX = boxX + 20;
        int labelY = boxY + 40;
        int spacing = 22;

        GuiLabel((Rectangle){labelX, labelY + spacing * 0, 300, 20}, TextFormat("Time: %.3f sec", grid->endStats.timeElapsed));
        GuiLabel((Rectangle){labelX, labelY + spacing * 1, 300, 20}, TextFormat("Revealed Tiles: %d", grid->endStats.numRevealed));
        GuiLabel((Rectangle){labelX, labelY + spacing * 2, 300, 20}, TextFormat("Flags Placed: %d", grid->endStats.numFlagged));
        GuiLabel((Rectangle){labelX, labelY + spacing * 3, 300, 20}, TextFormat("Bombs Left: %d", grid->endStats.bombsLeft));
        GuiLabel((Rectangle){labelX, labelY + spacing * 4, 300, 20}, TextFormat("Board Size: %d x %d", grid->endStats.width, grid->endStats.height));
        GuiLabel((Rectangle){labelX, labelY + spacing * 5, 300, 20}, TextFormat("Seed: %s", grid->endStats.seed32.c_str()));

        // Menu Button
        Rectangle quitBtn = {boxX + boxWidth / 2 - 50, boxY + boxHeight - 40, 100, 30};
        if (GuiButton(quitBtn, "Menu")) {
            windowState = WindowState::MENU;
            showEndscreen = false;
        }
    }
}

void highlightTile() {
    std::lock_guard<std::mutex> lock(highlightMutex);
    for (std::pair<int, int> tilePos : highlightedTiles) {
        Rectangle tile = {
            tilePos.first * TILE_TEXTURE_PIXEL_SIZE,
            tilePos.second * TILE_TEXTURE_PIXEL_SIZE,
            TILE_TEXTURE_PIXEL_SIZE,
            TILE_TEXTURE_PIXEL_SIZE};
        DrawRectangleRec(tile, Fade(YELLOW, 0.3f));
        DrawRectangleLinesEx(tile, 1, RED);
    }
    highlightedTiles.clear();  // Draw once per frame
}

Camera2D& GetCamera() {
    return camera;
}

int GetMapPixelWidth() {
    return activeGrid ? activeGrid->width * TILE_TEXTURE_PIXEL_SIZE : 0;
}

int GetMapPixelHeight() {
    return activeGrid ? activeGrid->height * TILE_TEXTURE_PIXEL_SIZE : 0;
}
}  // namespace render
