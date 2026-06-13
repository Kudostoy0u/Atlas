#include "atlas/tokenizer.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "test failed: " << message << '\n';
    std::exit(1);
  }
}

void test_tokenizer_normalizes_words() {
  const atlas::Tokenizer tokenizer;
  const std::vector<std::string> tokens =
      tokenizer.tokenize("Plan-Review, CODE 2026!");

  require(tokens.size() == 4, "expected four tokens");
  require(tokens[0] == "plan", "expected lowercase token");
  require(tokens[1] == "review", "expected punctuation split");
  require(tokens[2] == "code", "expected second lowercase token");
  require(tokens[3] == "2026", "expected numeric token");
}

}  // namespace

int main() {
  test_tokenizer_normalizes_words();
  std::cout << "atlas tests passed\n";
  return 0;
}
