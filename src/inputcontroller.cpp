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
    if (grid->gameState != GameState::ONGOING) {
        return;
    }

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
    grid->reveal(this->gc.x, this->gc.y);
    SetClipboardText(grid->getSeed32().c_str());
};

void InputController::handleRightClick() {
    grid->flag(this->gc.x, this->gc.y);
};

void InputController::handleMiddleClick() {
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
    if (!grid) {
        return {-1, -1};
    }

    Vector2 world = GetScreenToWorld2D(GetMousePosition(), camera);

    int x = (int)(world.x / TILE_TEXTURE_PIXEL_SIZE);
    int y = (int)(world.y / TILE_TEXTURE_PIXEL_SIZE);

    if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
        return {x, y};
    } else {
        return {-1, -1};
    }
};