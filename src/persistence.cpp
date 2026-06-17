#include "atlas/persistence.hpp"

#include "atlas/compression.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string_view>

namespace atlas {
namespace {

constexpr std::string_view kMagic = "ATLASIDX1";

class MappedFile {
 public:
  explicit MappedFile(const std::filesystem::path& path) {
    fd_ = ::open(path.c_str(), O_RDONLY);
    if (fd_ < 0) {
      throw std::runtime_error("failed to open index file");
    }

    struct stat statbuf {};
    if (::fstat(fd_, &statbuf) != 0) {
      ::close(fd_);
      throw std::runtime_error("failed to stat index file");
    }

    size_ = static_cast<std::size_t>(statbuf.st_size);
    if (size_ == 0) {
      ::close(fd_);
      throw std::runtime_error("index file is empty");
    }

    data_ = static_cast<const std::uint8_t*>(
        ::mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd_, 0));
    if (data_ == MAP_FAILED) {
      ::close(fd_);
      throw std::runtime_error("failed to memory-map index file");
    }
  }

  MappedFile(const MappedFile&) = delete;
  MappedFile& operator=(const MappedFile&) = delete;

  ~MappedFile() {
    if (data_ != nullptr && data_ != MAP_FAILED) {
      ::munmap(const_cast<std::uint8_t*>(data_), size_);
    }
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }

  [[nodiscard]] std::span<const std::uint8_t> bytes() const { return {data_, size_}; }

 private:
  int fd_{-1};
  const std::uint8_t* data_{nullptr};
  std::size_t size_{};
};

void write_u32(std::ostream& out, std::uint32_t value) {
  std::uint8_t bytes[4]{
      static_cast<std::uint8_t>(value & 0xFF),
      static_cast<std::uint8_t>((value >> 8) & 0xFF),
      static_cast<std::uint8_t>((value >> 16) & 0xFF),
      static_cast<std::uint8_t>((value >> 24) & 0xFF),
  };
  out.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}

std::uint32_t read_u32(std::span<const std::uint8_t> bytes, std::size_t& offset) {
  if (offset + 4 > bytes.size()) {
    throw std::runtime_error("truncated index file");
  }
  const std::uint32_t value = static_cast<std::uint32_t>(bytes[offset]) |
                              (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
                              (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
                              (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
  offset += 4;
  return value;
}

void write_string(std::ostream& out, const std::string& value) {
  write_u32(out, static_cast<std::uint32_t>(value.size()));
  out.write(value.data(), static_cast<std::streamsize>(value.size()));
}

std::string read_string(std::span<const std::uint8_t> bytes, std::size_t& offset) {
  const auto size = read_u32(bytes, offset);
  if (offset + size > bytes.size()) {
    throw std::runtime_error("truncated index string");
  }
  std::string value(reinterpret_cast<const char*>(bytes.data() + offset), size);
  offset += size;
  return value;
}

void write_bytes(std::ostream& out, std::span<const std::uint8_t> bytes) {
  write_u32(out, static_cast<std::uint32_t>(bytes.size()));
  out.write(reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
}

std::span<const std::uint8_t> read_bytes(std::span<const std::uint8_t> bytes,
                                         std::size_t& offset) {
  const auto size = read_u32(bytes, offset);
  if (offset + size > bytes.size()) {
    throw std::runtime_error("truncated index byte array");
  }
  const auto result = bytes.subspan(offset, size);
  offset += size;
  return result;
}

}  // namespace

void save_index(const InvertedIndex& index, const std::filesystem::path& path) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out) {
    throw std::runtime_error("failed to create index file");
  }

  out.write(kMagic.data(), static_cast<std::streamsize>(kMagic.size()));
  write_u32(out, static_cast<std::uint32_t>(index.documents_.size()));
  write_u32(out, static_cast<std::uint32_t>(index.postings_.size()));

  for (std::size_t i = 0; i < index.documents_.size(); ++i) {
    const auto& document = index.documents_[i];
    write_string(out, document.external_id);
    write_string(out, document.title);
    write_u32(out, index.document_lengths_[i]);
  }

  for (const auto& [term, postings] : index.postings_) {
    std::vector<std::uint32_t> doc_ids;
    std::vector<std::uint8_t> frequencies;
    doc_ids.reserve(postings.size());
    frequencies.reserve(postings.size());

    for (const auto& posting : postings) {
      doc_ids.push_back(posting.doc_id);
      encode_varint(posting.term_frequency, frequencies);
    }

    const auto encoded_doc_ids = encode_delta_varints(doc_ids);
    write_string(out, term);
    write_bytes(out, encoded_doc_ids);
    write_bytes(out, frequencies);
  }
}

InvertedIndex load_index(const std::filesystem::path& path) {
  const MappedFile mapped(path);
  const auto bytes = mapped.bytes();
  std::size_t offset = 0;

  if (bytes.size() < kMagic.size() ||
      std::memcmp(bytes.data(), kMagic.data(), kMagic.size()) != 0) {
    throw std::runtime_error("invalid Atlas index file");
  }
  offset += kMagic.size();

  InvertedIndex index;
  const auto document_count = read_u32(bytes, offset);
  const auto term_count = read_u32(bytes, offset);

  index.documents_.reserve(document_count);
  index.document_lengths_.reserve(document_count);
  for (std::uint32_t i = 0; i < document_count; ++i) {
    Document document;
    document.id = i;
    document.external_id = read_string(bytes, offset);
    document.title = read_string(bytes, offset);
    index.documents_.push_back(std::move(document));

    const auto length = read_u32(bytes, offset);
    index.document_lengths_.push_back(length);
    index.total_document_length_ += length;
  }

  for (std::uint32_t i = 0; i < term_count; ++i) {
    auto term = read_string(bytes, offset);
    const auto doc_ids = decode_delta_varints(read_bytes(bytes, offset));
    const auto frequencies = read_bytes(bytes, offset);

    std::size_t frequency_offset = 0;
    auto& postings = index.postings_[std::move(term)];
    postings.reserve(doc_ids.size());
    for (const auto doc_id : doc_ids) {
      postings.push_back(Posting{doc_id, decode_varint(frequencies, frequency_offset)});
    }
  }

  return index;
}

}  // namespace atlas
