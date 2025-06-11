#include "headers/grid.h"

#include <algorithm>
#include <array>
#include <ctime>
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
        this->prngSeed = metadata.prngSeed;
        this->seed16 = createBase64Seed(this->width, this->height, this->numMine);
    } else {
        GridMetadata metadataFromSeed = decodeBase64Seed(seed16);
        this->width = metadataFromSeed.width;
        this->height = metadataFromSeed.height;
        this->numMine = metadataFromSeed.numMine;
        this->prngSeed = metadataFromSeed.prngSeed;
        this->seed16 = seed16;
    }

    this->cells.resize(this->height, std::vector<Cell>(this->width));
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

// Main method: creates base64 seed with packed width/height/mines/prng
std::string createBase64Seed(uint8_t width, uint8_t height, uint16_t numMine) {
    std::vector<uint8_t> bytes;

    // Pack dimensions and mines
    bytes.push_back(width);
    bytes.push_back(height);
    bytes.push_back((numMine >> 8) & 0xFF);
    bytes.push_back(numMine & 0xFF);

    // Generate 64-bit random PRNG seed
    std::random_device rd;
    std::mt19937_64 gen(rd());
    uint64_t prngSeed = gen();

    for (int i = 7; i >= 0; --i)
        bytes.push_back((prngSeed >> (i * 8)) & 0xFF);

    return encodeBase64(bytes);
};

GridMetadata decodeBase64Seed(const std::string& base64) {
    auto bytes = decodeBase64Bytes(base64);
    if (bytes.size() != 12)
        throw std::runtime_error("Invalid Base64 seed length: expected 12 bytes after decoding.");

    uint16_t rawWidth = bytes[0];
    uint16_t rawHeight = bytes[1];
    uint16_t rawMines = (static_cast<uint16_t>(bytes[2]) << 8) | bytes[3];

    // Clamp width/height
    uint16_t width = std::min(rawWidth, static_cast<uint16_t>(1000));
    uint16_t height = std::min(rawHeight, static_cast<uint16_t>(1000));

    // Clamp mines to at most width * height - 1
    uint32_t maxMines = std::max(1, width * height - 1);  // ensure at least 1 cell is safe
    uint16_t numMines = std::min(rawMines, static_cast<uint16_t>(maxMines));

    // Decode PRNG seed
    uint64_t prngSeed = 0;
    for (int i = 4; i < 12; ++i)
        prngSeed = (prngSeed << 8) | bytes[i];

    GridMetadata meta;
    meta.width = static_cast<uint8_t>(width);
    meta.height = static_cast<uint8_t>(height);
    meta.numMine = numMines;
    meta.prngSeed = prngSeed;

    return meta;
}