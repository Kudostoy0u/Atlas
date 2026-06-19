# Atlas Benchmarks

Benchmarks were run locally with the release-style direct compiler command below
because this environment does not have CMake installed:

```sh
clang++ -std=c++20 -O3 -DNDEBUG -Wall -Wextra -Wpedantic -pthread -Iinclude \
  src/compression.cpp src/document.cpp src/tokenizer.cpp src/index.cpp \
  src/index_builder.cpp src/persistence.cpp bench/benchmark.cpp -o /tmp/atlas-bench
```

The harness generates a deterministic municipal-plan-review-style text corpus,
builds a serial index, builds an 8-worker parallel index, persists the compressed
index, reloads it through the mmap-backed loader, and measures top-10 BM25 query
latency across repeated queries.

## 100,000 documents

```text
documents              100000
raw_mib                96.31
serial_index_seconds   0.66
parallel_index_seconds 0.18
serial_mib_s           146.42
parallel_mib_s         549.76
speedup                3.75
persisted_mib          7.63
size_reduction_pct     92.08
top10_query_p95_ms     11.22
```

Command:

```sh
/tmp/atlas-bench 100000 120 8
```

## 250,000 documents

```text
documents              250000
raw_mib                240.97
serial_index_seconds   1.83
parallel_index_seconds 0.52
serial_mib_s           131.96
parallel_mib_s         459.40
speedup                3.48
persisted_mib          19.24
size_reduction_pct     92.02
top10_query_p95_ms     22.92
```

Command:

```sh
/tmp/atlas-bench 250000 120 8
```
