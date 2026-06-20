#include "atlas/index_builder.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {

struct ServerState {
  atlas::InvertedIndex index;
  std::vector<std::string> lexicon;
  std::size_t raw_bytes{};
  std::size_t compressed_bytes{};
  std::size_t token_count{};
  double index_ms{};
  bool ready{};
};

std::vector<std::string> fallback_words() {
  return {"man", "woman", "boy", "girl", "help", "computer", "internet", "answer",
          "zebra", "banana", "airport", "music", "weather", "friend", "permit",
          "search", "document", "result", "clean", "solar", "clinic", "playground"};
}

std::vector<std::string> load_lexicon(const std::filesystem::path& path) {
  std::ifstream in(path);
  std::vector<std::string> words;
  std::string word;
  while (std::getline(in, word)) {
    if (!word.empty()) {
      words.push_back(word);
    }
  }
  if (words.empty()) {
    return fallback_words();
  }
  return words;
}

std::string json_escape(std::string_view value) {
  std::string out;
  out.reserve(value.size() + 8);
  for (const char ch : value) {
    switch (ch) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(ch) < 0x20) {
          out += ' ';
        } else {
          out += ch;
        }
    }
  }
  return out;
}

std::string url_decode(std::string_view value) {
  std::string out;
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '%' && i + 2 < value.size()) {
      const std::string hex{value.substr(i + 1, 2)};
      out.push_back(static_cast<char>(std::stoi(hex, nullptr, 16)));
      i += 2;
    } else if (value[i] == '+') {
      out.push_back(' ');
    } else {
      out.push_back(value[i]);
    }
  }
  return out;
}

std::unordered_map<std::string, std::string> query_params(std::string_view target) {
  std::unordered_map<std::string, std::string> params;
  const auto question = target.find('?');
  if (question == std::string_view::npos) {
    return params;
  }

  std::string_view query = target.substr(question + 1);
  while (!query.empty()) {
    const auto amp = query.find('&');
    const auto part = query.substr(0, amp);
    const auto equals = part.find('=');
    if (equals != std::string_view::npos) {
      params.emplace(std::string(part.substr(0, equals)), url_decode(part.substr(equals + 1)));
    }
    if (amp == std::string_view::npos) {
      break;
    }
    query = query.substr(amp + 1);
  }
  return params;
}

std::string pick(const std::vector<std::string>& words, std::mt19937& rng) {
  std::uniform_int_distribution<std::size_t> dist(0, words.size() - 1);
  return words[dist(rng)];
}

std::string sentence(const std::vector<std::string>& words, std::mt19937& rng) {
  static const std::vector<std::string> determiners{"the", "a", "this", "each", "another"};
  static const std::vector<std::string> verbs{"links", "checks", "ranks", "helps", "routes",
                                              "measures", "records", "supports", "compares",
                                              "opens", "answers", "guides", "filters"};
  static const std::vector<std::string> preps{"near", "inside", "around", "before", "after",
                                              "with", "through", "beside", "under"};
  auto phrase = [&] {
    return pick(determiners, rng) + " " + pick(words, rng) + " " + pick(words, rng);
  };
  std::string text = phrase() + " " + pick(verbs, rng) + " " + phrase();
  if (std::uniform_int_distribution<int>(0, 99)(rng) < 65) {
    text += " " + pick(preps, rng) + " " + phrase();
  }
  if (std::uniform_int_distribution<int>(0, 99)(rng) < 35) {
    text += " while " + phrase() + " " + pick(verbs, rng) + " " + phrase();
  }
  text[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
  return text + ".";
}

std::vector<atlas::Document> generate_documents(std::size_t count,
                                                const std::vector<std::string>& lexicon) {
  static const std::vector<std::string> departments{
      "Planning", "Fire Prevention", "Stormwater", "Transportation", "Public Works",
      "Building Safety", "Environmental Review", "Utilities", "Housing", "Parks"};
  static const std::vector<std::string> jurisdictions{
      "Oak Meadow", "Northfield", "San Paloma", "Rivergate", "Lakeview", "Cedar Falls",
      "Westport", "Summit Ridge", "Marina Point", "Eastbank"};

  std::mt19937 rng(20260620);
  std::vector<atlas::Document> docs;
  docs.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    const auto year = 2020 + static_cast<int>(i % 7);
    const auto title = jurisdictions[i % jurisdictions.size()] + " " + pick(lexicon, rng) +
                       " review packet " + std::to_string(year);
    std::string body;
    for (int paragraph = 0; paragraph < 2; ++paragraph) {
      if (paragraph != 0) {
        body += "\n\n";
      }
      const int sentence_count = 5 + static_cast<int>(i + paragraph) % 4;
      for (int s = 0; s < sentence_count; ++s) {
        if (s != 0) {
          body += ' ';
        }
        body += sentence(lexicon, rng);
      }
    }
    docs.push_back(atlas::Document{static_cast<atlas::DocId>(i),
                                   "AT-" + std::to_string(year) + "-" + std::to_string(i + 1),
                                   title + " | " + departments[i % departments.size()],
                                   std::move(body)});
  }
  return docs;
}

std::string snippet(std::string_view body, std::string_view query) {
  const auto lower_body = [&] {
    std::string value(body);
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
      return static_cast<char>(std::tolower(ch));
    });
    return value;
  }();
  std::string lower_query(query);
  std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  const auto pos = lower_body.find(lower_query);
  const auto start = pos == std::string::npos ? 0 : pos > 90 ? pos - 90 : 0;
  return std::string(body.substr(start, 260));
}

