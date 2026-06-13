#include "atlas/tokenizer.hpp"
#include "atlas/index.hpp"

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

void test_index_ranks_matching_documents() {
  atlas::InvertedIndex index;
  index.add_document("permit-1", "Stormwater plan", "Drainage detention basin details");
  index.add_document("permit-2", "Fire plan", "Sprinkler riser and alarm details");
  index.add_document("permit-3", "Drainage report", "Stormwater basin basin basin");

  const auto results = index.search("stormwater basin", 2);

  require(results.size() == 2, "expected two search hits");
  require(results[0].external_id == "permit-3", "expected stronger term frequency to rank first");
  require(results[1].external_id == "permit-1", "expected second matching document");
  require(index.document_count() == 3, "expected indexed document count");
  require(index.term_count() > 0, "expected indexed terms");
}

}  // namespace

int main() {
  test_tokenizer_normalizes_words();
  test_index_ranks_matching_documents();
  std::cout << "atlas tests passed\n";
  return 0;
}
