#ifndef RENDER_H
#define RENDER_H

#include <string>

#include "raylib.h"

namespace render {
void LoadAssets();
void UnloadAssets();
void DrawBoard(const std::string& s);

Camera2D& GetCamera();
void CenterCameraOnMap();

// Expose tilemap bounds
int GetMapPixelWidth();
int GetMapPixelHeight();
}  // namespace render

#endif  // RENDER_H
