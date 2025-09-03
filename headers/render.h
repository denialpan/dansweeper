#ifndef RENDER_H
#define RENDER_H

#include <string>

#include "headers/grid.h"
#include "raylib.h"

namespace render {
void LoadAssets();
void UnloadAssets();
void DrawBoard(const Grid* grid);

Camera2D& GetCamera();
void CenterCameraOnMap(const Grid* grid);

// Expose tilemap bounds
int GetMapPixelWidth();
int GetMapPixelHeight();

void QueueHighlight(int x, int y);
void highlightTile();

void DrawScreenBorderFromTileset(Texture2D borderTexture, int sliceSize);
}  // namespace render

#endif  // RENDER_H
