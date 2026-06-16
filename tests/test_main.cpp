#include "atlas/compression.hpp"
#include "atlas/index_builder.hpp"
#include "atlas/persistence.hpp"
#include "atlas/tokenizer.hpp"
#include "atlas/index.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>
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

void test_delta_varint_round_trip() {
  const std::vector<std::uint32_t> values{1, 2, 127, 128, 16'384, 16'400};
  const auto encoded = atlas::encode_delta_varints(values);
  const auto decoded = atlas::decode_delta_varints(encoded);

  require(decoded == values, "expected delta varint round trip");
  require(encoded.size() < values.size() * sizeof(std::uint32_t),
          "expected compressed byte stream to be smaller than raw uint32 ids");
}

void test_parallel_builder_matches_serial_search() {
  std::vector<atlas::Document> documents{
      {0, "a", "Alpha permit", "stormwater drainage basin"},
      {1, "b", "Beta permit", "fire alarm sprinkler"},
      {2, "c", "Gamma permit", "stormwater basin basin"},
      {3, "d", "Delta permit", "zoning setback height"},
  };

  atlas::InvertedIndex serial;
  for (const auto& document : documents) {
    serial.add_document(document);
  }

  const atlas::IndexBuilder builder(2);
  const auto parallel = builder.build(documents);
  const auto serial_results = serial.search("stormwater basin", 10);
  const auto parallel_results = parallel.search("stormwater basin", 10);

  require(parallel.document_count() == serial.document_count(),
          "expected parallel document count to match serial index");
  require(parallel.term_count() == serial.term_count(),
          "expected parallel term count to match serial index");
  require(parallel_results.size() == serial_results.size(),
          "expected parallel result count to match serial result count");
  require(parallel_results[0].external_id == serial_results[0].external_id,
          "expected parallel top result to match serial top result");
}

void test_persistence_round_trip() {
  atlas::InvertedIndex index;
  index.add_document("permit-1", "Stormwater plan", "Drainage basin details");
  index.add_document("permit-2", "Fire plan", "Sprinkler alarm details");

  const auto path = std::filesystem::temp_directory_path() / "atlas-test-index.bin";
  atlas::save_index(index, path);
  const auto loaded = atlas::load_index(path);
  std::filesystem::remove(path);

  const auto results = loaded.search("stormwater basin", 10);
  require(loaded.document_count() == 2, "expected persisted document count");
  require(results.size() == 1, "expected persisted search hit");
  require(results[0].external_id == "permit-1", "expected persisted top hit");
}

}  // namespace

int main() {
  test_tokenizer_normalizes_words();
  test_index_ranks_matching_documents();
  test_delta_varint_round_trip();
  test_parallel_builder_matches_serial_search();
  test_persistence_round_trip();
  std::cout << "atlas tests passed\n";
  return 0;
}
