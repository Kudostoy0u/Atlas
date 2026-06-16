#pragma once

#include "atlas/index.hpp"

#include <filesystem>

namespace atlas {

void save_index(const InvertedIndex& index, const std::filesystem::path& path);
InvertedIndex load_index(const std::filesystem::path& path);

}  // namespace atlas
