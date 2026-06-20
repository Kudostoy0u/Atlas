#include "atlas/index.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>

namespace atlas {
namespace {

constexpr double kBm25K1 = 1.2;
constexpr double kBm25B = 0.75;

double inverse_document_frequency(std::size_t document_count,
                                  std::size_t document_frequency) {
  const double numerator =
      static_cast<double>(document_count) - static_cast<double>(document_frequency) + 0.5;
  const double denominator = static_cast<double>(document_frequency) + 0.5;
  return std::log(1.0 + numerator / denominator);
}

}  // namespace

DocId InvertedIndex::add_document(std::string external_id,
                                  std::string title,
                                  std::string body) {
  if (documents_.size() > static_cast<std::size_t>(std::numeric_limits<DocId>::max())) {
    throw std::overflow_error("Atlas document id space exhausted");
  }

  const auto doc_id = static_cast<DocId>(documents_.size());
  add_document(Document{doc_id, std::move(external_id), std::move(title), std::move(body)});
  return doc_id;
}

void InvertedIndex::add_document(const Document& document) {
  if (document.id != documents_.size()) {
    throw std::invalid_argument("document ids must be dense and zero-based");
  }

  std::unordered_map<std::string, std::uint32_t> frequencies;
  auto tokens = tokenizer_.tokenize(document.title);
  auto body_tokens = tokenizer_.tokenize(document.body);
  tokens.insert(tokens.end(), body_tokens.begin(), body_tokens.end());

  for (const auto& token : tokens) {
    ++frequencies[token];
  }

  documents_.push_back(document);
  document_lengths_.push_back(static_cast<std::uint32_t>(tokens.size()));
  total_document_length_ += tokens.size();

  for (const auto& [term, frequency] : frequencies) {
    postings_[term].push_back(Posting{document.id, frequency});
  }
}

std::vector<SearchResult> InvertedIndex::search(std::string_view query,
                                                std::size_t limit) const {
  if (limit == 0 || documents_.empty()) {
    return {};
  }

  const auto query_terms = tokenizer_.tokenize(query);
  if (query_terms.empty()) {
    return {};
  }

  std::unordered_map<DocId, double> scores;
  const double avgdl = average_document_length();

  for (const auto& term : query_terms) {
    const auto postings_it = postings_.find(term);
    if (postings_it == postings_.end()) {
      continue;
    }

    const auto& postings = postings_it->second;
    const double idf = inverse_document_frequency(documents_.size(), postings.size());

    for (const auto& posting : postings) {
      const double tf = static_cast<double>(posting.term_frequency);
      const double dl = static_cast<double>(document_lengths_[posting.doc_id]);
      const double norm = tf + kBm25K1 * (1.0 - kBm25B + kBm25B * (dl / avgdl));
      scores[posting.doc_id] += idf * ((tf * (kBm25K1 + 1.0)) / norm);
    }
  }

  std::vector<SearchResult> results;
  results.reserve(scores.size());
  for (const auto& [doc_id, score] : scores) {
    const auto& document = documents_[doc_id];
    results.push_back(SearchResult{doc_id, document.external_id, document.title, score});
  }

  const auto better_result = [](const auto& lhs, const auto& rhs) {
    if (lhs.score != rhs.score) {
      return lhs.score > rhs.score;
    }
    return lhs.doc_id < rhs.doc_id;
  };

  if (results.size() > limit) {
    std::nth_element(results.begin(), results.begin() + static_cast<std::ptrdiff_t>(limit),
                     results.end(), better_result);
    results.resize(limit);
  }
  std::sort(results.begin(), results.end(), better_result);

  return results;
}

std::size_t InvertedIndex::document_count() const { return documents_.size(); }

std::size_t InvertedIndex::term_count() const { return postings_.size(); }

double InvertedIndex::average_document_length() const {
  if (documents_.empty()) {
    return 0.0;
  }
  return static_cast<double>(total_document_length_) / static_cast<double>(documents_.size());
}

const std::vector<Posting>* InvertedIndex::postings_for(std::string_view term) const {
  const auto normalized = Tokenizer::normalize(term);
  const auto it = postings_.find(normalized);
  if (it == postings_.end()) {
    return nullptr;
  }
  return &it->second;
}

const Document& InvertedIndex::document(DocId doc_id) const {
  return documents_.at(doc_id);
}

void InvertedIndex::replace_from_builder(std::vector<Document> documents,
                                         std::vector<BuilderPartialIndex> partials) {
  documents_ = std::move(documents);
  postings_.clear();
  document_lengths_.assign(documents_.size(), 0);
  total_document_length_ = 0;

  for (auto& partial : partials) {
    for (std::size_t i = 0; i < partial.lengths.size(); ++i) {
      if (partial.lengths[i] != 0) {
        document_lengths_[i] = partial.lengths[i];
      }
    }

    for (auto& [term, postings] : partial.postings) {
      auto& destination = postings_[term];
      destination.insert(destination.end(), postings.begin(), postings.end());
    }
  }

  for (auto& [term, postings] : postings_) {
    (void)term;
    std::sort(postings.begin(), postings.end(), [](const auto& lhs, const auto& rhs) {
      return lhs.doc_id < rhs.doc_id;
    });
  }

  for (const auto length : document_lengths_) {
    total_document_length_ += length;
  }
}

}  // namespace atlas
