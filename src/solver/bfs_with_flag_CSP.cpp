// // #include "headers/solver/bfs_with_flag_CSP.h"

// #include <iostream>
// #include <queue>
// #include <set>
// #include <utility>
// #include <vector>

// #include "headers/grid.h"

// namespace solver {

// void runBFS_WITH_FLAG_CSP(Grid* grid, int startX, int startY) {
//     if (!grid || grid->cells[startY][startX].content == CELL_MINE) return;

//     grid->reveal(startX, startY);
//     std::set<std::pair<int, int>> visited;
//     std::queue<std::pair<int, int>> queue;

//     std::set<std::pair<int, int>> revealedNumberTiles;
//     queue.push({startX, startY});

//     // run until game end
//     while (grid->gameState == GameState::ONGOING) {
//         // visited and queue

//         while (queue.size() > 0) {
//             auto [x, y] = queue.front();
//             queue.pop();

//             // impossible tile out of bounds, or in visited
//             if (x < 0 || x >= grid->width || y < 0 || y >= grid->height || visited.count({x, y}))
//                 continue;

//             visited.insert({x, y});

//             std::vector<std::pair<int, int>> neighbors = getNeighbors(x, y);

//             // array for hidden neighbors
//             std::vector<std::pair<int, int>> hidden;
//             int flaggedNeighbors = 0;

//             for (std::pair<int, int> tile : hidden) {
//                 auto [neighborX, neighborY] = tile;
//                 Cell& neighborCell = grid->cells[neighborY][neighborX];

//                 if (!neighborCell.revealed && !neighborCell.flagged) {
//                     hidden.push_back(tile);
//                 } else if (neighborCell.flagged) {
//                     flaggedNeighbors += 1;
//                 }
//             }

//             Cell& cell = grid->cells[y][x];
//             if (cell.revealed && cell.adjacentMines > 0) {
//                 revealedNumberTiles.insert({x, y});
//             }

//             if (hidden.size() == cell.adjacentMines - flaggedNeighbors) {
//                 // all positions must be mine, flag all
//                 for (std::pair<int, int> tile : hidden) {
//                     auto [x, y] = tile;
//                     grid->flag(x, y);
//                     continue;
//                 }
//             } else if (cell.adjacentMines == flaggedNeighbors) {
//                 for (std::pair<int, int> tile : hidden) {
//                     auto [x, y] = tile;
//                     Cell& newCell = grid->cells[y][x];
//                     grid->reveal(x, y);
//                     if (newCell.adjacentMines > 0) {
//                         queue.push({x, y});
//                     }
//                 }
//             }
//         }

//         float minRisk = 1.0f;
//         std::pair<int, int> bestTile = {-1, -1};

//         // fallback choose lowest risk tile
//         // determined by smallest ratio of possible miles to number mine count
//         for (std::pair<int, int> tile : revealedNumberTiles) {
//             auto [x, y] = tile;
//             Cell& messyCode = grid->cells[y][x];
//             std::vector<std::pair<int, int>> neighbors = getNeighbors(x, y);
//             float hidden = 0.0f;
//             float flaggedNeighbor = 0.0f;

//             for (std::pair<int, int> neighbor : neighbors) {
//                 auto [neighborX, neighborY] = neighbor;
//                 Cell& neighborCell = grid->cells[neighborY][neighborX];
//                 if (!neighborCell.revealed && !neighborCell.flagged) {
//                     hidden += 1.0f;
//                 } else if (!neighborCell.flagged) {
//                     flaggedNeighbor += 1.0f;
//                 }
//             }

//             if ((messyCode.adjacentMines - flaggedNeighbor) / hidden < minRisk) {
//                 minRisk = (messyCode.adjacentMines - flaggedNeighbor) / hidden;
//                 bestTile = {x, y};
//             }
//         }

//         queue.push(bestTile);
//     }
// }

// // quick method to get adjacent neighbors given a tile coords
// std::vector<std::pair<int, int>> getNeighbors(int x, int y) {
//     std::vector<std::pair<int, int>> neighbors;

//     for (int dy = -1; dy <= 1; ++dy) {
//         for (int dx = -1; dx <= 1; ++dx) {
//             // skip the center tile
//             if (dx == 0 && dy == 0) continue;
//             neighbors.emplace_back(x + dx, y + dy);
//         }
//     }

//     return neighbors;
// }

// }  // namespace solver