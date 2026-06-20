import { Database, Loader2, Search, X } from "lucide-react";
import { useEffect, useMemo, useRef, useState } from "react";
import type { IndexStats, SearchResult } from "./types";

function formatNumber(value: number) {
  return new Intl.NumberFormat("en-US").format(Math.round(value));
}

function formatBytes(bytes: number) {
  if (bytes < 1024 * 1024) {
    return `${(bytes / 1024).toFixed(1)} KiB`;
  }
  return `${(bytes / 1024 / 1024).toFixed(1)} MiB`;
}

function CompactMetric({ label, value }: { label: string; value: string }) {
  return (
    <span className="metric">
      <span>{label}</span>
      <strong>{value}</strong>
    </span>
  );
}

export function App() {
  const workerRef = useRef<Worker | null>(null);
  const requestRef = useRef(0);
  const [stats, setStats] = useState<IndexStats | null>(null);
  const [query, setQuery] = useState("stormwater detention basin");
  const [results, setResults] = useState<SearchResult[]>([]);
  const [selectedResult, setSelectedResult] = useState<SearchResult | null>(null);
  const [latency, setLatency] = useState<number | null>(null);
  const [isIndexing, setIsIndexing] = useState(true);

  useEffect(() => {
    const worker = new Worker(new URL("./engine.worker.ts", import.meta.url), {
      type: "module"
    });
    workerRef.current = worker;

    worker.onmessage = (event) => {
      const { type, stats: nextStats, results: nextResults, latencyMs } = event.data;
      if (type === "built") {
        setStats(nextStats);
        setIsIndexing(false);
        search(query);
      }
      if (type === "results") {
        setResults(nextResults);
        setLatency(latencyMs);
        setSelectedResult((current) => {
          if (!current) {
            return null;
          }
          return nextResults.find((result: SearchResult) => result.id === current.id) ?? null;
        });
      }
    };

    post("build", { count: 65000 });
    return () => worker.terminate();
  }, []);

  useEffect(() => {
    if (!stats) {
      return;
    }
    const timeout = window.setTimeout(() => search(query), 80);
    return () => window.clearTimeout(timeout);
  }, [query, stats]);

  function post(type: string, payload: unknown) {
    workerRef.current?.postMessage({ type, payload, requestId: requestRef.current++ });
  }

  function search(nextQuery: string) {
    if (!nextQuery.trim()) {
      setResults([]);
      setLatency(null);
      return;
    }
    post("search", { query: nextQuery, limit: 10 });
  }

  const subtitle = useMemo(() => {
    if (!stats) {
      return "Building generated review corpus";
    }
    return `${formatNumber(stats.documents)} documents indexed`;
  }, [stats]);

  return (
    <main className="page">
      <header className="searchHeader">
        <div className="brand">
          <Database size={18} />
          <div>
            <strong>Atlas</strong>
            <span>{subtitle}</span>
          </div>
        </div>

        <label className="searchBox">
          <Search size={19} />
          <input
            autoFocus
            value={query}
            onChange={(event) => setQuery(event.target.value)}
            placeholder="Search generated plan-review documents"
            spellCheck={false}
          />
          {isIndexing && <Loader2 className="spin" size={18} />}
        </label>

        <div className="status">
          <CompactMetric
            label="Index"
            value={stats ? `${stats.indexMs.toFixed(0)} ms` : "building"}
          />
          <CompactMetric
            label="Size"
            value={stats ? formatBytes(stats.compressedBytes) : "-"}
          />
          <CompactMetric
            label="Query"
            value={latency === null ? "-" : `${latency.toFixed(1)} ms`}
          />
        </div>
      </header>

      <section className="content">
        <div className="summary">
          <span>
            {isIndexing
              ? "Indexing documents"
              : `${results.length} results for "${query || "empty query"}"`}
          </span>
          {stats && (
            <span>
              {formatNumber(stats.tokens)} tokens · {stats.throughputMiBs.toFixed(0)} MiB/s ·{" "}
              {(stats.compressionReduction * 100).toFixed(1)}% smaller
            </span>
          )}
        </div>

        <div className="results">
          {results.map((result, index) => (
            <article
              className="result"
              key={result.externalId}
              onClick={() => setSelectedResult(result)}
              tabIndex={0}
              onKeyDown={(event) => {
                if (event.key === "Enter" || event.key === " ") {
                  setSelectedResult(result);
                }
              }}
            >
              <div className="resultRank">{index + 1}</div>
              <div className="resultMain">
                <div className="meta">
                  <span>{result.externalId}</span>
                  <span>{result.department}</span>
                  <span>{result.jurisdiction}</span>
                  <span>{result.pages} pages</span>
                </div>
                <h2>{result.title}</h2>
                <p>{result.snippet}</p>
              </div>
              <div className="score">
                <span>BM25</span>
                <strong>{result.score.toFixed(2)}</strong>
              </div>
            </article>
          ))}
        </div>

        {!results.length && (
          <div className="empty">
            {isIndexing ? "Preparing index" : "Start typing to search the generated corpus"}
          </div>
        )}
      </section>

      {selectedResult && (
        <aside className="documentPanel" aria-label="Selected document">
          <div className="documentHeader">
            <div>
              <span>{selectedResult.externalId}</span>
              <h2>{selectedResult.title}</h2>
            </div>
            <button aria-label="Close document" onClick={() => setSelectedResult(null)}>
              <X size={18} />
            </button>
          </div>
          <div className="documentMeta">
            <span>{selectedResult.department}</span>
            <span>{selectedResult.jurisdiction}</span>
            <span>{selectedResult.year}</span>
            <span>{selectedResult.pages} pages</span>
            <span>BM25 {selectedResult.score.toFixed(2)}</span>
          </div>
          <div className="documentBody">
            {selectedResult.body.split("\n\n").map((paragraph) => {
              return (
                <p key={paragraph}>{paragraph}</p>
              );
            })}
          </div>
        </aside>
      )}
    </main>
  );
}
