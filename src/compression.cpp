#include "atlas/compression.hpp"

#include <stdexcept>

namespace atlas {

void encode_varint(std::uint32_t value, std::vector<std::uint8_t>& out) {
  while (value >= 0x80) {
    out.push_back(static_cast<std::uint8_t>((value & 0x7F) | 0x80));
    value >>= 7;
  }
  out.push_back(static_cast<std::uint8_t>(value));
}

std::uint32_t decode_varint(std::span<const std::uint8_t> bytes, std::size_t& offset) {
  std::uint32_t value = 0;
  std::uint32_t shift = 0;

  while (offset < bytes.size()) {
    const std::uint8_t byte = bytes[offset++];
    value |= static_cast<std::uint32_t>(byte & 0x7F) << shift;
    if ((byte & 0x80) == 0) {
      return value;
    }

    shift += 7;
    if (shift >= 32) {
      throw std::runtime_error("invalid varint: value exceeds 32 bits");
    }
  }

  throw std::runtime_error("invalid varint: truncated byte stream");
}

std::vector<std::uint8_t> encode_delta_varints(std::span<const std::uint32_t> values) {
  std::vector<std::uint8_t> encoded;
  encoded.reserve(values.size());

  std::uint32_t previous = 0;
  for (const std::uint32_t value : values) {
    if (value < previous) {
      throw std::invalid_argument("delta encoding requires sorted values");
    }
    encode_varint(value - previous, encoded);
    previous = value;
  }

  return encoded;
}

std::vector<std::uint32_t> decode_delta_varints(std::span<const std::uint8_t> bytes) {
  std::vector<std::uint32_t> values;
  std::size_t offset = 0;
  std::uint32_t previous = 0;

  while (offset < bytes.size()) {
    const std::uint32_t delta = decode_varint(bytes, offset);
    previous += delta;
    values.push_back(previous);
  }

  return values;
}

}  // namespace atlas
