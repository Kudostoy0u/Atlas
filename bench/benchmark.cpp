#include "atlas/index.hpp"
#include "atlas/index_builder.hpp"
#include "atlas/persistence.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

std::vector<atlas::Document> generate_documents(std::size_t count,
                                                std::size_t terms_per_document) {
  const std::vector<std::string> topics{
      "stormwater", "drainage", "basin",     "sprinkler", "alarm",   "zoning",
      "setback",    "height",   "inspection", "permit",    "parcel",  "egress",
      "structural", "beam",     "foundation", "grading",   "utility", "parking",
  };

  std::mt19937 rng(42);
  std::uniform_int_distribution<std::size_t> topic_dist(0, topics.size() - 1);
  std::vector<atlas::Document> documents;
  documents.reserve(count);

  for (std::size_t i = 0; i < count; ++i) {
    std::string body;
    body.reserve(terms_per_document * 12);
    for (std::size_t term = 0; term < terms_per_document; ++term) {
      body += topics[(topic_dist(rng) + i + term) % topics.size()];
      body.push_back(' ');
    }

    documents.push_back(atlas::Document{
        static_cast<atlas::DocId>(i),
        "doc-" + std::to_string(i),
        "permit packet " + topics[i % topics.size()],
        std::move(body),
    });
  }

  return documents;
}

template <typename Fn>
double seconds_for(Fn&& fn) {
  const auto started = Clock::now();
  fn();
  return std::chrono::duration<double>(Clock::now() - started).count();
}

double p95_ms(std::vector<double> values) {
  std::sort(values.begin(), values.end());
  const std::size_t index =
      static_cast<std::size_t>(static_cast<double>(values.size() - 1) * 0.95);
  return values[index];
}

std::uintmax_t raw_bytes(const std::vector<atlas::Document>& documents) {
  std::uintmax_t total = 0;
  for (const auto& document : documents) {
    total += document.external_id.size() + document.title.size() + document.body.size();
  }
  return total;
}

}  // namespace

int main(int argc, char** argv) {
  const std::size_t document_count = argc > 1 ? std::stoull(argv[1]) : 50'000;
  const std::size_t terms_per_document = argc > 2 ? std::stoull(argv[2]) : 120;
  const std::size_t workers = argc > 3 ? std::stoull(argv[3]) : 8;

  auto documents = generate_documents(document_count, terms_per_document);
  atlas::InvertedIndex serial;

  const double serial_seconds = seconds_for([&] {
    for (const auto& document : documents) {
      serial.add_document(document);
    }
  });

  atlas::InvertedIndex parallel;
  const double parallel_seconds = seconds_for([&] {
    const atlas::IndexBuilder builder(workers);
    parallel = builder.build(documents);
  });

  const auto path = std::filesystem::temp_directory_path() / "atlas-bench-index.atlas";
  atlas::save_index(parallel, path);
  const auto persisted_bytes = std::filesystem::file_size(path);
  const auto loaded = atlas::load_index(path);
  std::filesystem::remove(path);

  const std::vector<std::string> queries{
      "stormwater basin", "sprinkler alarm", "zoning setback", "foundation beam",
      "parking utility",  "egress inspection"};
  std::vector<double> latencies_ms;
  latencies_ms.reserve(queries.size() * 100);
  for (int round = 0; round < 100; ++round) {
    for (const auto& query : queries) {
      const auto started = Clock::now();
      (void)loaded.search(query, 10);
      const auto elapsed = Clock::now() - started;
      latencies_ms.push_back(std::chrono::duration<double, std::milli>(elapsed).count());
    }
  }

  const double raw_mib = static_cast<double>(raw_bytes(documents)) / (1024.0 * 1024.0);
  const double serial_mib_s = raw_mib / serial_seconds;
  const double parallel_mib_s = raw_mib / parallel_seconds;
  const double compression_reduction =
      1.0 - (static_cast<double>(persisted_bytes) / static_cast<double>(raw_bytes(documents)));

  std::cout << std::fixed << std::setprecision(2)
            << "documents\t" << document_count << '\n'
            << "raw_mib\t" << raw_mib << '\n'
            << "serial_index_seconds\t" << serial_seconds << '\n'
            << "parallel_index_seconds\t" << parallel_seconds << '\n'
            << "serial_mib_s\t" << serial_mib_s << '\n'
            << "parallel_mib_s\t" << parallel_mib_s << '\n'
            << "speedup\t" << (serial_seconds / parallel_seconds) << '\n'
            << "persisted_mib\t" << (static_cast<double>(persisted_bytes) / (1024.0 * 1024.0))
            << '\n'
            << "size_reduction_pct\t" << (compression_reduction * 100.0) << '\n'
            << "top10_query_p95_ms\t" << p95_ms(latencies_ms) << '\n';

  return 0;
}
