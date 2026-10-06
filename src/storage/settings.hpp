#pragma once
#include "reader/settings.hpp"
#include <string>
namespace vr {
ReadingSettings loadSettings(const std::string& path);
bool saveSettings(const std::string& path,const ReadingSettings& settings);
}
