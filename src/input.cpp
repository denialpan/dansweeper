#include "headers/input.h"

#include <iostream>

#include "headers/render.h"
#include "headers/tile.h"
#include "raylib.h"
#include "raymath.h"

namespace input {

const float zoomSpeed = 0.1f;
const float panSpeed = 1.0f;
HoveredTile hvt;

void HandleInput(Grid* grid) {
    static float targetZoom = 1.0f;
    Camera2D& camera = render::GetCamera();

    // Mouse drag panning (right-click)
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f / camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }

    // Zoom with scroll wheel (centered on cursor)
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mousePos = GetMousePosition();
        Vector2 worldBefore = GetScreenToWorld2D(mousePos, camera);

        // Update target zoom based on scroll
        targetZoom *= (1.0f + zoomSpeed * wheel);
        targetZoom = Clamp(targetZoom, 1.0f, 10.0f);

        // Adjust camera.target to keep zoom centered on cursor
        Vector2 worldAfter = GetScreenToWorld2D(mousePos, camera);
        Vector2 diff = Vector2Subtract(worldBefore, worldAfter);
        camera.target = Vector2Add(camera.target, diff);
    }

    float halfScreenWidth = GetScreenWidth() / (2.0f * camera.zoom);
    float halfScreenHeight = GetScreenHeight() / (2.0f * camera.zoom);

    int mapWidth = render::GetMapPixelWidth();
    int mapHeight = render::GetMapPixelHeight();

    float minX = halfScreenWidth;
    float maxX = mapWidth - halfScreenWidth;
    float minY = halfScreenHeight;
    float maxY = mapHeight - halfScreenHeight;

    if (mapWidth * camera.zoom <= GetScreenWidth()) {
        camera.target.x = mapWidth / 2.0f;
    } else {
        camera.target.x = Clamp(camera.target.x, minX, maxX);
    }

    if (mapHeight * camera.zoom <= GetScreenHeight()) {
        camera.target.y = mapHeight / 2.0f;
    } else {
        camera.target.y = Clamp(camera.target.y, minY, maxY);
    }

    camera.zoom = Lerp(camera.zoom, targetZoom, 0.15f);

    Vector2 mouse = GetMousePosition();
    Vector2 world = GetScreenToWorld2D(mouse, render::GetCamera());

    int x = (int)(world.x / TILE_TEXTURE_PIXEL_SIZE);
    int y = (int)(world.y / TILE_TEXTURE_PIXEL_SIZE);

    if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
        hvt = {true, x, y};
    } else {
        hvt = {false, -1, -1};
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (x >= 0 && x < grid->width &&
            y >= 0 && y < grid->height) {
            // first click implementation
            if (grid->firstClick) {
                grid->safeX = x;
                grid->safeY = y;
                grid->seed32 = createBase64Seed(grid->width, grid->height, grid->numMine, grid->safeX, grid->safeY, grid->prngSeed);
                grid->generateBoard();
                grid->firstClick = false;
            }

            // Reveal the tile
            grid->cells[y][x].renderTile = TILE_REVEALED;
        }
        SetClipboardText(grid->getSeed16().c_str());
    }

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        if (x >= 0 && x < grid->width &&
            y >= 0 && y < grid->height && grid->cells[y][x].renderTile != TILE_REVEALED) {
            // flag the tile
            grid->cells[y][x].renderTile = TILE_FLAG;
            std::cout << grid->cells[y][x].renderTile;
        }
    }
}

HoveredTile GetHoveredTile() {
    return hvt;
}

}  // namespace input
