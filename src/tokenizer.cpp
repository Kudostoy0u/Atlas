#include "atlas/tokenizer.hpp"

#include <algorithm>
#include <cctype>

namespace atlas {

std::vector<std::string> Tokenizer::tokenize(std::string_view text) const {
  std::vector<std::string> tokens;
  std::string current;
  current.reserve(32);

  for (const unsigned char ch : text) {
    if (std::isalnum(ch) != 0) {
      current.push_back(static_cast<char>(std::tolower(ch)));
      continue;
    }

    if (!current.empty()) {
      tokens.push_back(current);
      current.clear();
    }
  }

  if (!current.empty()) {
    tokens.push_back(current);
  }

  return tokens;
}

std::string Tokenizer::normalize(std::string_view token) {
  std::string normalized;
  normalized.reserve(token.size());

  for (const unsigned char ch : token) {
    if (std::isalnum(ch) != 0) {
      normalized.push_back(static_cast<char>(std::tolower(ch)));
    }
  }

  return normalized;
}

}  // namespace atlas
