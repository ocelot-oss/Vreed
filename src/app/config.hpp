#pragma once
#include <cstddef>
namespace vr::config {
constexpr const char* books = "ux0:/data/VitaReader/books";
constexpr const char* data = "ux0:/data/VitaReader";
constexpr const char* positions = "ux0:/data/VitaReader/positions";
constexpr const char* settings = "ux0:/data/VitaReader/settings.cfg";
constexpr int width = 960, height = 544;
constexpr std::size_t maxEntry = 4 * 1024 * 1024;
constexpr std::size_t maxEntries = 8192, maxBooks = 512;
}