std::size_t estimate_compressed_bytes(const atlas::InvertedIndex& index,
                                      const std::vector<std::string>& lexicon) {
  std::size_t bytes = index.document_count() * 24;
  for (const auto& term : lexicon) {
    if (const auto* postings = index.postings_for(term)) {
      bytes += term.size() + postings->size() * 2 + 8;
    }
  }
  return bytes;
}

std::string build(ServerState& state, std::size_t count) {
  const auto started = std::chrono::steady_clock::now();
  auto docs = generate_documents(count, state.lexicon);
  state.raw_bytes = 0;
  state.token_count = 0;
  for (const auto& doc : docs) {
    state.raw_bytes += doc.external_id.size() + doc.title.size() + doc.body.size();
    state.token_count += doc.body.size() / 6;
  }
  state.index = atlas::IndexBuilder(8).build(std::move(docs));
  state.compressed_bytes = estimate_compressed_bytes(state.index, state.lexicon);
  state.index_ms =
      std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
  state.ready = true;

  std::ostringstream json;
  json << "{\"documents\":" << state.index.document_count()
       << ",\"terms\":" << state.index.term_count()
       << ",\"tokens\":" << state.token_count
       << ",\"rawBytes\":" << state.raw_bytes
       << ",\"compressedBytes\":" << state.compressed_bytes
       << ",\"indexMs\":" << std::fixed << std::setprecision(2) << state.index_ms
       << ",\"throughputMiBs\":"
       << (static_cast<double>(state.raw_bytes) / 1024.0 / 1024.0 / (state.index_ms / 1000.0))
       << ",\"compressionReduction\":"
       << (1.0 - static_cast<double>(state.compressed_bytes) /
                    static_cast<double>(std::max<std::size_t>(1, state.raw_bytes)))
       << "}";
  return json.str();
}

std::string search(ServerState& state, const std::string& query, std::size_t limit) {
  const auto started = std::chrono::steady_clock::now();
  const auto results = state.index.search(query, limit);
  const auto latency =
      std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();

  std::ostringstream json;
  json << "{\"latencyMs\":" << std::fixed << std::setprecision(3) << latency << ",\"results\":[";
  for (std::size_t i = 0; i < results.size(); ++i) {
    const auto& result = results[i];
    const auto& doc = state.index.document(result.doc_id);
    const auto pipe = doc.title.find(" | ");
    const auto title = pipe == std::string::npos ? doc.title : doc.title.substr(0, pipe);
    const auto department = pipe == std::string::npos ? "" : doc.title.substr(pipe + 3);
    if (i != 0) {
      json << ',';
    }
    json << "{\"id\":" << result.doc_id
         << ",\"externalId\":\"" << json_escape(doc.external_id)
         << "\",\"title\":\"" << json_escape(title)
         << "\",\"department\":\"" << json_escape(department)
         << "\",\"jurisdiction\":\"Generated"
         << "\",\"year\":2026,\"pages\":2,\"score\":" << std::setprecision(4) << result.score
         << ",\"snippet\":\"" << json_escape(snippet(doc.body, query))
         << "\",\"body\":\"" << json_escape(doc.body) << "\"}";
  }
  json << "]}";
  return json.str();
}

void send_response(int client, std::string body, std::string status = "200 OK") {
  std::ostringstream response;
  response << "HTTP/1.1 " << status << "\r\n"
           << "Content-Type: application/json\r\n"
           << "Access-Control-Allow-Origin: *\r\n"
           << "Content-Length: " << body.size() << "\r\n"
           << "Connection: close\r\n\r\n"
           << body;
  const auto data = response.str();
  (void)::send(client, data.data(), data.size(), 0);
}

}  // namespace

int main(int argc, char** argv) {
  ServerState state;
  const auto lexicon_path = argc > 1 ? std::filesystem::path(argv[1]) : std::filesystem::path("data/lexicon.txt");
  state.lexicon = load_lexicon(lexicon_path);

  const int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    throw std::runtime_error("failed to create socket");
  }
  int yes = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons(8787);
  if (::bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
    throw std::runtime_error("failed to bind 127.0.0.1:8787");
  }
  if (::listen(server_fd, 16) < 0) {
    throw std::runtime_error("failed to listen");
  }

  std::cerr << "atlas demo server listening on http://127.0.0.1:8787\n";
  while (true) {
    const int client = ::accept(server_fd, nullptr, nullptr);
    if (client < 0) {
      continue;
    }
    char buffer[8192];
    const auto received = ::recv(client, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
      ::close(client);
      continue;
    }
    buffer[received] = '\0';
    std::istringstream request(buffer);
    std::string method;
    std::string target;
    request >> method >> target;
    try {
      if (target.starts_with("/api/build")) {
        const auto params = query_params(target);
        const auto count = params.contains("count") ? std::stoul(params.at("count")) : 65000UL;
        send_response(client, build(state, count));
      } else if (target.starts_with("/api/search")) {
        const auto params = query_params(target);
        const auto query = params.contains("q") ? params.at("q") : "";
        const auto limit = params.contains("limit") ? std::stoul(params.at("limit")) : 10UL;
        send_response(client, state.ready ? search(state, query, limit) : "{\"latencyMs\":0,\"results\":[]}");
      } else {
        send_response(client, "{\"error\":\"not found\"}", "404 Not Found");
      }
    } catch (const std::exception& error) {
      send_response(client, std::string("{\"error\":\"") + json_escape(error.what()) + "\"}",
                    "500 Internal Server Error");
    }
    ::close(client);
  }
}
