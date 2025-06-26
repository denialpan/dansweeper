#include "headers/render.h"

#include <iostream>

#include "headers/globals.h"
#include "headers/grid.h"
#include "headers/inputcontroller.h"
#include "headers/raygui.h"
#include "raylib.h"
#include "raymath.h"

namespace render {

static Texture2D textureTileset;
static Texture2D borderTileset;
static Camera2D camera;
static const Grid* activeGrid = nullptr;

bool showEndscreen = false;
GameState previousWindowState = GameState::ONGOING;

void LoadAssets() {
    Image texturemap = LoadImage("resources/texturemap.png");
    Image bordermap = LoadImage("resources/bordermap.png");
    textureTileset = LoadTextureFromImage(texturemap);
    borderTileset = LoadTextureFromImage(bordermap);
    UnloadImage(texturemap);
    UnloadImage(bordermap);
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

    EndMode2D();

    if ((grid->gameState == GameState::WON || grid->gameState == GameState::LOST) &&
        previousWindowState == GameState::ONGOING) {
        showEndscreen = true;
    }

    previousWindowState = grid->gameState;

    if (showEndscreen) {
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Larger window to show stats
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
