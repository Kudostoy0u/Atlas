#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace atlas {

class Tokenizer {
 public:
  std::vector<std::string> tokenize(std::string_view text) const;
  static std::string normalize(std::string_view token);
};

}  // namespace atlas
