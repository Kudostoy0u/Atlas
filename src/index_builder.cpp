#include "atlas/index_builder.hpp"

#include "atlas/tokenizer.hpp"

#include <algorithm>
#include <atomic>
#include <thread>
#include <unordered_map>

namespace atlas {
namespace {

std::size_t choose_worker_count(std::size_t requested) {
  if (requested != 0) {
    return requested;
  }

  const auto hardware = std::thread::hardware_concurrency();
  return std::max<std::size_t>(1, hardware == 0 ? 1 : hardware);
}

}  // namespace

IndexBuilder::IndexBuilder(std::size_t worker_count)
    : worker_count_(choose_worker_count(worker_count)) {}

InvertedIndex IndexBuilder::build(std::vector<Document> documents) const {
  for (std::size_t i = 0; i < documents.size(); ++i) {
    documents[i].id = static_cast<DocId>(i);
  }

  std::vector<BuilderPartialIndex> partials(worker_count_);
  for (auto& partial : partials) {
    partial.lengths.resize(documents.size());
  }

  std::atomic<std::size_t> next{0};
  std::vector<std::thread> threads;
  threads.reserve(worker_count_);

  for (std::size_t worker = 0; worker < worker_count_; ++worker) {
    threads.emplace_back([&, worker] {
      const Tokenizer tokenizer;
      auto& partial = partials[worker];

      while (true) {
        const std::size_t index = next.fetch_add(1, std::memory_order_relaxed);
        if (index >= documents.size()) {
          break;
        }

        const auto& document = documents[index];
        std::unordered_map<std::string, std::uint32_t> frequencies;

        auto tokens = tokenizer.tokenize(document.title);
        auto body_tokens = tokenizer.tokenize(document.body);
        tokens.insert(tokens.end(), body_tokens.begin(), body_tokens.end());

        for (const auto& token : tokens) {
          ++frequencies[token];
        }

        partial.lengths[index] = static_cast<std::uint32_t>(tokens.size());
        for (const auto& [term, frequency] : frequencies) {
          partial.postings[term].push_back(Posting{document.id, frequency});
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  InvertedIndex index;
  index.replace_from_builder(std::move(documents), std::move(partials));
  return index;
}

std::size_t IndexBuilder::worker_count() const { return worker_count_; }

}  // namespace atlas
