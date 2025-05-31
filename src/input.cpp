#include "headers/input.h"

#include "headers/render.h"
#include "raylib.h"
#include "raymath.h"

namespace input {

const float zoomSpeed = 0.1f;
const float panSpeed = 1.0f;

void HandleInput() {
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

        camera.zoom *= (1.0f + zoomSpeed * wheel);
        if (camera.zoom < 1.0f) camera.zoom = 1.0f;
        if (camera.zoom > 10.0f) camera.zoom = 10.0f;

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
}

}  // namespace input
