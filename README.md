# Atlas

Atlas is a C++20 full-text search engine focused on fast local indexing and
low-latency BM25 retrieval over document collections.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```
