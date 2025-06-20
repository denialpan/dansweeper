#include "headers/inputcontroller.h"

#include <iostream>

#include "headers/render.h"
#include "headers/tile.h"
#include "headers/utils/gridutils.h"
#include "raylib.h"
#include "raymath.h"

const float zoomIncrementSpeed = 0.1f;
static float targetZoom = 2.0f;
InputController::InputController(Grid* grid) {
    this->grid = grid;
}

void InputController::handleManualInput() {
    // camera rendering interactions first
    Camera2D& camera = render::GetCamera();
    handlePanning(camera);
    handleZoomCursor(camera);
    clampCameraTarget(camera);
    this->gc = handleHoverCursor(camera);

    // grid interactions second
    if (this->gc.x < 0 && this->gc.y < 0) {
        return;
    } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        handleLeftClick();
    } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        handleRightClick();
    } else if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
        handleMiddleClick();
    }
};

void InputController::handleLeftClick() {
    // game already over
    if (grid->gameState == GameState::WON || grid->gameState == GameState::LOST) {
        return;
    }

    // first click implementation
    if (grid->firstClick) {
        grid->safeX = this->gc.x;
        grid->safeY = this->gc.y;
        grid->seed32 = gridutils::createSeedFromManualInput(grid->width, grid->height, grid->numMine, this->gc.x, this->gc.y, grid);
        grid->generateBoard();
        grid->firstClick = false;
    }
    grid->reveal(this->gc.x, this->gc.y);
    SetClipboardText(grid->getSeed32().c_str());
};

void InputController::handleRightClick() {
    if (grid->gameState == GameState::WON || grid->gameState == GameState::LOST) {
        return;
    }

    grid->flag(this->gc.x, this->gc.y);
};

void InputController::handleMiddleClick() {
    if (grid->gameState == GameState::WON || grid->gameState == GameState::LOST) {
        return;
    }
    grid->chord(this->gc.x, this->gc.y);
}

void InputController::handlePanning(Camera2D& camera) {
    if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f / camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }
};

void InputController::handleZoomCursor(Camera2D& camera) {
    // scroll wheel zoom on cursor origin
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        Vector2 mousePos = GetMousePosition();
        Vector2 worldBefore = GetScreenToWorld2D(mousePos, camera);

        // Update target zoom based on scroll
        targetZoom *= (1.0f + zoomIncrementSpeed * wheel);
        targetZoom = Clamp(targetZoom, 0.50f, 10.0f);

        // Adjust camera.target to keep zoom centered on cursor
        Vector2 worldAfter = GetScreenToWorld2D(mousePos, camera);
        Vector2 diff = Vector2Subtract(worldBefore, worldAfter);
        camera.target = Vector2Add(camera.target, diff);
    }

    camera.zoom = Lerp(camera.zoom, targetZoom, 0.15f);
};

void InputController::clampCameraTarget(Camera2D& camera) {
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
};

GridCoordinates InputController::handleHoverCursor(Camera2D& camera) {
    Vector2 world = GetScreenToWorld2D(GetMousePosition(), camera);

    int x = (int)(world.x / TILE_TEXTURE_PIXEL_SIZE);
    int y = (int)(world.y / TILE_TEXTURE_PIXEL_SIZE);

    if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
        return {x, y};
    } else {
        return {-1, -1};
    }
};

// void HandleInput(Grid* grid) {
//     static float targetZoom = 0.50f;
//     Camera2D& camera = render::GetCamera();

//     // middle mouse panning
//     if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
//         Vector2 delta = GetMouseDelta();
//         delta = Vector2Scale(delta, -1.0f / camera.zoom);
//         camera.target = Vector2Add(camera.target, delta);
//     }

//     // scroll wheel zoom on cursor origin
//     float wheel = GetMouseWheelMove();
//     if (wheel != 0.0f) {
//         Vector2 mousePos = GetMousePosition();
//         Vector2 worldBefore = GetScreenToWorld2D(mousePos, camera);

//         // Update target zoom based on scroll
//         targetZoom *= (1.0f + zoomIncrementSpeed * wheel);
//         targetZoom = Clamp(targetZoom, 0.50f, 10.0f);

//         // Adjust camera.target to keep zoom centered on cursor
//         Vector2 worldAfter = GetScreenToWorld2D(mousePos, camera);
//         Vector2 diff = Vector2Subtract(worldBefore, worldAfter);
//         camera.target = Vector2Add(camera.target, diff);
//     }

//     float halfScreenWidth = GetScreenWidth() / (2.0f * camera.zoom);
//     float halfScreenHeight = GetScreenHeight() / (2.0f * camera.zoom);

//     int mapWidth = render::GetMapPixelWidth();
//     int mapHeight = render::GetMapPixelHeight();

//     float minX = halfScreenWidth;
//     float maxX = mapWidth - halfScreenWidth;
//     float minY = halfScreenHeight;
//     float maxY = mapHeight - halfScreenHeight;

//     if (mapWidth * camera.zoom <= GetScreenWidth()) {
//         camera.target.x = mapWidth / 2.0f;
//     } else {
//         camera.target.x = Clamp(camera.target.x, minX, maxX);
//     }

//     if (mapHeight * camera.zoom <= GetScreenHeight()) {
//         camera.target.y = mapHeight / 2.0f;
//     } else {
//         camera.target.y = Clamp(camera.target.y, minY, maxY);
//     }

//     camera.zoom = Lerp(camera.zoom, targetZoom, 0.15f);

//     Vector2 mouse = GetMousePosition();
//     Vector2 world = GetScreenToWorld2D(mouse, render::GetCamera());

//     int x = (int)(world.x / TILE_TEXTURE_PIXEL_SIZE);
//     int y = (int)(world.y / TILE_TEXTURE_PIXEL_SIZE);

//     // if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
//     //     hvt = {true, x, y};
//     // } else {
//     //     hvt = {false, -1, -1};
//     // }

//     if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
//         if (x >= 0 && x < grid->width &&
//             y >= 0 && y < grid->height) {
//             // first click implementation
//             if (grid->firstClick) {
//                 grid->safeX = x;
//                 grid->safeY = y;
//                 grid->seed32 = grid->createSeedFromManualInput(grid->width, grid->height, grid->numMine, x, y);
//                 grid->generateBoard();
//                 grid->firstClick = false;
//             }

//             // Reveal the tile
//             grid->cells[y][x].renderTile = TILE_REVEALED;
//         }
//         SetClipboardText(grid->getSeed32().c_str());
//     }

//     if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
//         if (x >= 0 && x < grid->width &&
//             y >= 0 && y < grid->height && grid->cells[y][x].renderTile != TILE_REVEALED) {
//             // flag the tile
//             grid->cells[y][x].renderTile = TILE_FLAG;
//             std::cout << grid->cells[y][x].renderTile;
//         }
//     }
// }

// HoveredTile GetHoveredTile() {
//     return hvt;
// }