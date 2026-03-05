#pragma once
#include "Common.h"

namespace TXD {
    namespace PS2 {
        std::vector<uint8_t> Unswizzle8(const std::vector<uint8_t>& buf, int w, int h);
        std::vector<uint8_t> UnswizzlePalette(const std::vector<uint8_t>& pal);
        void ProcessAndUploadTexture(RawTexture& raw);
        void TextureDataLoad(std::vector<uint8_t>& data, size_t& pos, size_t& sz, const std::vector<std::string>& allowedNames, bool fallback);
    }
}
