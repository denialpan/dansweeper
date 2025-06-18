#include "headers/utils/gridutils.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace gridutils {

// --- encoding ---
std::string createSeedFromManualInput(uint16_t width, uint16_t height, uint32_t numMines, uint16_t safeX, uint16_t safeY, Grid* grid) {
    auto now = std::chrono::high_resolution_clock::now();
    auto timeNs = now.time_since_epoch().count();

    std::ostringstream saltStream;
    saltStream << std::hex << timeNs;

    std::string salt = saltStream.str();

    std::string fullKey = std::to_string(width) + "x" + std::to_string(height) +
                          ":" + std::to_string(numMines) +
                          ":" + std::to_string(safeX) + "," + std::to_string(safeY) +
                          ":" + salt;

    std::hash<std::string> hasher;
    uint64_t prngSeed = hasher(fullKey);
    grid->prngSeed = prngSeed;

    return createBase64Seed(width, height, numMines, safeX, safeY, prngSeed);
}

std::string createBase64Seed(uint16_t width, uint16_t height, uint32_t numMines, uint16_t safeX, uint16_t safeY, uint64_t prngSeed) {
    std::vector<uint8_t> bytes;

    // Width (2 bytes)
    bytes.push_back((width >> 8) & 0xFF);
    bytes.push_back(width & 0xFF);

    // Height (2 bytes)
    bytes.push_back((height >> 8) & 0xFF);
    bytes.push_back(height & 0xFF);

    // Number of mines (4 bytes)
    bytes.push_back((numMines >> 24) & 0xFF);
    bytes.push_back((numMines >> 16) & 0xFF);
    bytes.push_back((numMines >> 8) & 0xFF);
    bytes.push_back(numMines & 0xFF);

    // PRNG seed (8 bytes)
    for (int i = 7; i >= 0; --i)
        bytes.push_back((prngSeed >> (i * 8)) & 0xFF);

    // Safe X (2 bytes)
    bytes.push_back((safeX >> 8) & 0xFF);
    bytes.push_back(safeX & 0xFF);

    // Safe Y (2 bytes)
    bytes.push_back((safeY >> 8) & 0xFF);
    bytes.push_back(safeY & 0xFF);

    // Encode to Base64 (should result in 28 Base64 characters with padding)
    return encodeBase64(bytes);
}

std::string encodeBase64(const std::vector<uint8_t>& data) {
    const char* b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;

    int val = 0;
    int valb = -6;

    for (uint8_t c : data) {
        val = (val << 8) | c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(b64_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6) {
        result.push_back(b64_table[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    while (result.size() % 4 != 0) {
        result.push_back('=');
    }

    return result;
};

// --- decoding ---
GridMetadata decodeSeed(const std::string& seed) {
    try {
        // is hashy seed
        auto bytes = decodeBase64Bytes(seed);
        if (bytes.size() == 20) {
            uint16_t width = (bytes[0] << 8) | bytes[1];
            uint16_t height = (bytes[2] << 8) | bytes[3];
            uint32_t numMines = (bytes[4] << 24) | (bytes[5] << 16) | (bytes[6] << 8) | bytes[7];

            uint64_t prngSeed = 0;
            for (int i = 8; i < 16; ++i)
                prngSeed = (prngSeed << 8) | bytes[i];

            uint16_t safeX = (bytes[16] << 8) | bytes[17];
            uint16_t safeY = (bytes[18] << 8) | bytes[19];

            return GridMetadata{width, height, (int)numMines, (int)prngSeed, safeX, safeY};
        }
    } catch (int error) {
        std::cerr << error;
    }

    // fall back random text
    std::hash<std::string> hasher;
    uint64_t prngSeed = hasher(seed);

    std::mt19937_64 gen(prngSeed);
    std::uniform_int_distribution<int> sizeDist(10, 100);  // Or up to 1000 if allowed
    uint16_t width = sizeDist(gen);
    uint16_t height = sizeDist(gen);

    int maxCells = width * height;
    int minMines = std::max(1, static_cast<int>(maxCells * 0.15));
    int maxMines = std::max(minMines + 1, static_cast<int>(maxCells * 0.25));
    std::uniform_int_distribution<int> mineDist(minMines, maxMines);
    uint32_t numMines = mineDist(gen);

    return GridMetadata{width, height, (int)numMines, (int)prngSeed, -1, -1};  // safeX/Y unset
};

std::vector<uint8_t> decodeBase64Bytes(const std::string& encoded) {
    static const std::array<int, 256> revMap = makeBase64ReverseMap();

    std::vector<uint8_t> out;
    int val = 0;
    int valb = -8;

    for (char c : encoded) {
        if (c == '=') break;

        int decoded = revMap[static_cast<unsigned char>(c)];
        if (decoded == -1)
            throw std::runtime_error("Invalid Base64 character in seed");

        val = (val << 6) + decoded;
        valb += 6;

        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return out;
};

std::array<int, 256> makeBase64ReverseMap() {
    std::array<int, 256> map{};
    map.fill(-1);

    const std::string b64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (int i = 0; i < 64; ++i) {
        map[static_cast<unsigned char>(b64_table[i])] = i;
    }

    return map;
};

}  // namespace gridutils