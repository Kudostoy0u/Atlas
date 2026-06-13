#pragma once

#include <cstdint>
#include <string>

namespace atlas {

using DocId = std::uint32_t;

struct Document {
  DocId id{};
  std::string external_id;
  std::string title;
  std::string body;
};

}  // namespace atlas
