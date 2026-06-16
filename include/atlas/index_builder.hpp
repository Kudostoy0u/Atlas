#pragma once

#include "atlas/document.hpp"
#include "atlas/index.hpp"

#include <cstddef>
#include <vector>

namespace atlas {

class IndexBuilder {
 public:
  explicit IndexBuilder(std::size_t worker_count = 0);

  [[nodiscard]] InvertedIndex build(std::vector<Document> documents) const;
  [[nodiscard]] std::size_t worker_count() const;

 private:
  std::size_t worker_count_;
};

}  // namespace atlas
