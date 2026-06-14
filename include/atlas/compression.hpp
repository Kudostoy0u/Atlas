#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace atlas {

void encode_varint(std::uint32_t value, std::vector<std::uint8_t>& out);
std::uint32_t decode_varint(std::span<const std::uint8_t> bytes, std::size_t& offset);

std::vector<std::uint8_t> encode_delta_varints(std::span<const std::uint32_t> values);
std::vector<std::uint32_t> decode_delta_varints(std::span<const std::uint8_t> bytes);

}  // namespace atlas
