#pragma once

#include "atlas/document.hpp"
#include "atlas/tokenizer.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas {

struct Posting {
  DocId doc_id{};
  std::uint32_t term_frequency{};
};

struct SearchResult {
  DocId doc_id{};
  std::string external_id;
  std::string title;
  double score{};
};

class InvertedIndex {
 public:
  DocId add_document(std::string external_id, std::string title, std::string body);
  void add_document(const Document& document);

  [[nodiscard]] std::vector<SearchResult> search(std::string_view query,
                                                 std::size_t limit = 10) const;
  [[nodiscard]] std::size_t document_count() const;
  [[nodiscard]] std::size_t term_count() const;
  [[nodiscard]] double average_document_length() const;
  [[nodiscard]] const std::vector<Posting>* postings_for(std::string_view term) const;

 private:
  std::vector<Document> documents_;
  std::vector<std::uint32_t> document_lengths_;
  std::unordered_map<std::string, std::vector<Posting>> postings_;
  Tokenizer tokenizer_;
  std::uint64_t total_document_length_{};
};

}  // namespace atlas
