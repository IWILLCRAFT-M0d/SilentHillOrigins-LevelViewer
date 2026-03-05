#include "Textures.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>

std::vector<uint8_t> TXD::PS2::Unswizzle8(const std::vector<uint8_t>& buf, int w, int h) {
    std::vector<uint8_t> out(w * h);
    if (buf.size() < (size_t)(w * h)) return out;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int block_loc = (y & (~0xf)) * w + (x & (~0xf)) * 2;
            int swap_sel = (((y + 2) >> 2) & 0x1) * 4;
            int posY = (((y & (~3)) >> 1) + (y & 1)) & 0x7;
            int col_loc = posY * w * 2 + ((x + swap_sel) & 0x7) * 4;
            int byte_num = ((y >> 1) & 1) + ((x >> 2) & 2);
            int swizzleid = block_loc + col_loc + byte_num;
            if ((size_t)swizzleid < buf.size()) out[y * w + x] = buf[swizzleid];
        }
    }
    return out;
}

std::vector<uint8_t> TXD::PS2::UnswizzlePalette(const std::vector<uint8_t>& pal) {
    std::vector<uint8_t> newPal(1024, 0);
    if (pal.size() < 1024) return pal;
    for (int p = 0; p < 256; p++) {
        int pos = ((p & 0xE7) + ((p & 8) << 1) + ((p & 16) >> 1));
        if (pos < 256 && (pos * 4 + 3) < (int)newPal.size() && (p * 4 + 3) < (int)pal.size()) {
            for (int k = 0; k < 4; k++) newPal[pos * 4 + k] = pal[p * 4 + k];
        }
    }
    return newPal;
}

void TXD::PS2::ProcessAndUploadTexture(RawTexture& raw) {
    int w = raw.width, h = raw.height;
    if (w <= 0 || h <= 0) return;

    std::vector<uint8_t> indices;
    if (raw.depth == 8) {
        indices = TXD::PS2::Unswizzle8(raw.pixels, w, h);
    } else if (raw.depth == 4) {
        // Unpack nibbles to a byte-per-pixel buffer, then deswizzle at full w×h.
        std::vector<uint8_t> unpacked(w * h, 0);
        for (size_t i = 0; i < raw.pixels.size(); i++) {
            uint8_t val = raw.pixels[i];
            if (i * 2     < unpacked.size()) unpacked[i * 2]     = val & 0xF;
            if (i * 2 + 1 < unpacked.size()) unpacked[i * 2 + 1] = (val >> 4) & 0xF;
        }
        indices = TXD::PS2::Unswizzle8(unpacked, w, h);
    }

    std::vector<uint8_t> finalPal;
    if (raw.depth == 8) finalPal = TXD::PS2::UnswizzlePalette(raw.palette);
    else finalPal = raw.palette;

    std::vector<uint8_t> rgba(w * h * 4, 255);
    for (int i = 0; i < w * h; ++i) {
        if ((size_t)i >= indices.size()) break;
        uint8_t idx = indices[i];
        if (raw.depth == 4) idx &= 0xF;
        int pIdx = idx * 4;
        if ((size_t)pIdx + 3 < finalPal.size()) {
            rgba[i * 4 + 0] = finalPal[pIdx + 0];
            rgba[i * 4 + 1] = finalPal[pIdx + 1];
            rgba[i * 4 + 2] = finalPal[pIdx + 2];
            rgba[i * 4 + 3] = std::min((int)finalPal[pIdx + 3] * 2, 255);
        }
    }

    glGenTextures(1, &raw.glID);
    glBindTexture(GL_TEXTURE_2D, raw.glID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

    GLint wrapS = raw.clampU ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    GLint wrapT = raw.clampV ? GL_CLAMP_TO_EDGE : GL_REPEAT;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    g_TextureMap[raw.name] = raw.glID;
    std::string upper = raw.name;
    for (auto& c : upper) c = toupper(c);
    g_TextureMap[upper] = raw.glID;

    // Store preview info for the TXD browser window
    TexPreviewInfo pi; pi.glID = raw.glID; pi.width = w; pi.height = h; pi.depth = raw.depth;
    g_TexInfo[raw.name]  = pi;
    g_TexInfo[upper]     = pi;
}

void TXD::PS2::TextureDataLoad(std::vector<uint8_t>& data, size_t& pos, size_t& sz, const std::vector<std::string>& allowedNames, bool fallback) {
    const int MAGIC_OFFSET = 80;
    //uint32_t type; memcpy(&type, &data[pos], 4);
    uint32_t chunkSz; memcpy(&chunkSz, &data[pos + 4], 4);
    size_t curr = pos + 32;
    uint32_t sLen; memcpy(&sLen, &data[curr + 4], 4);
    std::string tName = (sLen < 128) ? std::string((char*)&data[curr + 12]) : "Unknown";
    curr += 12 + sLen; memcpy(&sLen, &data[curr + 4], 4); curr += 12 + sLen; curr += 24;

    uint32_t w, h, d;
    memcpy(&w, &data[curr], 4); memcpy(&h, &data[curr + 4], 4); memcpy(&d, &data[curr + 8], 4);
    uint32_t dSz, pSz;
    memcpy(&dSz, &data[curr + 48], 4); memcpy(&pSz, &data[curr + 52], 4);
    uint32_t rawWrapU, rawWrapV;
    memcpy(&rawWrapU, &data[curr + 56], 4);
    memcpy(&rawWrapV, &data[curr + 60], 4);
    curr += 76;

    RawTexture t;
    t.name = tName; t.width = w; t.height = h; t.depth = d;
    t.clampU = (rawWrapU != 0);
    t.clampV = (rawWrapV != 0);

    bool nameAllowed = fallback;
    if (!fallback) {
        for (const auto& allowed : allowedNames) {
            if (strcasecmp(t.name.c_str(), allowed.c_str()) == 0) {
                nameAllowed = true;
                break;
            }
        }
    }
    if (!nameAllowed) {
        pos += 12 + chunkSz; return;
    }

    if (curr + MAGIC_OFFSET + (dSz - MAGIC_OFFSET) <= sz && dSz > MAGIC_OFFSET) {
        t.pixels.resize(dSz - MAGIC_OFFSET);
        memcpy(t.pixels.data(), &data[curr + MAGIC_OFFSET], dSz - MAGIC_OFFSET);
    }
    curr += dSz;
    if (pSz > MAGIC_OFFSET && curr + MAGIC_OFFSET + (pSz - MAGIC_OFFSET) <= sz) {
        t.palette.resize(pSz - MAGIC_OFFSET);
        memcpy(t.palette.data(), &data[curr + MAGIC_OFFSET], pSz - MAGIC_OFFSET);
    }

    if (!t.pixels.empty() && g_TextureMap.find(t.name) == g_TextureMap.end()) {
        TXD::PS2::ProcessAndUploadTexture(t);
    }

    pos += 12 + chunkSz; return;
}
