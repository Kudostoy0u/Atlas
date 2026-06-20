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

## Benchmark

```sh
clang++ -std=c++20 -O3 -DNDEBUG -Wall -Wextra -Wpedantic -pthread -Iinclude \
  src/*.cpp bench/benchmark.cpp -o /tmp/atlas-bench
/tmp/atlas-bench 100000 120 8
```

Recent local results are recorded in [docs/BENCHMARKS.md](docs/BENCHMARKS.md).

## Frontend Demo

Atlas also includes a Vite, TypeScript, and React demo that generates a large
document corpus, sends indexing/search work to a native C++ Atlas demo server,
and shows live throughput, compression, and top-k search latency.

```sh
npm install
npm run build:lexicon
npm run dev
```

`npm run dev` compiles `apps/demo_server.cpp`, starts the native C++ API on
`127.0.0.1:8787`, and starts Vite on `127.0.0.1:5173`. The React app only owns
the UI; indexing, BM25 scoring, and result retrieval are handled by C++.
