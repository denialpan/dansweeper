#include "headers/render.h"

#include <iostream>

#include "headers/grid.h"
#include "headers/input.h"
#include "raylib.h"
#include "raymath.h"

namespace render {
constexpr int TILE_SIZE = 16;
constexpr int TILESET_COLS = 4;  // e.g., if your tileset has 8 tiles per row

static Texture2D tileset;
static Camera2D camera;
static const Grid* activeGrid = nullptr;

void LoadAssets() {
    Image image = LoadImage("resources/texturemap.png");
    tileset = LoadTextureFromImage(image);
    UnloadImage(image);
}

void UnloadAssets() {
    UnloadTexture(tileset);
}

void CenterCameraOnMap(const Grid* grid) {
    int mapWidthPixels = grid->width * TILE_SIZE;
    int mapHeightPixels = grid->height * TILE_SIZE;

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

    int startX = Clamp((int)(topLeft.x / TILE_SIZE), 0, grid->width - 1);
    int endX = Clamp((int)(bottomRight.x / TILE_SIZE) + 1, 0, grid->width);
    int startY = Clamp((int)(topLeft.y / TILE_SIZE), 0, grid->height - 1);
    int endY = Clamp((int)(bottomRight.y / TILE_SIZE) + 1, 0, grid->height);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            // Replace tilemap access:
            int tileID = grid->cells[y][x].renderTile;  // Or use visibility/content to determine tileID

            int srcX = (tileID % TILESET_COLS) * TILE_SIZE;
            int srcY = (tileID / TILESET_COLS) * TILE_SIZE;

            Rectangle srcRect = {(float)srcX, (float)srcY, (float)TILE_SIZE, (float)TILE_SIZE};
            Vector2 pos = {(float)(x * TILE_SIZE), (float)(y * TILE_SIZE)};
            DrawTextureRec(tileset, srcRect, pos, WHITE);
        }
    };

    input::HoveredTile tile = input::GetHoveredTile();
    if (tile.valid) {
        Rectangle tileRect = {
            tile.x * TILE_SIZE,
            tile.y * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE};

        DrawRectangleRec(tileRect, Fade(YELLOW, 0.3f));
        DrawRectangleLinesEx(tileRect, 1, ORANGE);
    }

    EndMode2D();
}

Camera2D& GetCamera() {
    return camera;
}

int GetMapPixelWidth() {
    return activeGrid ? activeGrid->width * TILE_SIZE : 0;
}

int GetMapPixelHeight() {
    return activeGrid ? activeGrid->height * TILE_SIZE : 0;
}
}  // namespace render
