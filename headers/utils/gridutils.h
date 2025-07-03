#pragma once
#include <array>
#include <vector>

#include "headers/grid.h"

namespace gridutils {
// encoding
std::string createSeedFromManualInput(uint16_t width, uint16_t height, uint32_t numMines, uint16_t safeX, uint16_t safeY, uint64_t prngSeed);
std::string encodeBase64(const std::vector<uint8_t>& data);
std::string createBase64Seed(uint16_t width, uint16_t height, uint32_t numMines, uint16_t safeX, uint16_t safeY, uint64_t prngSeed);

// decoding
GridMetadata decodeSeed(const std::string& seed);
std::vector<uint8_t> decodeBase64Bytes(const std::string& encoded);
std::array<int, 256> makeBase64ReverseMap();

// validate metadata
GridMetadata validateMetadata(uint16_t width, uint16_t height, uint32_t numMines, uint64_t prngSeed, uint16_t safeX, uint16_t safeY);

}  // namespace gridutils
