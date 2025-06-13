#include "headers/grid.h"

#include <algorithm>
#include <array>
#include <ctime>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
// extremely messy grid seed generation handling

static const char b64_table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/";

// grid initialization
Grid::Grid(GridMetadata& metadata, const std::string& seed16, bool useSeed) {
    // generate 16 character seed from manual input
    if (!useSeed) {
        this->width = metadata.width;
        this->height = metadata.height;
        this->numMine = metadata.numMine;
        this->firstClick = true;

        std::random_device rd;
        std::mt19937_64 gen(rd());
        this->prngSeed = gen();

        this->cells.resize(this->height, std::vector<Cell>(this->width));
    } else {
        GridMetadata decoded = decodeBase64Seed(seed16);
        this->width = decoded.width;
        this->height = decoded.height;
        this->numMine = decoded.numMine;
        this->prngSeed = decoded.prngSeed;
        this->safeX = decoded.safeX;
        this->safeY = decoded.safeY;
        this->firstClick = false;
        this->seed32 = seed16;
        this->cells.resize(this->height, std::vector<Cell>(this->width));
        generateBoard();
    }
}

void Grid::generateBoard() {
    std::mt19937 rng(static_cast<unsigned int>(prngSeed));
    std::uniform_int_distribution<int> dist(0, width * height - 1);

    int avoidPos = safeY * width + safeX;
    std::unordered_set<int> minePositions;

    while (minePositions.size() < numMine) {
        int pos = dist(rng);
        if (pos == avoidPos) continue;  // skip first click cell
        minePositions.insert(pos);
    }

    for (int pos : minePositions) {
        int y = pos / width;
        int x = pos % width;
        cells[y][x].content = CELL_MINE;
        cells[y][x].renderTile = TILE_MINE_REVEALED;
    }
}

// Helper: encode raw bytes to Base64
std::string encodeBase64(const std::vector<uint8_t>& data) {
    std::string result;
    int val = 0;
    int valb = -6;
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(b64_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        result.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4)
        result.push_back('=');
    return result;
}

// Internal: reverse lookup table for Base64
std::array<int, 256> makeBase64ReverseMap() {
    std::array<int, 256> map{};
    map.fill(-1);
    const char* b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < 64; ++i)
        map[static_cast<unsigned char>(b64_table[i])] = i;
    return map;
}

// Internal: decode base64 string into bytes
std::vector<uint8_t> decodeBase64Bytes(const std::string& encoded) {
    static const auto revMap = makeBase64ReverseMap();
    std::vector<uint8_t> out;
    int val = 0, valb = -8;
    for (char c : encoded) {
        if (c == '=') break;
        int decoded = revMap[static_cast<unsigned char>(c)];
        if (decoded == -1) throw std::runtime_error("Invalid Base64 character in seed");
        val = (val << 6) + decoded;
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// Main method: creates base64 seed with packed width/height/mines/prng/safeXY
std::string createBase64Seed(uint8_t width, uint8_t height, uint16_t numMines,
                             uint16_t safeX, uint16_t safeY, uint64_t prngSeed) {
    std::vector<uint8_t> bytes;

    bytes.push_back(width);
    bytes.push_back(height);
    bytes.push_back((numMines >> 8) & 0xFF);
    bytes.push_back(numMines & 0xFF);

    for (int i = 7; i >= 0; --i)
        bytes.push_back((prngSeed >> (i * 8)) & 0xFF);

    bytes.push_back((safeX >> 8) & 0xFF);
    bytes.push_back(safeX & 0xFF);
    bytes.push_back((safeY >> 8) & 0xFF);
    bytes.push_back(safeY & 0xFF);

    return encodeBase64(bytes);
}

GridMetadata decodeBase64Seed(const std::string& base64) {
    auto bytes = decodeBase64Bytes(base64);
    if (bytes.size() != 16)
        throw std::runtime_error("Invalid Base64 seed length: expected 16 bytes after decoding.");

    uint16_t width = bytes[0];
    uint16_t height = bytes[1];
    uint16_t numMines = (bytes[2] << 8) | bytes[3];

    uint64_t prngSeed = 0;
    for (int i = 4; i < 12; ++i)
        prngSeed = (prngSeed << 8) | bytes[i];

    uint16_t safeX = (bytes[12] << 8) | bytes[13];
    uint16_t safeY = (bytes[14] << 8) | bytes[15];

    GridMetadata meta;
    meta.width = static_cast<uint8_t>(std::min<uint16_t>(width, 1000));
    meta.height = static_cast<uint8_t>(std::min<uint16_t>(height, 1000));
    meta.numMine = std::min(numMines, static_cast<uint16_t>(meta.width * meta.height - 1));
    meta.prngSeed = prngSeed;
    meta.safeX = safeX;
    meta.safeY = safeY;

    return meta;
}

std::string Grid::getSeed16() const {
    return this->seed32;
}