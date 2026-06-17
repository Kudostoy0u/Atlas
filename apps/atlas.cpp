#include "atlas/index_builder.hpp"
#include "atlas/persistence.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

void usage() {
  std::cerr
      << "usage:\n"
      << "  atlas index <documents.tsv> <index.atlas> [workers]\n"
      << "  atlas search <index.atlas> <query> [limit]\n"
      << "  atlas stats <index.atlas>\n\n"
      << "documents.tsv columns: external_id<TAB>title<TAB>body\n";
}

std::vector<atlas::Document> load_tsv(const std::filesystem::path& path) {
  std::ifstream in(path);
  if (!in) {
    throw std::runtime_error("failed to open corpus file");
  }

  std::vector<atlas::Document> documents;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }

    std::stringstream stream(line);
    std::string external_id;
    std::string title;
    std::string body;

    if (!std::getline(stream, external_id, '\t') || !std::getline(stream, title, '\t') ||
        !std::getline(stream, body)) {
      throw std::runtime_error("invalid TSV row; expected external_id, title, body");
    }

    documents.push_back(
        atlas::Document{static_cast<atlas::DocId>(documents.size()), std::move(external_id),
                        std::move(title), std::move(body)});
  }

  return documents;
}

int index_command(int argc, char** argv) {
  if (argc < 4 || argc > 5) {
    usage();
    return 2;
  }

  const auto corpus_path = std::filesystem::path(argv[2]);
  const auto index_path = std::filesystem::path(argv[3]);
  const std::size_t workers = argc == 5 ? static_cast<std::size_t>(std::stoul(argv[4])) : 0;

  const auto started = std::chrono::steady_clock::now();
  auto documents = load_tsv(corpus_path);
  const auto bytes = std::filesystem::file_size(corpus_path);

  const atlas::IndexBuilder builder(workers);
  auto index = builder.build(std::move(documents));
  atlas::save_index(index, index_path);

  const auto elapsed = std::chrono::steady_clock::now() - started;
  const double seconds = std::chrono::duration<double>(elapsed).count();
  const double mib = static_cast<double>(bytes) / (1024.0 * 1024.0);

  std::cout << "indexed " << index.document_count() << " documents, " << index.term_count()
            << " terms using " << builder.worker_count() << " workers in " << std::fixed
            << std::setprecision(3) << seconds << "s";
  if (seconds > 0.0) {
    std::cout << " (" << std::setprecision(2) << (mib / seconds) << " MiB/s)";
  }
  std::cout << '\n';
  return 0;
}

int search_command(int argc, char** argv) {
  if (argc < 4 || argc > 5) {
    usage();
    return 2;
  }

  const auto index = atlas::load_index(argv[2]);
  const std::size_t limit = argc == 5 ? static_cast<std::size_t>(std::stoul(argv[4])) : 10;

  const auto started = std::chrono::steady_clock::now();
  const auto results = index.search(argv[3], limit);
  const auto elapsed = std::chrono::steady_clock::now() - started;

  for (const auto& result : results) {
    std::cout << result.external_id << '\t' << std::fixed << std::setprecision(4)
              << result.score << '\t' << result.title << '\n';
  }
  std::cerr << "searched in "
            << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()
            << "us\n";
  return 0;
}

int stats_command(int argc, char** argv) {
  if (argc != 3) {
    usage();
    return 2;
  }

  const auto index = atlas::load_index(argv[2]);
  std::cout << "documents\t" << index.document_count() << '\n'
            << "terms\t" << index.term_count() << '\n'
            << "avg_document_length\t" << std::fixed << std::setprecision(2)
            << index.average_document_length() << '\n';
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    if (argc < 2) {
      usage();
      return 2;
    }

    const std::string command = argv[1];
    if (command == "index") {
      return index_command(argc, argv);
    }
    if (command == "search") {
      return search_command(argc, argv);
    }
    if (command == "stats") {
      return stats_command(argc, argv);
    }

    usage();
    return 2;
  } catch (const std::exception& error) {
    std::cerr << "atlas: " << error.what() << '\n';
    return 1;
  }
}
