# Atlas

Atlas is a C++20 full-text search engine focused on fast local indexing and
low-latency BM25 retrieval over document collections.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

If CMake is unavailable, the core test suite can be compiled directly:

```sh
clang++ -std=c++20 -Wall -Wextra -Wpedantic -pthread -Iinclude \
  src/*.cpp tests/test_main.cpp -o /tmp/atlas-tests
/tmp/atlas-tests
```

## CLI

Atlas indexes tab-separated documents with three columns:

```text
external_id<TAB>title<TAB>body
```

```sh
./build/atlas index data/sample.tsv sample.atlas 8
./build/atlas search sample.atlas "stormwater basin" 10
./build/atlas stats sample.atlas
```
