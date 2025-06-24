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

        // Popup size and position
        int boxWidth = 300;
        int boxHeight = 180;
        int boxX = (screenWidth - boxWidth) / 2;
        int boxY = (screenHeight - boxHeight) / 2;
        Rectangle windowBounds = {(float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight};

        // Handle [X] button — returns true if clicked
        const char* title = grid->gameState == GameState::WON ? "You Win!" : "Game Over!";
        if (GuiWindowBox(windowBounds, title)) {
            showEndscreen = false;  // x button window
            return;
        }

        // Menu button (centered below the title)
        Rectangle quitBtn = {boxX + 100, boxY + 110, 100, 30};
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
