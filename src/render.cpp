#include "headers/render.h"

#include "raylib.h"
#include "raymath.h"

namespace render {
constexpr int TILE_SIZE = 16;
constexpr int MAP_WIDTH = 10000;
constexpr int MAP_HEIGHT = 10000;
constexpr int TILESET_COLS = 4;  // e.g., if your tileset has 8 tiles per row

static Texture2D tileset;
static Camera2D camera;
static int tilemap[MAP_HEIGHT][MAP_WIDTH];  // 2D tile indices

enum TileId {
    TILE_1 = 0,
    TILE_2 = 1,
    TILE_3 = 2,
    TILE_4 = 3,
    TILE_5 = 4,
    TILE_6 = 5,
    TILE_7 = 6,
    TILE_8 = 7,
    TILE_REVEALED = 8,
    TILE_BLANK = 9,
    TILE_FLAG = 10,
    TILE_BOMB_WRONG = 11,
    TILE_QUESTION_REVEALED = 12,
    TILE_QUESTION = 13,
    TILE_BOMB_REVEALED = 14,
    TILE_BOMB_HIT = 15,
};

void LoadAssets() {
    Image image = LoadImage("resources/texturemap.png");
    tileset = LoadTextureFromImage(image);
    UnloadImage(image);

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            tilemap[y][x] = TILE_BLANK;
        }
    }

    CenterCameraOnMap();
}

void UnloadAssets() {
    UnloadTexture(tileset);
}

void CenterCameraOnMap() {
    int mapWidthPixels = MAP_WIDTH * TILE_SIZE;
    int mapHeightPixels = MAP_HEIGHT * TILE_SIZE;

    camera.target = {mapWidthPixels / 2.0f, mapHeightPixels / 2.0f};
    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    camera.zoom = 1.0f;
    camera.rotation = 0.0f;
}

void DrawBoard(const std::string& s) {
    camera.offset = {GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    BeginMode2D(camera);

    // CULLING: compute visible tile bounds
    Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D(
        {(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    int startX = Clamp((int)(topLeft.x / TILE_SIZE), 0, MAP_WIDTH - 1);
    int endX = Clamp((int)(bottomRight.x / TILE_SIZE) + 1, 0, MAP_WIDTH);
    int startY = Clamp((int)(topLeft.y / TILE_SIZE), 0, MAP_HEIGHT - 1);
    int endY = Clamp((int)(bottomRight.y / TILE_SIZE) + 1, 0, MAP_HEIGHT);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileID = tilemap[y][x];
            int srcX = (tileID % TILESET_COLS) * TILE_SIZE;
            int srcY = (tileID / TILESET_COLS) * TILE_SIZE;

            Rectangle srcRect = {(float)srcX, (float)srcY, (float)TILE_SIZE, (float)TILE_SIZE};
            Vector2 pos = {(float)(x * TILE_SIZE), (float)(y * TILE_SIZE)};

            DrawTextureRec(tileset, srcRect, pos, WHITE);
        }
    }

    EndMode2D();
}

Camera2D& GetCamera() {
    return camera;
}

int GetMapPixelWidth() {
    return MAP_WIDTH * TILE_SIZE;
}

int GetMapPixelHeight() {
    return MAP_HEIGHT * TILE_SIZE;
}
}  // namespace render
